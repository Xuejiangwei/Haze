#include "HazePch.h"
#include "CompilerSymbol.h"
#include "HazeTypeInfo.h"
#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerFunction.h"
#include "CompilerValue.h"
#include "CompilerClass.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"

CompilerSymbol::CompilerSymbol(Compiler* compiler) : m_Compiler(compiler)
{
	m_TypeInfo = new HazeTypeInfoMap(compiler);
}

CompilerSymbol::~CompilerSymbol()
{
	if (m_TypeInfo)
	{
		delete m_TypeInfo;
	}

	for (auto iter : m_Symbols)
	{
		switch (((SymbolTypeHeader*)iter.second.second)->Type)
		{
			case HazeValueType::Class:
				delete (ClassSymbol*)iter.second.second;
				break;
			case HazeValueType::Enum:
				delete (EnumSymbol*)iter.second.second;
				break;
			default:
				break;
		}
	}
}

x_uint32 CompilerSymbol::RegisterSymbol(const HString& name)
{
	auto iter = m_Symbols.find(name);
	if (iter != m_Symbols.end())
	{
		return iter->second.first;
	}

	auto typeId = m_TypeInfo->ReserveTypeId(name);
	m_Symbols[name] = { typeId, nullptr };
	return typeId;
}

//void CompilerSymbol::RegisterFunctionSymbol(const HString& name, x_uint32 functionType, V_Array<x_uint32>&& params, HazeFunctionDesc desc, const HString* className, bool isClassPublic)
//{
//	auto iter = m_FunctionSymbols.end();
//	
//	FunctionSymbol* data = nullptr;
//	if (className)
//	{
//		HString classFuncName = GetHazeClassFunctionName(*className, name);
//		iter = m_FunctionSymbols.find(classFuncName);
//		if (iter != m_FunctionSymbols.end())
//		{
//			return;
//		}
//
//		data = &m_FunctionSymbols[classFuncName];
//		data->ClassFunctionIsPublic = isClassPublic;
//	}
//	else
//	{
//		iter = m_FunctionSymbols.find(name);
//		iter = m_FunctionSymbols.find(name);
//		if (iter != m_FunctionSymbols.end())
//		{
//			return;
//		}
//
//		data = &m_FunctionSymbols[name];
//	}
//
//	if (data)
//	{
//		data->ClassTypeId = className ? GetSymbolTypeId(*className) : 0;
//		data->FunctionType = functionType;
//		data->Params = params;
//		data->Desc = desc;
//	}
//}

void CompilerSymbol::Register_GlobalVariable(const HString& moduleName, const HString& name, x_uint32 typeId, x_uint32 line)
{
	auto m = m_Compiler->GetModule(moduleName);
	auto iter = m_GlobalVariables.find(name);
	if (iter != m_GlobalVariables.end())
	{
		if (iter->second.Module != m)
		{
			SYMBOL_ERR_W("模块<%s>与模块<%s>存在相同名称的全局变量<%s>", moduleName.c_str(), iter->second.Module->GetName().c_str(), name.c_str());
		}

		return;
	}

	m_GlobalVariables[name] = { m, typeId, line };
}

void CompilerSymbol::Register_Class(const HString& moduleName, const HString& name, V_Array<HString>& parents, V_Array<Pair<HString, x_uint32>>&& publicMembers, V_Array<Pair<HString, x_uint32>>&& privateMembers, bool publicFirst)
{
	auto iter = m_Symbols.find(name);
	if (iter != m_Symbols.end())
	{
		if (!iter->second.second)
		{
			auto data = new ClassSymbol();
			iter->second.second = data;

			data->Type = HazeValueType::Class;
			data->PublicFirst = publicFirst;
			data->PublicMembers = publicMembers;
			data->PrivateMembers = privateMembers;

			data->Parents.resize(parents.size());
			for (x_uint64 i = 0; i < parents.size(); i++)
			{
				auto it = m_Symbols.find(parents[i]);
				if (it != m_Symbols.end())
				{
					data->Parents[i] = it->second.first;
				}
				else
				{
					SYMBOL_ERR_W("未能找到符号<%s>的信息", parents[i].c_str());
				}
			}

			CLASS_TYPE_INFO(info, name);
			m_TypeInfo->RegisterResolvedType(moduleName, iter->second.first, &info);
		}
		else
		{
			SYMBOL_ERR_W("重复注册类<%s>成员信息", name.c_str());
		}
	}
	else
	{
		SYMBOL_ERR_W("注册类<%s>成员失败, 未能找到其符号信息", name.c_str());
	}
}

