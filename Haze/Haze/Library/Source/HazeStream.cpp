#include "HazePch.h"
#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeStream.h"
#include "ObjectString.h"
#include "ObjectDynamicClass.h"
#include "HazeMemory.h"

static HashMap<STDString, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ H_TEXT("打印"), &HazeStream::HazePrintf },
	{ H_TEXT("输入"), &HazeStream::HazeScanf },
	{ H_TEXT("字符格式化"), &HazeStream::HazeStringFormat },
	{ H_TEXT("打印函数栈"), &HazeStream::HazeLogStack },
	{ H_TEXT("生成动态类"), &HazeStream::CreateDynamicClass },
};

static bool Z_NoUse_HazeStream = HazeStandardLibraryBase::AddStdLib(H_TEXT("HazeStream"), &s_HashMap_Functions);

const x_HChar* HazeStream::GetFormat(const x_HChar* strfrmt, x_HChar* form)
{
#define L_FMTFLAGSF	H_TEXT("-+#0 123456789.")
#define MAX_FORMAT 32

	/* spans flags, width, and precision ('0' is included as a flag) */
	size_t len = wcsspn(strfrmt, L_FMTFLAGSF "123456789.");
	if (len >= MAX_FORMAT - 10)
	{
		return nullptr;
	}

	*(form++) = x_HChar('%');
	memcpy(form, strfrmt, len * sizeof(x_HChar));
	*(form + len) = '\0';
	return strfrmt + len - 1;
}

void HazeStream::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(H_TEXT("标准流"), &s_HashMap_Functions);
}

STDString HazeStream::GetObjectFormatString(HAZE_STD_CALL_PARAM)
{
	NO_PARAM_WARNING();

#define PRE_SIGN x_HChar('%')

	x_uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE - (int)sizeof(v);
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1 + 1;

	const x_HChar* str = ((ObjectString*)v)->GetData();
	auto start = str;
	while (*start != '\0')
	{
		if (*start != PRE_SIGN)
		{
			/*if (*start == HChar('\\'))
			{
				if (*(++start) == HChar('n'))
				{
					start++;
					hss << HAZE_ENDL;
				}
			}
			else*/
			{
				hss << *(start++);
			}
		}
		else if (*(++start) == PRE_SIGN)
		{
			hss << *(start++);
		}
		else
		{
			if (++argNum > multiParamNum + 1) //入栈参数个数
			{
				HAZE_LOG_ERR_W("调用<打印>函数参数个数错误!\n");
				break;
			}
			x_HChar Form[MAX_FORMAT];
			start = GetFormat(start, Form);

			start++;
			if (*start == x_HChar('d'))
			{
				auto ins = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - argNum - 1];
				int size = GetSizeByHazeType(ins.Operator[0].Type);

				offset -= size;
				if (size == 1)
				{
					char TempV;
					char* Address = stack->GetAddressByEBP(offset);
					memcpy(&TempV, Address, size);
					hss << (int)TempV;
				}
				else
				{
					int TempV;
					char* Address = stack->GetAddressByEBP(offset);
					memcpy(&TempV, Address, size);
					hss << TempV;
				}

				start++;
			}
			else if (*start == x_HChar('f'))
			{
				offset -= sizeof(float);

				float tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(float));
				hss << tempV;
				start++;
			}
			else if (*start == x_HChar('c'))
			{
				offset -= sizeof(x_HChar);

				x_HChar tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(x_HChar));
				hss << tempV;
				start++;
			}
			else if (*start == x_HChar('p'))
			{
				x_uint64 pointerAddress;
				offset -= sizeof(pointerAddress);

				char* address = stack->GetAddressByEBP(offset);
				memcpy(&pointerAddress, address, sizeof(float));
				hss << (void*)pointerAddress;
				start++;
			}
			else if (*start == x_HChar('s'))
			{
				x_uint64 tempAddress;

				offset -= sizeof(tempAddress);

				char* Address = stack->GetAddressByEBP(offset);
				memcpy(&tempAddress, Address, sizeof(tempAddress));

				const x_HChar* tempStr = ((ObjectString*)tempAddress)->GetData();
				hss << tempStr;
				start++;
			}
		}
	}

	return hss.str();
}

