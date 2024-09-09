#include "HazePch.h"
#include "CompilerHelper.h"

#include "Compiler.h"
#include "CompilerModule.h"
#include "CompilerBlock.h"
#include "CompilerClass.h"
#include "CompilerFunction.h"
#include "CompilerArrayValue.h"
#include "CompilerRefValue.h"
#include "CompilerStringValue.h"
#include "CompilerPointerFunction.h"
#include "CompilerClassValue.h"
#include "CompilerEnum.h"
#include "CompilerEnumValue.h"
#include "HazeLogDefine.h"

static HashMap<CompilerValue*, uint64>  g_CacheOffset;

HString GetLocalVariableName(const HString& name, Share<CompilerValue> value)
{
	static HAZE_STRING_STREAM s_Hss;

	s_Hss.str(H_TEXT(""));

	s_Hss << name;
	if (value->GetCount() > 0)
	{
		s_Hss << HAZE_LOCAL_VARIABLE_CONBINE << value->GetCount();
	}

	return s_Hss.str();
}

void HazeCompilerStream(HAZE_STRING_STREAM& hss, CompilerValue* value, bool bStreamValue)
{
	if (bStreamValue)
	{
		const auto& v = value->GetValue();

		HazeValueType type = value->IsEnum() ? value->GetValueType().SecondaryType : 
			value->GetValueType().PrimaryType;

		switch (type)
		{
		case HazeValueType::Bool:
			hss << v.Value.Bool;
			break;
		case HazeValueType::Int8:
			hss << v.Value.Int8;
			break;
		case HazeValueType::UInt8:
			hss << v.Value.UInt8;
			break;
		case HazeValueType::Int16:
			hss << v.Value.Int16;
			break;
		case HazeValueType::UInt16:
			hss << v.Value.UInt16;
			break;
		case HazeValueType::Int32:
			hss << v.Value.Int32;
			break;
		case HazeValueType::UInt32:
			hss << v.Value.UInt32;
			break;
		case HazeValueType::Int64:
			hss << v.Value.Int64;
			break;
		case HazeValueType::UInt64:
			hss << v.Value.UInt64;
			break;
		case HazeValueType::Float32:
			hss << v.Value.Float32;
			break;
		case HazeValueType::Float64:
			hss << v.Value.Float64;
			break;
		default:
			break;
		}
	}
	else
	{
		hss << " ";
		value->GetValueType().StringStreamTo(hss);
	}
}

void HazeCompilerStream(HAZE_STRING_STREAM& hss, Share<CompilerValue> value, bool bStreamValue)
{
	HazeCompilerStream(hss, value.get(), bStreamValue);
}

Share<CompilerValue> CreateVariableImpl(CompilerModule* compilerModule, const HazeDefineType& type, HazeVariableScope scope, 
	HazeDataDesc desc, int count, Share<CompilerValue> assignValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	switch (type.PrimaryType)
	{
	case HazeValueType::Void:
	case HazeValueType::Bool:
	case HazeValueType::Int8:
	case HazeValueType::UInt8:
	case HazeValueType::Int16:
	case HazeValueType::UInt16:
	case HazeValueType::Int32:
	case HazeValueType::UInt32:
	case HazeValueType::Int64:
	case HazeValueType::UInt64:
	case HazeValueType::Float32:
	case HazeValueType::Float64:
	case HazeValueType::MultiVariable:
		return MakeShare<CompilerValue>(compilerModule, type, scope, desc, count, assignValue);
	case HazeValueType::Array:
		return MakeShare<CompilerArrayValue>(compilerModule, type, scope, desc, count, arrayDimension);
	case HazeValueType::Refrence:
		return MakeShare<CompilerRefValue>(compilerModule, type, scope, desc, count, assignValue);
	case HazeValueType::Function:
		return MakeShare<CompilerPointerFunction>(compilerModule, type, scope, desc, count, params ? params : nullptr);
	case HazeValueType::String:
		return MakeShare<CompilerStringValue>(compilerModule, type, scope, desc, count);
	case HazeValueType::Class:
	{
		return MakeShare<CompilerClassValue>(compilerModule, type, scope, desc, count);
	}
	case HazeValueType::Enum:
	{
		auto enumValue = compilerModule->GetEnum(compilerModule, *type.CustomName).get();
		return MakeShare<CompilerEnumValue>(enumValue, compilerModule, type, scope, desc, count, assignValue);
	}
	default:
		break;
	}

	return nullptr;
}

