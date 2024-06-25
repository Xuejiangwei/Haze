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

HAZE_STRING GetHazeClassFunctionName(const HAZE_STRING& className, const HAZE_STRING& functionName)
{
	return className + HAZE_CLASS_FUNCTION_CONBINE + functionName;
}

HAZE_STRING GetLocalVariableName(const HAZE_STRING& name, std::shared_ptr<HazeCompilerValue> value)
{
	static HAZE_STRING_STREAM s_Hss;

	s_Hss.str(HAZE_TEXT(""));
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

void HazeCompilerStream(HAZE_STRING_STREAM& hss, std::shared_ptr<HazeCompilerValue> value, bool bStreamValue)
{
	HazeCompilerStream(hss, value.get(), bStreamValue);
}

std::shared_ptr<HazeCompilerValue> CreateVariableImpl(HazeCompilerModule* compilerModule, const HazeDefineType& type, 
	HazeVariableScope scope, HazeDataDesc desc, int count, std::shared_ptr<HazeCompilerValue> assignValue,
	std::vector<std::shared_ptr<HazeCompilerValue>> arraySize, std::vector<HazeDefineType>* params)
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
		return std::make_shared<HazeCompilerValue>(compilerModule, type, scope, desc, count, assignValue);
	case HazeValueType::MultiVariable:
	{
		return std::make_shared<HazeCompilerValue>(compilerModule, type, scope, desc, count, assignValue);
	}
	case HazeValueType::ArrayBase:
	case HazeValueType::ArrayClass:
	case HazeValueType::ArrayPointer:
		return std::make_shared<HazeCompilerArrayValue>(compilerModule, type, scope, desc, count, arraySize);
	case HazeValueType::PointerBase:
	case HazeValueType::PointerClass:
	case HazeValueType::PointerPointer:
		return std::make_shared<HazeCompilerPointerValue>(compilerModule, type, scope, desc, count);
	case HazeValueType::PointerFunction:
		return std::make_shared<HazeCompilerPointerFunction>(compilerModule, type, scope, desc, count, params ? params : nullptr);
	case HazeValueType::ReferenceBase:
	case HazeValueType::ReferenceClass:
		if (assignValue || compilerModule->IsBeginCreateFunctionVariable())
		{
			return std::make_shared<HazeCompilerRefValue>(compilerModule, type, scope, desc, count, assignValue);
		}
		else
		{
			HAZE_LOG_ERR(HAZE_TEXT("创建引用变量失败，必须赋予初始化值!\n"));
			return nullptr;
		}
	case HazeValueType::Class:
	{
		if (params)
		{
			compilerModule->ResetTemplateClassRealName(const_cast<HAZE_STRING&>(type.CustomName), *params);
			return std::make_shared<HazeCompilerClassValue>(compilerModule, type, scope, desc, count);
		}
		else
		{
			return std::make_shared<HazeCompilerClassValue>(compilerModule, type, scope, desc, count);
		}
	}
	case HazeValueType::Enum:
		return std::make_shared<HazeCompilerEnumValue>(compilerModule, type, scope, desc, count);
	default:
		break;
	}

	return nullptr;
}

std::shared_ptr<HazeCompilerValue> CreateVariable(HazeCompilerModule* compilerModule, const HazeDefineVariable& var, HazeVariableScope scope,
	HazeDataDesc desc, int count, std::shared_ptr<HazeCompilerValue> refValue, std::vector<std::shared_ptr<HazeCompilerValue>> arraySize,
	std::vector<HazeDefineType>* params)
{
	return CreateVariableImpl(compilerModule, var.Type, scope, desc, count, refValue, arraySize, params);
}

std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> CreateVariableCopyClassMember(HazeCompilerModule* compilerModule,
	HazeVariableScope scope, HazeCompilerClass* compilerClass)
{
	std::vector<std::pair<HazeDataDesc, std::vector<std::shared_ptr<HazeCompilerValue>>>> members;
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
				std::vector<std::shared_ptr<HazeCompilerValue>>{},
				var->IsPointerFunction() ? &const_cast<std::vector<HazeDefineType>&>(std::dynamic_pointer_cast<HazeCompilerPointerFunction>(var)->GetParamTypes()) : nullptr);
		}
	}

	return members;
}

//std::shared_ptr<HazeCompilerValue> CreateVariable(const HazeValue& Var, HazeDataDesc Scope)
//{
//	return std::make_shared<HazeCompilerValue>(Var, Scope);
//}

void StreamPointerValue(HAZE_STRING_STREAM& hss, std::shared_ptr<HazeCompilerValue> value)
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

void StreamClassValue(HAZE_STRING_STREAM& hss, std::shared_ptr<HazeCompilerValue> value)
{
	auto classValue = std::dynamic_pointer_cast<HazeCompilerClassValue>(value);
	hss << " " << classValue->GetOwnerClassName();
}

void StreamCompilerValue(HAZE_STRING_STREAM& hss, InstructionOpCode insCode, std::shared_ptr<HazeCompilerValue> value, const HAZE_CHAR* defaultName)
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

