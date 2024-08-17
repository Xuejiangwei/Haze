#include "HazePch.h"
#include "HazeCompilerHelper.h"

#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerRefValue.h"
#include "HazeCompilerStringValue.h"
#include "HazeCompilerPointerFunction.h"
#include "HazeCompilerClassValue.h"
#include "HazeCompilerEnum.h"
#include "HazeCompilerEnumValue.h"
#include "HazeLogDefine.h"

HString GetLocalVariableName(const HString& name, Share<HazeCompilerValue> value)
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

void HazeCompilerStream(HAZE_STRING_STREAM& hss, HazeCompilerValue* value, bool bStreamValue)
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

void HazeCompilerStream(HAZE_STRING_STREAM& hss, Share<HazeCompilerValue> value, bool bStreamValue)
{
	HazeCompilerStream(hss, value.get(), bStreamValue);
}

Share<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* compilerModule, const HazeDefineType& type, 
	HazeVariableScope scope, HazeDataDesc desc, int count, Share<HazeCompilerValue> assignValue,
	V_Array<Share<HazeCompilerValue>> arraySize, V_Array<HazeDefineType>* params)
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
		return MakeShare<HazeCompilerValue>(compilerModule, type, scope, desc, count, assignValue);
	case HazeValueType::Array:
		return MakeShare<HazeCompilerArrayValue>(compilerModule, type, scope, desc, count, arraySize);
	case HazeValueType::Refrence:
		return MakeShare<HazeCompilerRefValue>(compilerModule, type, scope, desc, count, assignValue);
	case HazeValueType::Function:
		return MakeShare<HazeCompilerPointerFunction>(compilerModule, type, scope, desc, count, params ? params : nullptr);
	case HazeValueType::String:
		return MakeShare<HazeCompilerStringValue>(compilerModule, type, scope, desc, count);
	case HazeValueType::Class:
	{
		return MakeShare<HazeCompilerClassValue>(compilerModule, type, scope, desc, count);
	}
	case HazeValueType::Enum:
	{
		auto enumValue = compilerModule->GetEnum(compilerModule, *type.CustomName).get();
		return MakeShare<HazeCompilerEnumValue>(enumValue, compilerModule, type, scope, desc, count, assignValue);
	}
	default:
		break;
	}

	return nullptr;
}

Share<HazeCompilerValue> CreateVariable(HazeCompilerModule* compilerModule, const HazeDefineVariable& var, HazeVariableScope scope,
	HazeDataDesc desc, int count, Share<HazeCompilerValue> refValue, V_Array<Share<HazeCompilerValue>> arraySize,
	V_Array<HazeDefineType>* params)
{
	return CreateVariableImpl(compilerModule, var.Type, scope, desc, count, refValue, arraySize, params);
}