Share<CompilerValue> CreateVariable(CompilerModule* compilerModule, const HazeDefineType& type, HazeVariableScope scope,
	HazeDataDesc desc, int count, Share<CompilerValue> refValue, uint64 arrayDimension, V_Array<HazeDefineType>* params)
{
	return CreateVariableImpl(compilerModule, type, scope, desc, count, refValue, arrayDimension, params);
}

V_Array<Pair<HazeDataDesc, V_Array<Share<CompilerValue>>>> CreateVariableCopyClassMember(CompilerModule* compilerModule,
	HazeVariableScope scope, CompilerClass* compilerClass)
{
	V_Array<Pair<HazeDataDesc, V_Array<Share<CompilerValue>>>> members;
	for (auto& it : compilerClass->GetClassMemberData())
	{
		members.push_back({ it.first, {} });
		members.back().second.resize(it.second.size());
		for (size_t i = 0; i < it.second.size(); i++)
		{
			auto& var = it.second[i].second;
			members.back().second[i] = CreateVariableImpl(compilerModule, var->GetValueType(), scope, var->GetVariableDesc(), 0, var,
				var->IsArray() ? DynamicCast<CompilerArrayValue>(var)->GetArrayDimension() : 0,
				var->IsFunction() ? &const_cast<V_Array<HazeDefineType>&>(DynamicCast<CompilerPointerFunction>(var)->GetParamTypes()) : nullptr);
		}
	}

	return members;
}

//Share<HazeCompilerValue> CreateVariable(const HazeValue& Var, HazeDataDesc Scope)
//{
//	return MakeShare<HazeCompilerValue>(Var, Scope);
//}

//void StreamPointerValue(HAZE_STRING_STREAM& hss, Share<HazeCompilerValue> value)
//{
//	auto pointerValue = DynamicCast<HazeCompilerPointerValue>(value);
//	if (value->GetValueType().PrimaryType == HazeValueType::PointerBase)
//	{
//		hss << " " << (uint32)pointerValue->GetValueType().PrimaryType;
//	}
//	else
//	{
//		hss << " " << pointerValue->GetValueType().CustomName;
//	}
//}

void StreamClassValue(HAZE_STRING_STREAM& hss, Share<CompilerValue> value)
{
	auto classValue = DynamicCast<CompilerClassValue>(value);
	hss << " " << classValue->GetOwnerClassName();
}

void StreamCompilerValue(HAZE_STRING_STREAM& hss, InstructionOpCode insCode, Share<CompilerValue> value, const HChar* defaultName)
{
	hss << GetInstructionString(insCode) << " " << (uint32)value->GetValueType().PrimaryType;
	if (defaultName)
	{
		hss << " " << defaultName;
	}
	hss << " " << (uint32)value->GetVariableDesc();

	/*if (value->IsPointer())
	{
		StreamPointerValue(hss, value);
	}
	else*/ if (value->IsClass())
	{
		StreamClassValue(hss, value);
	}

	hss << std::endl;
}

HString GetObjectName(const HString& inName)
{
	size_t pos = inName.find(TOKEN_THIS);
	if (pos != HString::npos)
	{
		return inName.substr(0, pos);
	}
	else
	{
		pos = inName.find(TOKEN_THIS);
		if (pos != HString::npos)
		{
			return inName.substr(0, pos);
		}
	}

	return inName;
}

Share<CompilerValue> GetObjectMember(CompilerModule* compilerModule, const HString& inName)
{
	bool isPointer;
	return GetObjectMember(compilerModule, inName, isPointer);
}

Share<CompilerValue> GetObjectMember(CompilerModule* compilerModule, const HString& inName, bool& isPointer)
{
	HString objectName;
	HString memberName;
	return GetObjectNameAndMemberName(compilerModule, inName, objectName, memberName, isPointer);
}