void CompilerSymbol::Register_Enum(const HString& moduleName, const HString& name, V_Array<Pair<HString, int>>&& members)
{
	auto iter = m_Symbols.find(name);
	if (iter != m_Symbols.end())
	{
		if (!iter->second.second)
		{
			auto data = new EnumSymbol();
			iter->second.second = data;

			data->Type = HazeValueType::Enum;
			data->Members = members;

			ENUM_TYPE_INFO(info, name);
			m_TypeInfo->RegisterResolvedType(moduleName, iter->second.first, &info);
		}
		else
		{
			SYMBOL_ERR_W("重复注册枚举<%s>成员信息", name.c_str());
		}
	}
	else
	{
		SYMBOL_ERR_W("注册枚举<%s>成员失败, 未能找到其符号信息", name.c_str());
	}
}

void CompilerSymbol::Register_Function(const HString& moduleName, const HString& name, x_uint32 functionType, V_Array<HazeDefineVariable>&& params, HazeFunctionDesc desc, const HString* className, bool isClassPublic)
{
	auto m = m_Compiler->GetModule(moduleName);
	auto iter = m_FunctionSymbols.end();
	FunctionSymbol* data = nullptr;
	if (className)
	{
		HString classFuncName = GetHazeClassFunctionName(*className, name);
		iter = m_FunctionSymbols.find(classFuncName);
	
		if (iter == m_FunctionSymbols.end())
		{
			data = &m_FunctionSymbols[classFuncName];
		}
	}
	else
	{
		iter = m_FunctionSymbols.find(name);
	}

	if (iter != m_FunctionSymbols.end())
	{
		// 比对检查
		if (iter->second.Params.size() != params.size())
		{
			SYMBOL_ERR_W("模块<%s>的<%s>函数符号信息的参数个数不匹配, 定义为<%d>个, 调用为<%d>个", m->GetName().c_str(), name.c_str(), params.size(), iter->second.Params.size());
		}

		if (className && *className != *m_TypeInfo->GetTypeName(iter->second.ClassTypeId))
		{
			SYMBOL_ERR_W("模块<%s>的<%s>函数符号信息的类型不匹配, 定义为<%s>, 调用为<%s>", m->GetName().c_str(), name.c_str(), className->c_str(), *m_TypeInfo->GetTypeName(iter->second.ClassTypeId)->c_str());
		}

		iter->second.ParamNames.resize(params.size());
		for (x_uint64 i = 0; i < params.size(); i++)
		{
			iter->second.ParamNames[i] = Move(params[i].Name);
		}
		iter->second.Module = m;
	}
	else
	{
		if (!data)
		{
			data = &m_FunctionSymbols[name];
		}

		data->Module = m;
		data->ClassFunctionIsPublic = isClassPublic;
		data->ClassTypeId = className ? GetSymbolTypeId(*className) : 0;
		data->FunctionType = functionType;
		data->Desc = desc;

		data->Params.resize(params.size());
		for (x_uint64 i = 0; i < params.size(); i++)
		{
			data->Params[i] = params[i].Type.TypeId;
		}

		data->ParamNames.resize(params.size());
		for (x_uint64 i = 0; i < params.size(); i++)
		{
			data->ParamNames[i] = Move(params[i].Name);
		}
	}
}

void CompilerSymbol::AddModuleRefSymbol(const HString& moduleName, const HString& symbol)
{
	auto iter = m_Symbols.find(symbol);
	if (iter != m_Symbols.end())
	{
		return;
	}

	m_TypeInfo->AddModuleRef(moduleName, RegisterSymbol(symbol));
}