HAZE_STRING GetObjectName(const HAZE_STRING& inName)
{
	size_t pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HAZE_STRING::npos)
	{
		return inName.substr(0, pos);
	}
	else
	{
		pos = inName.find(HAZE_CLASS_ATTR);
		if (pos != HAZE_STRING::npos)
		{
			return inName.substr(0, pos);
		}
	}

	return inName;
}

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HAZE_STRING& inName)
{
	bool isPointer;
	return GetObjectMember(compilerModule, inName, isPointer);
}

std::shared_ptr<HazeCompilerValue> GetObjectMember(HazeCompilerModule* compilerModule, const HAZE_STRING& inName, bool& isPointer)
{
	HAZE_STRING objectName;
	HAZE_STRING memberName;
	return GetObjectNameAndMemberName(compilerModule, inName, objectName, memberName, isPointer);
}

std::shared_ptr<HazeCompilerValue> GetObjectNameAndMemberName(HazeCompilerModule* compilerModule, const HAZE_STRING& inName, 
	HAZE_STRING& outObjectName, HAZE_STRING& outMemberName, bool& isPointer)
{
	auto pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HAZE_STRING::npos)
	{
		isPointer = true;
		outObjectName = inName.substr(0, pos);
		outMemberName = inName.substr(pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

		auto pointerValue = std::dynamic_pointer_cast<HazeCompilerPointerValue>(compilerModule->GetCurrFunction()->GetLocalVariable(outObjectName));
		auto compilerClass = compilerModule->GetClass(pointerValue->GetValueType().CustomName);

		std::shared_ptr<HazeCompilerClassValue> classValue = nullptr;
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
		if (pos != HAZE_STRING::npos)
		{
			isPointer = false;
			outObjectName = inName.substr(0, pos);
			outMemberName = inName.substr(pos + HAZE_STRING(HAZE_CLASS_ATTR).size());

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

std::shared_ptr<HazeCompilerFunction> GetObjectFunction(HazeCompilerModule* compilerModule, const HAZE_STRING& inName)
{
	bool isPointer;
	return GetObjectFunction(compilerModule, inName, isPointer).first;
}

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> GetObjectFunction(HazeCompilerModule* compilerModule, 
	const HAZE_STRING& inName, bool& isPointer)
{
	HAZE_STRING objectName;
	HAZE_STRING functionName;
	return GetObjectNameAndFunctionName(compilerModule, inName, objectName, functionName, isPointer);
}

std::pair<std::shared_ptr<HazeCompilerFunction>, std::shared_ptr<HazeCompilerValue>> GetObjectNameAndFunctionName(HazeCompilerModule* compilerModule,
	const HAZE_STRING& inName, HAZE_STRING& outObjectName, HAZE_STRING& outFunctionName, bool& isPointer)
{
	std::shared_ptr<HazeCompilerValue> findVariable = nullptr;
	auto pos = inName.find(HAZE_CLASS_POINTER_ATTR);
	if (pos != HAZE_STRING::npos)
	{
		isPointer = true;
		outObjectName = inName.substr(0, pos);
		outFunctionName = inName.substr(pos + HAZE_STRING(HAZE_CLASS_POINTER_ATTR).size());

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
		if (pos != HAZE_STRING::npos)
		{
			isPointer = false;
			outObjectName = inName.substr(0, pos);
			outFunctionName = inName.substr(pos + HAZE_STRING(HAZE_CLASS_ATTR).size());

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
		}
	}

	return { nullptr, nullptr };
}

bool TrtGetVariableName(HazeCompilerFunction* function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& data,
	const std::shared_ptr<HazeCompilerValue>& value, HAZE_STRING& outName)
{
	return TrtGetVariableName(function, data, value.get(), outName);
}

bool TrtGetVariableName(HazeCompilerFunction* function, const std::pair<HAZE_STRING, std::shared_ptr<HazeCompilerValue>>& data,
	const HazeCompilerValue* value, HAZE_STRING& outName)
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
					HAZE_LOG_ERR(HAZE_TEXT("不能够访问类<%s>非公开成员变量<%s>!\n"), compilerClass->GetOwnerClassName().c_str(), outName.c_str());
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

std::shared_ptr<HazeCompilerValue> GetArrayElementToValue(HazeCompilerModule* compilerModule, std::shared_ptr<HazeCompilerValue> elementValue,
	std::shared_ptr<HazeCompilerValue> movToValue)
{
	auto compiler = compilerModule->GetCompiler();
	auto arrayPointer = compiler->CreatePointerToArrayElement(elementValue);

	return compiler->CreateMovPV(movToValue ? movToValue : compiler->GetTempRegister(), arrayPointer);
}

void GetTemplateClassName(HAZE_STRING& inName, const std::vector<HazeDefineType>& templateTypes)
{
	for (auto& type : templateTypes)
	{
		if (type.HasCustomName())
		{
			inName += (HAZE_TEXT("__") + type.CustomName);
			if (IsPointerType(type.SecondaryType))
			{
				inName += HAZE_TEXT("*");
			}
		}
		else
		{
			if (IsPointerType(type.PrimaryType))
			{
				inName += (HAZE_STRING(HAZE_TEXT("__")) + GetHazeValueTypeString(type.SecondaryType));
				inName += HAZE_TEXT("*");
			}
			else
			{
				inName += (HAZE_STRING(HAZE_TEXT("__")) + GetHazeValueTypeString(type.PrimaryType));
			}

		}
	}
}