Share<CompilerValue> GetObjectNameAndMemberName(CompilerModule* compilerModule, const HString& inName, 
	HString& outObjectName, HString& outMemberName, bool& isPointer)
{
	auto pos = inName.find(TOKEN_THIS);
	if (pos != HString::npos)
	{
		isPointer = true;
		outObjectName = inName.substr(0, pos);
		outMemberName = inName.substr(pos + HString(TOKEN_THIS).size());

		auto pointerValue = DynamicCast<HazeCompilerPointerValue>(compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName));
		auto compilerClass = compilerModule->GetClass(*pointerValue->GetValueType().CustomName);

		Share<CompilerClassValue> classValue = nullptr;
		if (outObjectName == TOKEN_THIS)
		{
			//classValue = compilerClass->GetThisPointerToValue();
		}
		else
		{
			//classValue = compilerClass->GetNewPointerToValue();
		}

		return classValue->GetMember(outMemberName);
	}
	else
	{
		pos = inName.find(TOKEN_THIS);
		if (pos != HString::npos)
		{
			isPointer = false;
			outObjectName = inName.substr(0, pos);
			outMemberName = inName.substr(pos + HString(TOKEN_THIS).size());

			auto classValue = DynamicCast<CompilerClassValue>(compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName));
			if (classValue)
			{
				return classValue->GetMember(outMemberName);
			}
			else
			{
				classValue = DynamicCast<CompilerClassValue>(CompilerModule::GetGlobalVariable(compilerModule, outObjectName));
				if (classValue)
				{
					return classValue->GetMember(outMemberName);
				}
				else
				{
					COMPILER_ERR_MODULE_W("函数<%s>中未能找到类对象<%s>", compilerModule->GetName().c_str(),
						compilerModule->GetCurrFunction()->GetName().c_str(), outObjectName.c_str());
				}
			}
		}
	}

	return nullptr;
}

Share<CompilerFunction> GetObjectFunction(CompilerModule* compilerModule, const HString& inName)
{
	bool isPointer;
	return GetObjectFunction(compilerModule, inName, isPointer).first;
}

Pair<Share<CompilerFunction>, Share<CompilerValue>> GetObjectFunction(CompilerModule* compilerModule, 
	const HString& inName, bool& isPointer)
{
	HString objectName;
	HString functionName;
	return GetObjectNameAndFunctionName(compilerModule, inName, objectName, functionName, isPointer);
}

Pair<Share<CompilerFunction>, Share<CompilerValue>> GetObjectNameAndFunctionName(CompilerModule* compilerModule,
	const HString& inName, HString& outObjectName, HString& outFunctionName, bool& isPointer)
{
	Share<CompilerValue> findVariable = nullptr;
	auto pos = inName.find(TOKEN_THIS);
	if (pos != HString::npos)
	{
		isPointer = true;
		outObjectName = inName.substr(0, pos);
		outFunctionName = inName.substr(pos + HString(TOKEN_THIS).size());

		if (compilerModule->GetCurrFunction())
		{
			findVariable = compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName);
		}

		if (!findVariable)
		{
			findVariable = CompilerModule::GetGlobalVariable(compilerModule, outObjectName);
		}

		auto pointerValue = DynamicCast<HazeCompilerPointerValue>(findVariable);
		if (pointerValue)
		{
			auto compilerClass = compilerModule->GetClass(*pointerValue->GetValueType().CustomName);
			return { compilerClass->FindFunction(outFunctionName), findVariable };
		}
	}
	else
	{
		pos = inName.find(TOKEN_THIS);
		if (pos != HString::npos)
		{
			isPointer = false;
			outObjectName = inName.substr(0, pos);
			outFunctionName = inName.substr(pos + HString(TOKEN_THIS).size());

			if (compilerModule->GetCurrFunction())
			{
				findVariable = compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName);
			}

			if (!findVariable)
			{
				findVariable = CompilerModule::GetGlobalVariable(compilerModule, outObjectName);
			}

			auto classValue = DynamicCast<CompilerClassValue>(findVariable);
			if (classValue)
			{
				return { classValue->GetOwnerClass()->FindFunction(outFunctionName), findVariable };
			}
			else if (findVariable)
			{
				HAZE_LOG_ERR_W("获得类对象<%s>的成员函数错误!\n"), outObjectName.c_str();
			}
		}
	}

	return { nullptr, nullptr };
}

bool TrtGetVariableName(CompilerFunction* function, const Pair<HString, Share<CompilerValue>>& data,
	const CompilerValue* value, HString& outName, bool getOffsets, V_Array<Pair<uint64, CompilerValue*>>* offsets)
{
	if (data.second.get() == value)
	{
		outName = GetLocalVariableName(data.first, data.second);
		return true;
	}

	if (value->IsClassMember())
	{
		if (data.second->IsClass())
		{
			auto compilerClass = DynamicCast<CompilerClassValue>(data.second);
			if (compilerClass->GetMemberName(value, outName, getOffsets, offsets))
			{
				outName = GetLocalVariableName(data.first, data.second) + outName;
				if (!value->IsClassPublicMember() && function->GetClass() != compilerClass->GetOwnerClass())
				{
					HAZE_LOG_ERR_W("不能够访问类<%s>非公开成员变量<%s>!\n", compilerClass->GetOwnerClassName().c_str(), outName.c_str());
					return false;
				}
				return true;
			}
		}
	}

	return false;
}