//void CompilerSymbol::ResolveSymbol_Class(const HString& name, CompilerClass* compilerClass)
//{
//	auto iter = (ClassSymbol*)m_Symbols[name].second;
//	auto data = compilerClass->GetClassMemberData();
//	if (iter->PublicMembers.size() + iter->PrivateMembers.size() != data.size())
//	{
//		SYMBOL_ERR_W("验证类<%s>符号时，成员个数不相等, 第一遍为<%d>, 第二遍为<%d>", name.c_str(), iter->PublicMembers.size() + iter->PrivateMembers.size(), data.size());
//	}
//
//	if (data.size() > 0)
//	{
//		decltype(iter->PublicMembers)* firstScopeMembers = nullptr;
//		decltype(firstScopeMembers) secondScopeMembers = nullptr;
//		if (data[0].second->IsClassPublicMember())
//		{
//			firstScopeMembers = &iter->PublicMembers;
//			secondScopeMembers = &iter->PrivateMembers;
//		}
//		else
//		{
//			firstScopeMembers = &iter->PrivateMembers;
//			secondScopeMembers = &iter->PublicMembers;
//		}
//		
//
//		for (x_uint64 i = 0; i < firstScopeMembers->size(); i++)
//		{
//			if (firstScopeMembers->at(i).first == data[i].first)
//			{
//				if (firstScopeMembers->at(i).second != data[i].second->GetTypeId())
//				{
//					SYMBOL_ERR_W("验证类<%s>符号时，成员<%s>类型不对应, 第一遍为<%s>, 第二遍为<%s>", name.c_str(), data[i].first.c_str(), m_TypeInfo->GetTypeName(firstScopeMembers->at(i).second)->c_str(),
//						m_TypeInfo->GetTypeName(data[i].second->GetTypeId())->c_str());
//				}
//			}
//			else
//			{
//				SYMBOL_ERR_W("验证类<%s>符号时，成员名称位置不对应, 第一遍为<%s>, 第二遍为<%s>", name.c_str(), firstScopeMembers->at(i).first.c_str(), data[i].first.c_str());
//			}
//		}
//
//		for (x_uint64 i = 0; i < secondScopeMembers->size(); i++)
//		{
//			auto dataIndex = i + firstScopeMembers->size();
//			if (secondScopeMembers->at(i).first == data[dataIndex].first)
//			{
//				if (secondScopeMembers->at(i).second != data[dataIndex].second->GetTypeId())
//				{
//					SYMBOL_ERR_W("验证类<%s>符号时，成员<%s>类型不对应, 第一遍为<%s>, 第二遍为<%s>", name.c_str(), data[dataIndex].first.c_str(), m_TypeInfo->GetTypeName(secondScopeMembers->at(i).second)->c_str(),
//						m_TypeInfo->GetTypeName(data[dataIndex].second->GetTypeId())->c_str());
//				}
//			}
//			else
//			{
//				SYMBOL_ERR_W("验证类<%s>符号时，成员名称位置不对应, 第一遍为<%s>, 第二遍为<%s>", name.c_str(), secondScopeMembers->at(i).first.c_str(), data[i].first.c_str());
//			}
//		}
//	}
//}
//
//void CompilerSymbol::ResolveSymbol_Enum(const HString& name, CompilerEnum* compilerEnum)
//{
//	auto iter = (EnumSymbol*)m_Symbols[name].second;
//	if (iter->Members.size() != compilerEnum->m_EnumValues.size())
//	{
//		SYMBOL_ERR_W("验证枚举<%s>符号时，成员个数不相等, 第一遍为<%d>, 第二遍为<%d>", name.c_str(), iter->Members.size(), compilerEnum->m_EnumValues.size());
//	}
//
//	for (x_uint64 i = 0; i < compilerEnum->m_EnumValues.size(); i++)
//	{
//		if (iter->Members[i].first != compilerEnum->m_EnumValues[i].first)
//		{
//			SYMBOL_ERR_W("验证枚举<%s>符号时，第<%d>成员命名不同, 第一遍为<%d>, 第二遍为<%d>", name.c_str(), i, iter->Members[i].first.c_str(), compilerEnum->m_EnumValues[i].first.c_str());
//		}
//		else if (iter->Members[i].second != compilerEnum->m_EnumValues[i].second->GetValue().Value.Int32)
//		{
//			SYMBOL_ERR_W("验证枚举<%s>符号时，第<%d>成员值不同, 第一遍为<%d>, 第二遍为<%d>", name.c_str(), i, iter->Members[i].second, compilerEnum->m_EnumValues[i].second->GetValue().Value.Int32);
//		}
//	}
//}

