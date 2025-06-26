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

static HashMap<HString, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ H_TEXT("��ӡ"), &HazeStream::HazePrintf },
	{ H_TEXT("����"), &HazeStream::HazeScanf },
	{ H_TEXT("�ַ���ʽ��"), &HazeStream::HazeStringFormat },
	{ H_TEXT("���ɶ�̬��"), &HazeStream::CreateDynamicClass },
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
	HazeStandardLibraryBase::AddStdLib(H_TEXT("��׼��"), &s_HashMap_Functions);
}

HString HazeStream::GetObjectFormatString(HAZE_STD_CALL_PARAM)
{
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
			if (++argNum > multiParamNum + 1) //��ջ��������
			{
				HAZE_LOG_ERR_W("����<��ӡ>����������������!\n");
				break;
			}
			x_HChar Form[MAX_FORMAT];
			start = GetFormat(start, Form);

			start++;
			if (*start == x_HChar('d'))
			{
				auto ins = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - argNum - 1];
				int size = ins.Operator[0].Variable.Type.GetTypeSize();

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

HString HazeStream::GetFormatString(HAZE_STD_CALL_PARAM)
{
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
			if (++argNum > multiParamNum) //��ջ��������
			{
				HAZE_LOG_ERR_W("����<��ӡ>����������������!\n");
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

HString HazeStream::FormatConstantString(const HString& str)
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
			if (++argNum > multiParamNum) //��ջ��������
			{
				HAZE_LOG_ERR_W("����<��ӡ>����������������!\n");
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

			HAZE_LOG_ERR_W("���뺯��ֻ�������ʽ�ַ�!\n");
			return;
		}
		else if (*(++Start) == PRE_SIGN)
		{
			hss << *(Start++);
		}
		else
		{
			if (++argNum > multiParamNum) //��ջ��������
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

}

void HazeStream::HazeStringFormatCall()
{

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

void TestGetMember(HazeStack* stack, const HString& name, void* obj)
{
	HAZE_LOG_INFO(H_TEXT("get member %s\n"), name.c_str());
	SET_RET_BY_TYPE(HazeValueType::Int32, ((TestDynamic*)obj)->value);
}

void TestSetMember(HazeStack* stack, const HString& name, void* obj, x_uint8* currESP)
{
	HAZE_LOG_INFO(H_TEXT("set member %s\n"), name.c_str());
	auto size = sizeof(((TestDynamic*)obj)->value);
	memcpy(&((TestDynamic*)obj)->value, currESP - size, size);
}

void TestCallFunction(HazeStack* stack, const HString& name, void* obj, x_uint8* currESP)
{
	std::wcout << "call function : " << name.c_str() << HAZE_ENDL;
	int a, b;
	memcpy(&a, currESP - sizeof(a), sizeof(a));
	memcpy(&b, currESP - sizeof(a) - sizeof(b), sizeof(b));

	((TestDynamic*)obj)->Add(a, b);
	SET_RET_BY_TYPE(HazeValueType::Void, obj);
}

void HazeStream::CreateDynamicClass(HAZE_STD_CALL_PARAM)
{
	static ObjectDynamicClass::CustomMethods methods;
	methods.Constructor = &TestConstructor;
	methods.Deconstructor = &TestDeconstructor;
	methods.GetMember = &TestGetMember;
	methods.SetMember = &TestSetMember;
	methods.CallFunction = &TestCallFunction;

	auto address = nullptr;// HazeMemory::Alloca(sizeof(ObjectDynamicClass));
	/*auto testObj = new TestDynamic();
	new(address) ObjectDynamicClass(&methods, testObj);*/

	SET_RET_BY_TYPE(HazeValueType::DynamicClass, address);
}