uint32 GetSizeByCompilerValue(Share<CompilerValue> v)
{
	if (v->IsEnum())
	{
		return v->GetSize();
	}
	else
	{
		return GetSizeByHazeType(v->GetValueType().PrimaryType);
	}
}

void GetTemplateClassName(HString& inName, const V_Array<TemplateDefineType>& templateTypes)
{
	for (auto& type : templateTypes)
	{
		if (type.IsDefines)
		{
			HString name;
			GetTemplateClassName(name, type.Defines->Types);
			inName += HAZE_TEMPLATE_CONBINE + name;
		}
		else
		{
			inName += HAZE_TEMPLATE_CONBINE + type.Type->GetFullTypeName();
		}
	}
}


void GenVariableHzic(CompilerModule* compilerModule, HAZE_STRING_STREAM& hss, const Share<CompilerValue>& value)
{
	static HString s_StrName;

	bool find = false; 
	s_StrName.clear();
	if (value->IsGlobalVariable())
	{
		find = CompilerModule::GetGlobalVariableName(compilerModule, value, s_StrName);
		if (!find && value->IsNullPtr())
		{
			find = true;
			s_StrName = NULL_PTR;
		}
	}
	else if (value->IsLocalVariable())
	{
		find = compilerModule->GetCurrFunction()->FindLocalVariableName(value.get(), s_StrName);
	}
	else if (value->IsFunctionAddress())
	{
		s_StrName = *value->GetValueType().CustomName;
		if (!s_StrName.empty())
		{
			find = true;
		}
	}
	else if (value->IsTempVariable())
	{
		find = true;
		s_StrName = compilerModule->GetCompiler()->GetRegisterName(value);
	}
	else
	{
		HAZE_LOG_ERR_W("生成中间代码错误,变量作用域错误!\n");
		return;
	}

	if (!find)
	{
		/*if (ownerClass && ownerClass->TryGetMemberOrMemberMemberName())
		{

		}*/

		HAZE_LOG_ERR_W("生成中间代码错误,未能找到变量!\n");
		return;
	}

	hss << s_StrName << " " << CAST_SCOPE(value->GetVariableScope()) << " " << CAST_DESC(value->GetVariableDesc()) << " ";
	value->GetValueType().StringStreamTo(hss);

	if (value->IsString())
	{
		hss << " " << compilerModule->GetGlobalStringIndex(value);
		//index = compilerModule->GetGlobalStringIndex(value);
	}
}

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, Share<CompilerValue> assignTo,
	Share<CompilerValue> oper1, Share<CompilerValue> oper2, const HazeDefineType* expectType)
{
	if (expectType)
	{
		auto type = *expectType;

		if (IsMultiVariableTye(expectType->PrimaryType))
		{
			if (IsNumberType(type.PrimaryType))
			{
				if (IsUnsignedIntegerType(type.PrimaryType))
				{
					type.PrimaryType = HazeValueType::UInt64;
				}
				else if (IsIntegerType(type.PrimaryType))
				{
					type.PrimaryType = HazeValueType::Int64;
				}
				else
				{
					type.PrimaryType = HazeValueType::Float64;
				}
				oper1 = m->GetCompiler()->CreateMov(m->GetCompiler()->GetTempRegister(type), oper1);
			}
		}
		else if (IsRefrenceType(type.PrimaryType) && !oper1->IsRefrence())
		{
			type.PrimaryType = HazeValueType::UInt64;
			oper1 = m->GetCompiler()->CreateLea(m->GetCompiler()->GetTempRegister(type), oper1);
		}
	}
	
	switch (opCode)
	{
	case InstructionOpCode::NONE:
		COMPILER_ERR_MODULE_W("生成中间代码错误, 中间操作码为空", m->GetName().c_str());
		break;
	case InstructionOpCode::MOV:
	case InstructionOpCode::MOVPV:
	case InstructionOpCode::MOVTOPV:
	case InstructionOpCode::LEA:
	case InstructionOpCode::BIT_NEG:
	case InstructionOpCode::NOT:
	case InstructionOpCode::NEG:
	case InstructionOpCode::NEW:
	case InstructionOpCode::CVT:
	{

		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(m, hss, assignTo);
		hss << " ";
		GenVariableHzic(m, hss, oper1);
	}
		break;
	case InstructionOpCode::ADD:
	case InstructionOpCode::SUB:
	case InstructionOpCode::MUL:
	case InstructionOpCode::DIV:
	case InstructionOpCode::MOD:
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHR:
	{
		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(m, hss, assignTo);
		hss << " ";
		GenVariableHzic(m, hss, oper1);
		hss << " ";
		GenVariableHzic(m, hss, oper2);
	}
		break;
	case InstructionOpCode::CMP:
	{
		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(m, hss, oper1);
		hss << " ";
		GenVariableHzic(m, hss, oper2);
	}
		break;
	case InstructionOpCode::PUSH:
	case InstructionOpCode::POP:
	{
		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(m, hss, oper1);
	}
		break;
	case InstructionOpCode::RET:
	{
		if (oper1)
		{
			hss << GetInstructionString(opCode) << " ";
			GenVariableHzic(m, hss, oper1);
		}
		else
		{
			hss << GetInstructionString(InstructionOpCode::RET) << " " << H_TEXT("Void") << " " << 
				CAST_SCOPE(HazeVariableScope::None) << " " << CAST_DESC(HazeDataDesc::None) << " " << CAST_TYPE(HazeValueType::Void);
		}
	}
		break;
	default:
		break;
	}

	hss << std::endl;
}

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, Share<CompilerBlock> block1,
	Share<CompilerBlock> block2)
{
	switch (opCode)
	{
	case InstructionOpCode::JMP:
		hss << GetInstructionString(opCode) << " " << block1->GetName() << std::endl;
		break;
	case InstructionOpCode::JNE:
	case InstructionOpCode::JNG:
	case InstructionOpCode::JNL:
	case InstructionOpCode::JE:
	case InstructionOpCode::JG:
	case InstructionOpCode::JL:
	{
		hss << GetInstructionString(opCode) << " ";

		if (block1)
		{
			hss << block1->GetName() << " ";
		}
		else
		{
			hss << HAZE_JMP_NULL << " ";
		}

		if (block2)
		{
			hss << block2->GetName();
		}
		else
		{
			hss << HAZE_JMP_NULL << " ";
		}

		hss << std::endl;

	}
		break;
	}
}

