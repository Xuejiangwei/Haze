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
#include "CompilerElementValue.h"
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

Share<CompilerValue> CreateVariableCopyVar(CompilerModule* compilerModule, HazeVariableScope scope, Share<CompilerValue> var)
{
	return CreateVariableImpl(compilerModule, var->GetValueType(), scope, var->GetVariableDesc(), 0, var,
		var->IsArray() ? DynamicCast<CompilerArrayValue>(var)->GetArrayDimension() : 0,
		var->IsFunction() ? &const_cast<V_Array<HazeDefineType>&>(DynamicCast<CompilerPointerFunction>(var)->GetParamTypes()) : nullptr);
}

bool TrtGetVariableName(CompilerFunction* function, const Pair<HString, Share<CompilerValue>>& data,
	const CompilerValue* value, HString& outName)
{
	if (data.second.get() == value)
	{
		outName = GetLocalVariableName(data.first, data.second);
		return true;
	}

	/*if (value->IsClassMember())
	{
		if (data.second->IsClass())
		{
			auto compilerClass = DynamicCast<CompilerClassValue>(data.second);
			if (compilerClass->GetMemberName(value, outName))
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
	}*/

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
		find = compilerModule->GetCurrFunction()->FindLocalVariableName(value, s_StrName);
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
	Share<CompilerValue> oper1, Share<CompilerValue> oper2, const HazeDefineType* expectType, bool check)
{
	struct ScopeAssign
	{
		ScopeAssign(HAZE_STRING_STREAM& hss, CompilerModule* m)
			: Hss(hss), Module(m), Element(nullptr), AssignTo(nullptr), Value(nullptr) {}

		void SetElement(Share<CompilerElementValue> v, Share<CompilerValue> v1)
		{
			Element = v;
			Value = v1;
		}

		void SetAssignTo(Share<CompilerValue> v, Share<CompilerValue> v1)
		{
			AssignTo = v;
			Value = v1;
		}

		void InsertPreIRCode()
		{
			if (!Hss.str().empty())
			{
				Module->GetCompiler()->GetInsertBlock()->PushIRCode(Hss.str());
				Hss.str(H_TEXT(""));
			}
		}

		~ScopeAssign()
		{
			if (Element && Value)
			{
				InsertPreIRCode();

				//之后封装CreateSetArrayElement 和 CreateSetClassMember 到一个函数
				if (IsArrayType(Element->GetParentBaseType()))
				{
					Module->GetCompiler()->CreateSetArrayElement(Element->GetParent(), Element->GetElement(), Value);
				}
				else if (IsClassType(Element->GetParentBaseType()))
				{
					Module->GetCompiler()->CreateSetClassMember(Element->GetParent(), Element->GetElement(), Value);
				}
			}
			else if (AssignTo && Value)
			{
				InsertPreIRCode();
				Module->GetCompiler()->CreateMovToPV(AssignTo, Value);
			}
		}

	private:
		HAZE_STRING_STREAM& Hss;
		CompilerModule* Module;
		Share<CompilerElementValue> Element;
		Share<CompilerValue> AssignTo;
		Share<CompilerValue> Value;
	};

	Share<ScopeAssign> elementAssign = MakeShare<ScopeAssign>(hss, m);
	if (check)
	{
		Share<CompilerElementValue> assignElementValue = DynamicCast<CompilerElementValue>(assignTo);
		if (assignElementValue)
		{
			assignTo = m->GetCompiler()->GetTempRegister(assignElementValue->GetElement());
			elementAssign->SetElement(assignElementValue, assignTo);
		}
		else if (assignTo && assignTo->IsRefrence())
		{
			auto tempValue = m->GetCompiler()->GetTempRegister(assignTo->GetValueType().SecondaryType);
			elementAssign->SetAssignTo(assignTo, tempValue);
			assignTo = tempValue;
		}

		Share<CompilerElementValue> operElementValue1 = DynamicCast<CompilerElementValue>(oper1);
		if (operElementValue1)
		{
			oper1 = operElementValue1->CreateGetFunctionCall();
		}
		else if (oper1 && oper1->IsRefrence())
		{
			oper1 = m->GetCompiler()->CreateMovPV(m->GetCompiler()->GetTempRegister(oper1->GetValueType().SecondaryType), oper1);
		}
	
		Share<CompilerElementValue> operElementValue2 = DynamicCast<CompilerElementValue>(oper2);
		if (operElementValue2)
		{
			oper2 = operElementValue2->CreateGetFunctionCall();
		}
		else if (oper2 && oper2->IsRefrence())
		{
			oper2 = m->GetCompiler()->CreateMovPV(m->GetCompiler()->GetTempRegister(oper2->GetValueType().SecondaryType), oper2);
		}

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
					oper1 = m->GetCompiler()->CreateCVT(m->GetCompiler()->GetTempRegister(type), oper1);
				}
			}
			else if (IsNumberType(type.PrimaryType) && IsNumberType(oper1->GetValueType().PrimaryType))
			{
				auto strongerType = GetStrongerType(type.PrimaryType, oper1->GetValueType().PrimaryType);
				if (IsIntegerType(type.PrimaryType) && IsIntegerType(oper1->GetValueType().PrimaryType))
				{
					if (type != oper1->GetValueType())
					{
						oper1 = m->GetCompiler()->CreateCVT(m->GetCompiler()->GetTempRegister(strongerType), oper1);
					}
				}
				else if (IsFloatingType(type.PrimaryType) && IsFloatingType(oper1->GetValueType().PrimaryType))
				{
					if (type != oper1->GetValueType())
					{
						oper1 = m->GetCompiler()->CreateCVT(m->GetCompiler()->GetTempRegister(strongerType), oper1);
					}
				}
			}
			else if (IsRefrenceType(type.PrimaryType) && !oper1->IsRefrence())
			{
				type.PrimaryType = HazeValueType::UInt64;
				oper1 = m->GetCompiler()->CreateLea(m->GetCompiler()->GetTempRegister(type), oper1);
			}
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
		default:
			COMPILER_ERR_MODULE_W("生成<%s>中间代码错误, 中间操作码为空", GetInstructionString(opCode), m->GetName().c_str());
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
	Share<CompilerValue> pointerFunction, Share<CompilerValue> advancePointerTo, void* advanceFuncAddress, const HString* nameSpace)
{
	switch (opCode)
	{
	case InstructionOpCode::CALL:
	{
		HString varName;
		hss << GetInstructionString(InstructionOpCode::CALL) << " ";
		if (function)
		{
			auto desc = function->IsVirtualFunction() && !nameSpace ? HazeDataDesc::FunctionDynamicAddress : HazeDataDesc::FunctionAddress;
			auto& funcName = desc == HazeDataDesc::FunctionDynamicAddress ? function->GetName() : function->GetRealName();
			hss << funcName << " " << CAST_TYPE(HazeValueType::None) << " " << CAST_SCOPE(HazeVariableScope::Ignore) << " " <<
				CAST_DESC(desc) << " " << paramCount << " " << paramSize << " " << function->GetModule()->GetName() << " "
				<< CAST_DESC(HazeDataDesc::CallFunctionModule) << std::endl;
		}
		else if (pointerFunction)
		{
			m->GetGlobalVariableName(m, pointerFunction, varName);
			if (varName.empty())
			{
				m->GetCurrFunction()->FindLocalVariableName(pointerFunction, varName);
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
				m->GetCurrFunction()->FindLocalVariableName(advancePointerTo, varName);
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