V_Array<Pair<HazeDataDesc, V_Array<Share<HazeCompilerValue>>>> CreateVariableCopyClassMember(HazeCompilerModule* compilerModule,
	HazeVariableScope scope, HazeCompilerClass* compilerClass)
{
	V_Array<Pair<HazeDataDesc, V_Array<Share<HazeCompilerValue>>>> members;
	for (auto& it : compilerClass->GetClassMemberData())
	{
		members.push_back({ it.first, {} });
		members.back().second.resize(it.second.size());
		for (size_t i = 0; i < it.second.size(); i++)
		{
			auto& var = it.second[i].second;
			members.back().second[i] = CreateVariableImpl(compilerModule, var->GetValueType(), scope, var->GetVariableDesc(), 0,
				var,
				var->IsArray() ? DynamicCast<HazeCompilerArrayValue>(var)->GetArraySize() :
				V_Array<Share<HazeCompilerValue>>{},
				var->IsFunction() ? &const_cast<V_Array<HazeDefineType>&>(DynamicCast<HazeCompilerPointerFunction>(var)->GetParamTypes()) : nullptr);
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

void StreamClassValue(HAZE_STRING_STREAM& hss, Share<HazeCompilerValue> value)
{
	auto classValue = DynamicCast<HazeCompilerClassValue>(value);
	hss << " " << classValue->GetOwnerClassName();
}

void StreamCompilerValue(HAZE_STRING_STREAM& hss, InstructionOpCode insCode, Share<HazeCompilerValue> value, const HChar* defaultName)
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

Share<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HString& inName)
{
	bool isPointer;
	return GetObjectMember(compilerModule, inName, isPointer);
}

Share<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HString& inName, bool& isPointer)
{
	HString objectName;
	HString memberName;
	return GetObjectNameAndMemberName(compilerModule, inName, objectName, memberName, isPointer);
}

Share<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* compilerModule, const HString& inName, 
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

		Share<HazeCompilerClassValue> classValue = nullptr;
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

			auto classValue = DynamicCast<HazeCompilerClassValue>(compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName));
			if (classValue)
			{
				return classValue->GetMember(outMemberName);
			}
			else
			{
				classValue = DynamicCast<HazeCompilerClassValue>(HazeCompilerModule::GetGlobalVariable(compilerModule, outObjectName));
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

Share<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* compilerModule, const HString& inName)
{
	bool isPointer;
	return GetObjectFunction(compilerModule, inName, isPointer).first;
}

Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetObjectFunction(HazeCompilerModule* compilerModule, 
	const HString& inName, bool& isPointer)
{
	HString objectName;
	HString functionName;
	return GetObjectNameAndFunctionName(compilerModule, inName, objectName, functionName, isPointer);
}

Pair<Share<HazeCompilerFunction>, Share<HazeCompilerValue>> GetObjectNameAndFunctionName(HazeCompilerModule* compilerModule,
	const HString& inName, HString& outObjectName, HString& outFunctionName, bool& isPointer)
{
	Share<HazeCompilerValue> findVariable = nullptr;
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
			findVariable = HazeCompilerModule::GetGlobalVariable(compilerModule, outObjectName);
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
				findVariable = HazeCompilerModule::GetGlobalVariable(compilerModule, outObjectName);
			}

			auto classValue = DynamicCast<HazeCompilerClassValue>(findVariable);
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

bool TrtGetVariableName(HazeCompilerFunction* function, const Pair<HString, Share<HazeCompilerValue>>& data,
	const HazeCompilerValue* value, HString& outName, bool getOffsets, V_Array<uint64>* offsets)
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
			auto compilerClass = DynamicCast<HazeCompilerClassValue>(data.second);
			if (compilerClass->GetMemberName(value, outName, getOffsets, offsets))
			{
				outName = GetLocalVariableName(data.first, data.second) + outName;
				if (!value->IsClassPublicMember())
				{
					HAZE_LOG_ERR_W("不能够访问类<%s>非公开成员变量<%s>!\n", compilerClass->GetOwnerClassName().c_str(), outName.c_str());
					return false;
				}
				return true;
			}
		}
	}
	else if (value->IsArrayElement())
	{
		auto arrayElement = static_cast<const HazeCompilerArrayElementValue*>(value);
		if (data.second->IsArray())
		{
			if (arrayElement->GetArray() == data.second.get())
			{
				outName = GetLocalVariableName(data.first, data.second);
				return true;
			}
		}
	}

	return false;
}

