#include "HazePch.h"
#include "HazeCompilerHelper.h"

#include "HazeCompiler.h"
#include "HazeCompilerModule.h"
#include "HazeCompilerClass.h"
#include "HazeCompilerFunction.h"
#include "HazeCompilerArrayValue.h"
#include "HazeCompilerPointerValue.h"
#include "HazeCompilerPointerFunction.h"
#include "HazeCompilerRefValue.h"
#include "HazeCompilerClassValue.h"
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
		switch (value->GetValueType().PrimaryType)
		{
		case HazeValueType::Bool:
			hss << v.Value.Bool;
			break;
		case HazeValueType::Byte:
			hss << v.Value.Byte;
			break;
		case HazeValueType::Char:
			hss << v.Value.Char;
			break;
		case HazeValueType::Int:
			hss << v.Value.Int;
			break;
		case HazeValueType::Float:
			hss << v.Value.Float;
			break;
		case HazeValueType::UnsignedInt:
			hss << v.Value.UnsignedInt;
			break;
		case HazeValueType::Long:
			hss << v.Value.Long;
			break;
		case HazeValueType::Double:
			hss << v.Value.Double;
			break;
		case HazeValueType::UnsignedLong:
			hss << v.Value.UnsignedLong;
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
	case HazeValueType::Byte:
	case HazeValueType::Char:
	case HazeValueType::Int:
	case HazeValueType::Float:
	case HazeValueType::Long:
	case HazeValueType::Double:
	case HazeValueType::UnsignedInt:
	case HazeValueType::UnsignedLong:
		return MakeShare<HazeCompilerValue>(compilerModule, type, scope, desc, count, assignValue);
	case HazeValueType::MultiVariable:
	{
		return MakeShare<HazeCompilerValue>(compilerModule, type, scope, desc, count, assignValue);
	}
	case HazeValueType::ArrayBase:
	case HazeValueType::ArrayClass:
	case HazeValueType::ArrayPointer:
		return MakeShare<HazeCompilerArrayValue>(compilerModule, type, scope, desc, count, arraySize);
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerPointer:
		return MakeShare<HazeCompilerPointerValue>(compilerModule, type, scope, desc, count);
	case HazeValueType::PointerFunction:
		return MakeShare<HazeCompilerPointerFunction>(compilerModule, type, scope, desc, count, params ? params : nullptr);
	case HazeValueType::ReferenceBase:
	case HazeValueType::ReferenceClass:
		if (assignValue || compilerModule->IsBeginCreateFunctionVariable())
		{
			return MakeShare<HazeCompilerRefValue>(compilerModule, type, scope, desc, count, assignValue);
		}
		else
		{
			HAZE_LOG_ERR(H_TEXT("创建引用变量失败，必须赋予初始化值!\n"));
			return nullptr;
		}
	case HazeValueType::Class:
	{
		if (params)
		{
			compilerModule->ResetTemplateClassRealName(const_cast<HString&>(type.CustomName), *params);
			return MakeShare<HazeCompilerClassValue>(compilerModule, type, scope, desc, count);
		}
		else
		{
			return MakeShare<HazeCompilerClassValue>(compilerModule, type, scope, desc, count);
		}
	}
	/*case HazeValueType::Enum:
		return MakeShare<HazeCompilerEnumValue>(compilerModule, type, scope, desc, count);*/
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
				var->IsRef() ? std::dynamic_pointer_cast<HazeCompilerRefValue>(var)->GetRefValue() : var,
				var->IsArray() ? std::dynamic_pointer_cast<HazeCompilerArrayValue>(var)->GetArraySize() :
				V_Array<Share<HazeCompilerValue>>{},
				var->IsPointerFunction() ? &const_cast<V_Array<HazeDefineType>&>(std::dynamic_pointer_cast<HazeCompilerPointerFunction>(var)->GetParamTypes()) : nullptr);
		}
	}

	return members;
}

//Share<HazeCompilerValue> CreateVariable(const HazeValue& Var, HazeDataDesc Scope)
//{
//	return MakeShare<HazeCompilerValue>(Var, Scope);
//}

void StreamPointerValue(HAZE_STRING_STREAM& hss, Share<HazeCompilerValue> value)
{
	auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(value);
	if (value->GetValueType().PrimaryType == HazeValueType::PointerBase)
	{
		hss << " " << (uint32)pointerValue->GetValueType().PrimaryType;
	}
	else
	{
		hss << " " << pointerValue->GetValueType().CustomName;
	}
}

void StreamClassValue(HAZE_STRING_STREAM& hss, Share<HazeCompilerValue> value)
{
	auto classValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(value);
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

	if (value->IsPointer())
	{
		StreamPointerValue(hss, value);
	}
	else if (value->IsClass())
	{
		StreamClassValue(hss, value);
	}

	hss << std::endl;
}