STDString HazeStream::GetFormatString(HAZE_STD_CALL_PARAM)
{
	NO_PARAM_WARNING();

#define PRE_SIGN x_HChar('%')

	x_uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE;
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1;

	const x_HChar* str = ((ObjectString*)v)->GetData();
	auto start = str;
	while (*start != '\0')
	{
		if (*start != PRE_SIGN)
		{
			if (*start == x_HChar('\\'))
			{
				if (*(++start) == x_HChar('n'))
				{
					start++;
					hss << HAZE_ENDL;
				}
			}
			else
			{
				hss << *(start++);
			}
		}
		else if (*(++start) == PRE_SIGN)
		{
			hss << *(start++);
		}
		else
		{
			if (++argNum > multiParamNum) //入栈参数个数
			{
				HAZE_LOG_ERR_W("调用<打印>函数参数个数错误!\n");
				break;
			}
			x_HChar Form[MAX_FORMAT];
			start = GetFormat(start, Form);

			start++;
			if (*start == x_HChar('d'))
			{
				int size = sizeof(int);

				offset -= size;
				if (size == 1)
				{
					char TempV;
					char* Address = stack->GetAddressByEBP(offset);
					memcpy(&TempV, Address, size);
					hss << (int)TempV;
				}
				else
				{
					int TempV;
					char* Address = stack->GetAddressByEBP(offset);
					memcpy(&TempV, Address, size);
					hss << TempV;
				}

				start++;
			}
			else if (*start == x_HChar('f'))
			{
				offset -= sizeof(float);

				float tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(float));
				hss << tempV;
				start++;
			}
			else if (*start == x_HChar('c'))
			{
				offset -= sizeof(x_HChar);

				x_HChar tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(x_HChar));
				hss << tempV;
				start++;
			}
			else if (*start == x_HChar('p'))
			{
				x_uint64 pointerAddress;
				offset -= sizeof(pointerAddress);

				char* address = stack->GetAddressByEBP(offset);
				memcpy(&pointerAddress, address, sizeof(float));
				hss << (void*)pointerAddress;
				start++;
			}
			else if (*start == x_HChar('s'))
			{
				x_uint64 tempAddress;
				offset -= sizeof(tempAddress);

				char* Address = stack->GetAddressByEBP(offset);
				memcpy(&tempAddress, Address, sizeof(tempAddress));

				const x_HChar* tempStr = ((ObjectString*)tempAddress)->GetData();
				hss << tempStr;
				start++;
			}
		}
	}
	
	return hss.str();
}

STDString HazeStream::FormatConstantString(const STDString& str)
{
	HAZE_STRING_STREAM hss;
	auto start = str.c_str();
	while (*start != '\0')
	{
		if (*start != PRE_SIGN)
		{
			if (*start == x_HChar('\\'))
			{
				if (*(++start) == x_HChar('n'))
				{
					start++;
					hss << HAZE_ENDL;
				}
				else
				{
					hss << *(start++);
				}
			}
			else
			{
				hss << *(start++);
			}
		}
		else
		{
			hss << *(start++);
		}
	}

	return hss.str();
}

void HazeStream::HazePrintf(HAZE_STD_CALL_PARAM)
{
	NO_PARAM_WARNING();

#define PRE_SIGN x_HChar('%')

	x_uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE;
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1;

	const x_HChar* str = ((ObjectString*)v)->GetData();
	auto start = str;
	while (*start != '\0')
	{
		if (*start != PRE_SIGN)
		{
			if (*start == x_HChar('\\'))
			{
				if (*(++start) == x_HChar('n'))
				{
					start++;
					hss << HAZE_ENDL;
				}
			}
			else
			{
				hss << *(start++);
			}
		}
		else if (*(++start) == PRE_SIGN)
		{
			hss << *(start++);
		}
		else
		{
			if (++argNum > multiParamNum) //入栈参数个数
			{
				HAZE_LOG_ERR_W("调用<打印>函数参数个数错误!\n");
				return;
			}
			x_HChar Form[MAX_FORMAT];
			start = GetFormat(start, Form);

			start++;
			if (*start == x_HChar('d'))
			{
				int size = sizeof(int);

				offset -= size;
				if (size == 1)
				{
					char TempV;
					char* Address = stack->GetAddressByEBP(offset);
					memcpy(&TempV, Address, size);
					hss << (int)TempV;
				}
				else
				{
					int TempV;
					char* Address = stack->GetAddressByEBP(offset);
					memcpy(&TempV, Address, size);
					hss << TempV;
				}

				start++;
			}
			else if (*start == x_HChar('f'))
			{
				offset -= sizeof(float);

				float tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(float));
				hss << tempV;
				start++;
			}
			else if (*start == x_HChar('c'))
			{
				offset -= sizeof(x_HChar);

				x_HChar tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(x_HChar));
				hss << tempV;
				start++;
			}
			else if (*start == x_HChar('p'))
			{
				x_uint64 pointerAddress;
				offset -= sizeof(pointerAddress);

				char* address = stack->GetAddressByEBP(offset);
				memcpy(&pointerAddress, address, sizeof(float));
				hss << (void*)pointerAddress;
				start++;
			}
			else if (*start == x_HChar('s'))
			{
				x_uint64 tempAddress;
				offset -= sizeof(tempAddress);

				char* Address = stack->GetAddressByEBP(offset);
				memcpy(&tempAddress, Address, sizeof(tempAddress));

				const x_HChar* tempStr = ((ObjectString*)tempAddress)->GetData();
				hss << tempStr;
				start++;
			}
		}
	}

	HazePrintfCall(hss.str().c_str());
}