void CompilerSymbol::IdentifySymbolType()
{
	HashMap<x_uint32, ResolveClassData> classMap;

	for (auto& iter : m_Symbols)
	{
		if (iter.second.second)
		{
			if (IsClassType(iter.second.second->Type))
			{
				auto info = (ClassSymbol*)(iter.second.second);
				auto moduleName = m_TypeInfo->GetRegisterTypeModule(iter.first);
				auto m = m_Compiler->GetModule(*moduleName);

				classMap[iter.second.first] = { MakeShare<CompilerClass>(m, iter.first, iter.second.first), info };
			}
			else if (IsEnumType(iter.second.second->Type))
			{
				auto moduleName = m_TypeInfo->GetRegisterTypeModule(iter.first);
				auto m = m_Compiler->GetModule(*moduleName);
				auto compEnum = m->CreateEnum(iter.first);
				
				auto info = (EnumSymbol*)(iter.second.second);
				for (x_uint64 i = 0; i < info->Members.size(); i++)
				{
					compEnum->AddEnumValue(info->Members[i].first, m_Compiler->GetConstantValueInt(info->Members[i].second));
				}
			}
			else
			{
				SYMBOL_ERR_W("符号<%s>解析为<%s>类型", iter.first.c_str(), GetHazeValueTypeString(iter.second.second->Type));
			}
		}
		else
		{
			SYMBOL_ERR_W("符号<%s>未能找到定义", iter.first.c_str());
		}

	}

	for (auto& iter : classMap)
	{
		if (!ResolveCompilerClass(classMap, iter.first))
		{
			return;
		}
	}

	for (auto& iter : m_FunctionSymbols)
	{
		auto symbol = GetSymbolByTypeId(iter.second.FunctionType);
		auto symbolData = m_Symbols.find(*symbol);
		if (symbolData != m_Symbols.end())
		{
			if (!symbolData->second.second)
			{
				SYMBOL_ERR_W("函数<%s>的返回类型<%s>未能找到定义", iter.first.c_str(), symbol->c_str());
			}
		}

		for (x_uint64 i = 0; i < iter.second.Params.size(); i++)
		{
			symbol = GetSymbolByTypeId(iter.second.Params[i]);
			symbolData = m_Symbols.find(*symbol);
			if (symbolData != m_Symbols.end())
			{
				if (!symbolData->second.second)
				{
					SYMBOL_ERR_W("函数<%s>的第<%d>个参数<%s>未能找到定义", iter.first.c_str(), i, symbol->c_str());
				}
			}
		}

		if (iter.second.ClassTypeId > 0)
		{
			symbol = GetSymbolByTypeId(iter.second.ClassTypeId);
			symbolData = m_Symbols.find(*symbol);
			if (symbolData != m_Symbols.end())
			{
				if (!symbolData->second.second)
				{
					SYMBOL_ERR_W("函数<%s>所属类<%s>未能找到定义", iter.first.c_str(), symbol->c_str());
				}
			}

			auto funcClass = iter.second.Module->GetClass(*symbol);
			auto funcName = NativeClassFunctionName(*symbol, iter.first);

			V_Array<HazeDefineVariable> params(iter.second.Params.size());
			for (x_uint64 i = 0; i < iter.second.Params.size(); i++)
			{
				params[i].Name = iter.second.ParamNames[i];
				params[i].Type = m_TypeInfo->GetVarTypeById(iter.second.Params[i]);
			}

			iter.second.Module->CreateFunction(funcClass, iter.second.Desc, funcName, m_TypeInfo->GetVarTypeById(iter.second.FunctionType), params);
		}
		else
		{
			V_Array<HazeDefineVariable> params(iter.second.Params.size());
			for (x_uint64 i = 0; i < iter.second.Params.size(); i++)
			{
				params[i].Name = iter.second.ParamNames[i];
				params[i].Type = m_TypeInfo->GetVarTypeById(iter.second.Params[i]);
			}
			iter.second.Module->CreateFunction(iter.first, m_TypeInfo->GetVarTypeById(iter.second.FunctionType), params);
		}
	}

	for (auto& iter : m_GlobalVariables)
	{
		iter.second.Module->CreateGlobalVariable({ m_TypeInfo->GetVarTypeById(iter.second.TypeId), iter.first }, iter.second.Line);
	}
}