Share<HazeCompilerValue> GetArrayElementToValue(HazeCompilerModule* compilerModule, Share<HazeCompilerValue> elementValue,
	Share<HazeCompilerValue> movToValue)
{
	auto compiler = compilerModule->GetCompiler();
	//auto arrayPointer = compiler->CreatePointerToArrayElement(elementValue);

	//return compiler->CreateMovPV(movToValue ? movToValue : compiler->GetTempRegister(), nullptr);
	return nullptr;
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


void GenVariableHzic(HazeCompilerModule* compilerModule, HAZE_STRING_STREAM& hss,
	const Share<HazeCompilerValue>& value/*, int index*/)
{
	static HString s_StrName;

	bool find = false; 
	s_StrName.clear();
	if (value->IsGlobalVariable())
	{
		find = HazeCompilerModule::GetGlobalVariableName(compilerModule, value, s_StrName);
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

Share<HazeCompilerValue> GenIRCode_GetClassMember(HAZE_STRING_STREAM& hss, HazeCompilerModule* m, Share<HazeCompilerValue> v)
{
	static V_Array<uint64> s_Offsets;
	static HString s_Name;

	s_Offsets.clear();
	s_Name.clear();
	auto compiler = m->GetCompiler();

	bool find = false;
	if (v->IsGlobalVariable())
	{
		find = m->GetGlobalVariableName(m, v, s_Name, true, &s_Offsets);
	}
	else if (v->IsLocalVariable())
	{
		find = m->GetCurrFunction()->FindLocalVariableName(v.get(), s_Name, true, &s_Offsets);
	}
	else
	{
		COMPILER_ERR_MODULE_W("查找类成员偏移错误， 变量类型错误", m->GetName().c_str());
	}

	if (find)
	{
		auto cacheType = v->GetValueType();
		for (int i = s_Offsets.size() - 1; i >= 0; i--)
		{
			if (i != s_Offsets.size() - 1)
			{
				//v = compiler->CreateMovPV(compiler->GetTempRegister(), v);
				auto& type = const_cast<HazeDefineType&>(v->GetValueType());
				type.Pointer();

				v = compiler->CreateAdd(v, v, compiler->GetConstantValueUint64(s_Offsets[i]));

				if (i == 0)
				{
					//if (s_Offsets.size() <= 2)
					{
						auto& type = const_cast<HazeDefineType&>(v->GetValueType());
						type.UpToRefrence();
					}
					//v = compiler->CreateLea(v, v);
				}
				else
				{
					v = compiler->CreateMovPV(v, v);
				}
			}
			else
			{
				Share<HazeCompilerValue> startVaule = nullptr;
				if (v->IsGlobalVariable())
				{
					startVaule = m->GetGlobalVariable(m, s_Name);
				}
				else if (v->IsLocalVariable())
				{
					startVaule = m->GetCurrFunction()->GetLocalVariable(s_Name);
				}

				if (startVaule)
				{
					v = compiler->CreateMov(compiler->GetTempRegister(startVaule->GetValueType()), startVaule);
				}
				else
				{
					COMPILER_ERR_MODULE_W("查找类成员偏移错误, 未能找到变量<%s>", s_Name.c_str());
					return v;
				}

			}
		}

		return v;
	}
	else
	{
		COMPILER_ERR_MODULE_W("查找类成员偏移错误, 未能找到", m->GetName().c_str());
		return v;
	}

}

void GenIRCode(HAZE_STRING_STREAM& hss, HazeCompilerModule* m, InstructionOpCode opCode, Share<HazeCompilerValue> assignTo, Share<HazeCompilerValue> oper1,
	Share<HazeCompilerValue> oper2, const HazeDefineType* expectType)
{
	if (assignTo)
	{
		if (assignTo->IsClassMember())
		{
			assignTo = GenIRCode_GetClassMember(hss, m, assignTo);
		}
		else
		{
			auto arrElement = DynamicCast<HazeCompilerArrayElementValue>(oper1);
			V_Array<Share<HazeCompilerValue>> params;
			for (auto& i : arrElement->GetIndex())
			{
				params.push_back(i->GetShared());
			}
			assignTo = m->GetCompiler()->CreateAdvanceTypeFunctionCall(HazeValueType::Array, H_TEXT("设置"),
				params, arrElement->GetArray()->GetShared());
		}
	}

	if (oper1)
	{
		if (oper1->IsClassMember())
		{
			oper1 = GenIRCode_GetClassMember(hss, m, oper1);
		}
		else if (oper1->IsArrayElement())
		{
			auto arrElement = DynamicCast<HazeCompilerArrayElementValue>(oper1);
			V_Array<Share<HazeCompilerValue>> params;
			for (auto& i : arrElement->GetIndex())
			{
				params.push_back(i->GetShared());
			}
			oper1 = m->GetCompiler()->CreateAdvanceTypeFunctionCall(HazeValueType::Array, H_TEXT("获得"),
				params, arrElement->GetArray()->GetShared());
		}
	}

	if (oper2)
	{
		if (oper2->IsClassMember())
		{
			oper2 = GenIRCode_GetClassMember(hss, m, oper2);
		}
		else if (oper2->IsArrayElement())
		{
			auto arrElement = DynamicCast<HazeCompilerArrayElementValue>(oper2);
			V_Array<Share<HazeCompilerValue>> params;
			for (auto& i : arrElement->GetIndex())
			{
				params.push_back(i->GetShared());
			}
			oper2 = m->GetCompiler()->CreateAdvanceTypeFunctionCall(HazeValueType::Array, H_TEXT("获得"),
				params, arrElement->GetArray()->GetShared());
		}
	}

	if (expectType && IsMultiVariableTye(expectType->PrimaryType))
	{
		auto type = *expectType;
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
		}

		oper1 = m->GetCompiler()->CreateMov(m->GetCompiler()->GetTempRegister(type), oper1);
	}
	
	switch (opCode)
	{
	case InstructionOpCode::NONE:
		break;
	case InstructionOpCode::MOV:
	{
		/*if (v1->IsArrayElement())
		{
			auto arrElement = DynamicCast<HazeCompilerArrayElementValue>(v1);
			V_Array<Share<HazeCompilerValue>> params;
			for (auto& i : arrElement->GetIndex())
			{
				params.push_back(i->GetShared());
			}
			m->GetCompiler()->CreateAdvanceTypeFunctionCall(HazeValueType::Array, H_TEXT("设置"),
				params, arrElement->GetArray()->GetShared());
		}
		else
		{
			hss << GetInstructionString(opCode) << " ";
			GenVariableHzic(m, hss, v1);
			if (v2)
			{
				hss << " ";
				GenVariableHzic(m, hss, v2);
			}
		}*/
	}
		break;
	case InstructionOpCode::MOVPV:
	case InstructionOpCode::MOVTOPV:
	case InstructionOpCode::LEA:
	case InstructionOpCode::ADD:
	case InstructionOpCode::SUB:
	case InstructionOpCode::MUL:
	case InstructionOpCode::DIV:
	case InstructionOpCode::MOD:
	case InstructionOpCode::CVT:
	case InstructionOpCode::BIT_AND:
	case InstructionOpCode::BIT_OR:
	case InstructionOpCode::BIT_XOR:
	case InstructionOpCode::SHL:
	case InstructionOpCode::SHR:
	case InstructionOpCode::CMP:
	{
		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(m, hss, oper1);
		if (oper2)
		{
			hss << " ";
			GenVariableHzic(m, hss, oper2);
		}
	}
		break;
	case InstructionOpCode::NEG:
		break;
	case InstructionOpCode::NOT:
		break;
	case InstructionOpCode::INC:
		break;
	case InstructionOpCode::DEC:
		break;
	case InstructionOpCode::BIT_NEG:
		break;
	case InstructionOpCode::PUSH:
	{
		hss << GetInstructionString(opCode) << " ";
		GenVariableHzic(m, hss, oper1);
	}
		break;
	case InstructionOpCode::POP:
		break;
	case InstructionOpCode::CALL:
		break;
	case InstructionOpCode::RET:
		break;
	case InstructionOpCode::NEW:
	{
		hss << GetInstructionString(opCode) << " ";
		hss << NEW_REGISTER << " ";
		HazeDefineType::StringStreamTo(hss, *expectType);
		hss << " " << CAST_SCOPE(HazeVariableScope::Local) << " " << CAST_DESC(HazeDataDesc::RegisterNew) << " ";

		GenVariableHzic(m, hss, oper2);
	}
		break;
	case InstructionOpCode::JMP:
		break;
	case InstructionOpCode::JNE:
		break;
	case InstructionOpCode::JNG:
		break;
	case InstructionOpCode::JNL:
		break;
	case InstructionOpCode::JE:
		break;
	case InstructionOpCode::JG:
		break;
	case InstructionOpCode::JL:
		break;
	case InstructionOpCode::LINE:
		break;
	case InstructionOpCode::SIGN:
	{
		hss << GetInstructionString(opCode) << " ";
		hss << oper1->GetValue().Value.UInt64;
	}
		break;
	default:
		break;
	}

	hss << std::endl;
}