void HazeStream::HazePrintfCall(const x_HChar* V)
{
	std::wcout << V;
}

void HazeStream::HazeScanf(HAZE_STD_CALL_PARAM)
{
	NO_PARAM_WARNING();

#define PRE_SIGN x_HChar('%')

	x_uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE;
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1;

	const x_HChar* str = ((ObjectString*)v)->GetData();
	auto Start = str;

	x_uint64 Address = 0;

	while (*Start != '\0')
	{
		if (*Start != PRE_SIGN)
		{
			/*if (*Start == HChar('\\'))
			{
				if (*(++Start) == HChar('n'))
				{
					Start++;
					HSS << HAZE_ENDL;
				}
			}
			else
			{
				HSS << *(Start++);
			}*/

			HAZE_LOG_ERR_W("输入函数只能输入格式字符!\n");
			return;
		}
		else if (*(++Start) == PRE_SIGN)
		{
			hss << *(Start++);
		}
		else
		{
			if (++argNum > multiParamNum) //入栈参数个数
			{
				return;
			}
			x_HChar Form[MAX_FORMAT];
			Start = GetFormat(Start, Form);

			Start++;
			if (*Start == x_HChar('d'))
			{
				offset -= sizeof(Address);

				int TempV;
				std::cin >> TempV;

				char* Addr = stack->GetAddressByEBP(offset);
				memcpy(&Address, Addr, sizeof(Address));
				memcpy((char*)Address, &TempV, sizeof(TempV));

				hss << TempV;
				Start++;
			}
			else if (*Start == x_HChar('f'))
			{
				offset -= sizeof(Address);

				float TempV;
				std::cin >> TempV;

				char* Addr = stack->GetAddressByEBP(offset);
				memcpy(&Address, Addr, sizeof(Address));
				memcpy((char*)Address, &TempV, sizeof(TempV));

				hss << TempV;
				Start++;
			}
			/*else if (*Start == HChar('s'))
			{
				uint64 TempAddress;

				Offset -= sizeof(TempAddress);

				char* Address = Stack->GetAddressByEBP(Offset);
				memcpy(&TempAddress, Address, sizeof(TempAddress));

				const HString* Str = (HString*)TempAddress;
				HSS << *Str;
				Start++;
			}*/
		}
	}
}

void HazeStream::HazeScanfCall()
{
}

void HazeStream::HazeStringFormat(HAZE_STD_CALL_PARAM)
{
	NO_PARAM_WARNING();
	
	if (stack)
	{

	}
}

void HazeStream::HazeStringFormatCall()
{

}

void HazeStream::HazeLogStack(HAZE_STD_CALL_PARAM)
{
	NO_PARAM_WARNING();
	stack->LogStack();
}

V_Array<TestDynamic> g_TestDynamics;
void TestConstructor(void* ptr)
{
	std::cout << "constructor : " << ptr << HAZE_ENDL;
}

void TestDeconstructor(void* ptr)
{
	std::cout << "deconstructor : " << ptr << HAZE_ENDL;
}

void TestGetMember(HazeStack* stack, const STDString& name, void* obj)
{
	HAZE_LOG_INFO(H_TEXT("get member %s\n"), name.c_str());
	//SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::Int32), ((TestDynamic*)obj)->value);
}

void TestSetMember(HazeStack* stack, const STDString& name, void* obj, x_uint8* currESP)
{
	assert(stack);

	HAZE_LOG_INFO(H_TEXT("set member %s\n"), name.c_str());
	auto size = sizeof(((TestDynamic*)obj)->value);
	memcpy(&((TestDynamic*)obj)->value, currESP - size, size);
}

void TestCallFunction(HazeStack* stack, const STDString& name, void* obj, x_uint8* currESP)
{
	std::wcout << "call function : " << name.c_str() << HAZE_ENDL;
	int a, b;
	memcpy(&a, currESP - sizeof(a), sizeof(a));
	memcpy(&b, currESP - sizeof(a) - sizeof(b), sizeof(b));

	((TestDynamic*)obj)->Add(a, b);
	//SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::Void), obj);
}

void HazeStream::CreateDynamicClass(HAZE_STD_CALL_PARAM)
{
	GET_PARAM_START_WITH_RET();

	static ObjectDynamicClass::CustomMethods methods;
	methods.Constructor = &TestConstructor;
	methods.Deconstructor = &TestDeconstructor;
	methods.GetMember = &TestGetMember;
	methods.SetMember = &TestSetMember;
	methods.CallFunction = &TestCallFunction;

	auto address = nullptr;// HazeMemory::Alloca(sizeof(ObjectDynamicClass));
	/*auto testObj = new TestDynamic();
	new(address) ObjectDynamicClass(&methods, testObj);*/

	SET_RET_BY_TYPE(HAZE_VAR_TYPE(HazeValueType::DynamicClass), address);
}