bool CompilerSymbol::CheckValidClassInherit(x_uint32 checkTypeId, x_uint32 typeId, const HashMap<x_uint32, ResolveClassData>& classMap)
{
	auto iter = classMap.find(typeId);
	if (iter == classMap.end())
	{
		SYMBOL_ERR_W("未能找到类<%s>的定义", GetSymbolByTypeId(typeId)->c_str());
		return false;
	}

	if (checkTypeId == typeId)
	{
		return false;
	}

	auto data = iter->second;
	for (auto parent : data.SymbolInfo->Parents)
	{
		if (!CheckValidClassInherit(checkTypeId, parent, classMap))
		{
			return false;
		}
	}

	return true;
}

bool CompilerSymbol::ResolveCompilerClass(HashMap<x_uint32, ResolveClassData>& classMap, x_uint32 typeId)
{
	auto data = classMap[typeId];
	if (data.IsResolved)
	{
		return true;
	}

	// 补全父类
	{	
		V_Array<CompilerClass*> parentClass;
		for (x_uint64 i = 0; i < data.SymbolInfo->Parents.size(); i++)
		{
			if (!CheckValidClassInherit(typeId, data.SymbolInfo->Parents[i], classMap))
			{
				SYMBOL_ERR_W("类<%s>与类<%s>存在循环继承", GetSymbolByTypeId(typeId)->c_str(), GetSymbolByTypeId(data.SymbolInfo->Parents[i])->c_str());
				return false;
			}

			if (data.IsResolved)
			{
				parentClass.push_back(classMap[data.SymbolInfo->Parents[i]].CompClass.get());
			}
			else
			{
				ResolveCompilerClass(classMap, data.SymbolInfo->Parents[i]);
				parentClass.push_back(classMap[data.SymbolInfo->Parents[i]].CompClass.get());
			}
		}

		data.CompClass->ResolveClassParent(Move(parentClass));

		if (m_Compiler->IsCompileError())
		{
			return false;
		}
	}

	// 补全成员
	{
		V_Array<Pair<HString, Share<CompilerValue>>> classData;

		decltype(data.SymbolInfo->PublicMembers)* firstScopeMembers = nullptr;
		decltype(firstScopeMembers) secondScopeMembers = nullptr;
		if (data.SymbolInfo->PublicFirst)
		{
			firstScopeMembers = &data.SymbolInfo->PublicMembers;
			secondScopeMembers = &data.SymbolInfo->PrivateMembers;
		}
		else
		{
			firstScopeMembers = &data.SymbolInfo->PrivateMembers;
			secondScopeMembers = &data.SymbolInfo->PublicMembers;
		}

		for (x_uint64 i = 0; i < firstScopeMembers->size(); i++)
		{
			classData.push_back({ firstScopeMembers->at(i).first, m_Compiler->CreateClassVariable(data.CompClass->m_Module, m_TypeInfo->GetVarTypeById(firstScopeMembers->at(i).second)) });
		}
		for (x_uint64 i = 0; i < secondScopeMembers->size(); i++)
		{
			classData.push_back({ secondScopeMembers->at(i).first, m_Compiler->CreateClassVariable(data.CompClass->m_Module, m_TypeInfo->GetVarTypeById(secondScopeMembers->at(i).second)) });
		}

		data.CompClass->ResolveClassData(Move(classData));
	}

	data.IsResolved = true;
	data.CompClass->m_Module->m_HashMap_Classes[data.CompClass->GetName()] = data.CompClass;

	return true;
}

bool CompilerSymbol::IsValidSymbol(const HString& symbol)
{
	return m_Symbols.find(symbol) != m_Symbols.end();
}

x_uint32 CompilerSymbol::GetSymbolTypeId(const HString& symbol)
{
	auto iter = m_Symbols.find(symbol);
	if (iter != m_Symbols.end())
	{
		return iter->second.first;
	}

	HAZE_LOG_INFO_W("符号<%s>未能找到有效的类型ID", symbol.c_str());
	return 0;
}

const HString* CompilerSymbol::GetSymbolByTypeId(x_uint32 typeId)
{
	return m_TypeInfo->GetClassNameById(typeId);
}