HString GetObjectName(const HString& inName)
{
	size_t pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HString::npos)
	{
		return inName.substr(0, pos);
	}
	else
	{
		pos = inName.find(HAZE_CLASS_ATTR);
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
	auto pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HString::npos)
	{
		isPointer = true;
		outObjectName = inName.substr(0, pos);
		outMemberName = inName.substr(pos + HString(HAZE_CLASS_POINTER_ATTR).size());

		auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName));
		auto compilerClass = compilerModule->GetClass(pointerValue->GetValueType().CustomName);

		Share<HazeCompilerClassValue> classValue = nullptr;
		if (outObjectName == HAZE_CLASS_THIS)
		{
			classValue = compilerClass->GetThisPointerToValue();
		}
		else
		{
			classValue = compilerClass->GetNewPointerToValue();
		}

		return classValue->GetMember(outMemberName);
	}
	else
	{
		pos = inName.find(HAZE_CLASS_ATTR);
		if (pos != HString::npos)
		{
			isPointer = false;
			outObjectName = inName.substr(0, pos);
			outMemberName = inName.substr(pos + HString(HAZE_CLASS_ATTR).size());

			auto classValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName));
			if (classValue)
			{
				return classValue->GetMember(outMemberName);
			}
			else
			{
				classValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(HazeCompilerModule::GetGlobalVariable(compilerModule, outObjectName));
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
	auto pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HString::npos)
	{
		isPointer = true;
		outObjectName = inName.substr(0, pos);
		outFunctionName = inName.substr(pos + HString(HAZE_CLASS_POINTER_ATTR).size());

		if (compilerModule->GetCurrFunction())
		{
			findVariable = compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName);
		}

		if (!findVariable)
		{
			findVariable = HazeCompilerModule::GetGlobalVariable(compilerModule, outObjectName);
		}

		auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(findVariable);
		if (pointerValue)
		{
			auto compilerClass = compilerModule->GetClass(pointerValue->GetValueType().CustomName);
			return { compilerClass->FindFunction(outFunctionName), findVariable };
		}
	}
	else
	{
		pos = inName.find(HAZE_CLASS_ATTR);
		if (pos != HString::npos)
		{
			isPointer = false;
			outObjectName = inName.substr(0, pos);
			outFunctionName = inName.substr(pos + HString(HAZE_CLASS_ATTR).size());

			if (compilerModule->GetCurrFunction())
			{
				findVariable = compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName);
			}

			if (!findVariable)
			{
				findVariable = HazeCompilerModule::GetGlobalVariable(compilerModule, outObjectName);
			}

			auto classValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(findVariable);
			if (classValue)
			{
				return { classValue->GetOwnerClass()->FindFunction(outFunctionName), findVariable };
			}
			else if (findVariable && findVariable->IsRefClass())
			{
				return { compilerModule->GetClass(findVariable->GetValueType().CustomName)->FindFunction(outFunctionName), findVariable };
			}
			else if (findVariable)
			{
				HAZE_LOG_ERR(H_TEXT("获得类对象<%s>的成员函数错误!\n"), outObjectName.c_str());
			}
		}
	}

	return { nullptr, nullptr };
}

bool TrtGetVariableName(HazeCompilerFunction* function, const Pair<HString, Share<HazeCompilerValue>>& data,
	const Share<HazeCompilerValue>& value, HString& outName)
{
	return TrtGetVariableName(function, data, value.get(), outName);
}

bool TrtGetVariableName(HazeCompilerFunction* function, const Pair<HString, Share<HazeCompilerValue>>& data,
	const HazeCompilerValue* value, HString& outName)
{
	if (data.second.get() == value)
	{
		outName = GetLocalVariableName(data.first, data.second);
		return true;
	}

	if (value->IsClassMember())
	{
		if (data.second->IsPointerClass())
		{
			if (function)
			{
				auto compilerClass = function->GetModule()->GetClass(data.second->GetValueType().CustomName);
				compilerClass->GetMemberName(value, outName);
				if (!outName.empty())
				{
					outName = GetLocalVariableName(data.first, data.second) + HAZE_CLASS_POINTER_ATTR + outName;
					return true;
				}
			}
		}
		else if (data.second->IsClass())
		{
			auto compilerClass = std::dynamic_pointer_cast<HazeCompilerClassValue>(data.second);
			compilerClass->GetMemberName(value, outName);
			if (!outName.empty())
			{
				outName = GetLocalVariableName(data.first, data.second) + HAZE_CLASS_ATTR + outName;
				if (!value->IsClassPublicMember())
				{
					HAZE_LOG_ERR(H_TEXT("不能够访问类<%s>非公开成员变量<%s>!\n"), compilerClass->GetOwnerClassName().c_str(), outName.c_str());
				}
				return true;
			}
		}
	}
	else if (value->IsArrayElement())
	{
		auto arrayElement = static_cast<const HazeCompilerArrayElementValue*>(value);
		if (data.second->IsArray() || data.second->IsPointer())
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
	auto arrayPointer = compiler->CreatePointerToArrayElement(elementValue);

	return compiler->CreateMovPV(movToValue ? movToValue : compiler->GetTempRegister(), arrayPointer);
}

void GetTemplateClassName(HString& inName, const V_Array<HazeDefineType>& templateTypes)
{
	for (auto& type : templateTypes)
	{
		if (type.HasCustomName())
		{
			inName += (H_TEXT("__") + type.CustomName);
			if (IsPointerType(type.SecondaryType))
			{
				inName += H_TEXT("*");
			}
		}
		else
		{
			if (IsPointerType(type.PrimaryType))
			{
				inName += (HString(H_TEXT("__")) + GetHazeValueTypeString(type.SecondaryType));
				inName += H_TEXT("*");
			}
			else
			{
				inName += (HString(H_TEXT("__")) + GetHazeValueTypeString(type.PrimaryType));
			}

		}
	}
}