HString GenIRCode(InstructionOpCode opCode, uint64 number)
{
	switch (opCode)
	{
	case InstructionOpCode::LINE:
		return GetInstructionString(opCode) + (H_TEXT(" ") + HAZE_TO_HAZE_STR(number)) + H_TEXT("\n");
	}
	
	return HString();
}

void GenIRCode(HAZE_STRING_STREAM& hss, CompilerModule* m, InstructionOpCode opCode, uint64 paramCount, uint64 paramSize, Share<CompilerFunction> function,
	Share<CompilerValue> pointerFunction, Share<CompilerValue> advancePointerTo, void* advanceFuncAddress)
{
	switch (opCode)
	{
	case InstructionOpCode::CALL:
	{
		HString varName;
		hss << GetInstructionString(InstructionOpCode::CALL) << " ";
		if (function)
		{
			hss << function->GetName() << " " << CAST_TYPE(HazeValueType::None) << " " << CAST_SCOPE(HazeVariableScope::Ignore) << " " <<
				CAST_DESC(HazeDataDesc::FunctionAddress) << " " << paramCount << " " << paramSize << " " << m->GetName() << " "
				<< CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;
		}
		else if (pointerFunction)
		{
			m->GetGlobalVariableName(m, pointerFunction, varName);
			if (varName.empty())
			{
				m->GetCurrFunction()->FindLocalVariableName(pointerFunction.get(), varName);
				if (varName.empty())
				{
					HAZE_LOG_ERR_W("函数指针调用失败!\n");
					return;
				}
			}

			hss << varName << " " << CAST_TYPE(HazeValueType::Function) << " "
				<< CAST_SCOPE(pointerFunction->GetVariableScope()) << " " << CAST_DESC(pointerFunction->GetVariableDesc()) << " " << paramCount
				<< " " << paramSize << " " << m->GetName() << " " << CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;
		}
		else
		{
			m->GetGlobalVariableName(m, advancePointerTo, varName);
			if (varName.empty())
			{
				m->GetCurrFunction()->FindLocalVariableName(advancePointerTo.get(), varName);
				if (varName.empty())
				{
					HAZE_LOG_ERR_W("复杂类型函数调用失败!\n");
					return;
				}
			}

			hss << varName << " " << CAST_TYPE(HazeValueType::Function) << " " << CAST_SCOPE(advancePointerTo->GetVariableScope()) << " "
				<< CAST_DESC(advancePointerTo->GetVariableDesc()) << " " << paramCount << " " << paramSize << " " << m->GetName() << " " 
				<< CAST_DESC(HazeDataDesc::CallFunctionPointer) << " " << advanceFuncAddress << std::endl;
		}
	}
		break;
	}
}