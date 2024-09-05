#include "HazePch.h"
#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeStream.h"
#include "ObjectString.h"

static HashMap<HString, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ H_TEXT("打印"), &HazeStream::HazePrintf },
	{ H_TEXT("输入"), &HazeStream::HazeScanf },
};

static bool Z_NoUse_HazeStream = HazeStandardLibraryBase::AddStdLib(H_TEXT("HazeStream"), &s_HashMap_Functions);

const HChar* HazeStream::GetFormat(const HChar* strfrmt, HChar* form)
{
#define L_FMTFLAGSF	H_TEXT("-+#0 123456789.")
#define MAX_FORMAT 32

	/* spans flags, width, and precision ('0' is included as a flag) */
	size_t len = wcsspn(strfrmt, L_FMTFLAGSF "123456789.");
	if (len >= MAX_FORMAT - 10)
	{
		return nullptr;
	}

	*(form++) = HChar('%');
	memcpy(form, strfrmt, len * sizeof(HChar));
	*(form + len) = '\0';
	return strfrmt + len - 1;
}

void HazeStream::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(H_TEXT("标准流"), &s_HashMap_Functions);
}

HString HazeStream::GetFormatString(HAZE_STD_CALL_PARAM)
{
#define PRE_SIGN HChar('%')

	uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE - sizeof(v);
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1 + 1;

	const HChar* str = ((ObjectString*)v)->GetData();
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
					hss << std::endl;
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
			HChar Form[MAX_FORMAT];
			start = GetFormat(start, Form);

			start++;
			if (*start == HChar('d'))
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
			else if (*start == HChar('f'))
			{
				offset -= sizeof(float);

				float tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(float));
				hss << tempV;
				start++;
			}
			else if (*start == HChar('c'))
			{
				offset -= sizeof(HChar);

				HChar tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(HChar));
				hss << tempV;
				start++;
			}
			else if (*start == HChar('p'))
			{
				uint64 pointerAddress;
				offset -= sizeof(pointerAddress);

				char* address = stack->GetAddressByEBP(offset);
				memcpy(&pointerAddress, address, sizeof(float));
				hss << (void*)pointerAddress;
				start++;
			}
			else if (*start == HChar('s'))
			{
				uint64 tempAddress;

				offset -= sizeof(tempAddress);

				char* Address = stack->GetAddressByEBP(offset);
				memcpy(&tempAddress, Address, sizeof(tempAddress));

				const HChar* tempStr = ((ObjectString*)tempAddress)->GetData();
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
			if (*start == HChar('\\'))
			{
				if (*(++start) == HChar('n'))
				{
					start++;
					hss << std::endl;
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
#define PRE_SIGN HChar('%')

	uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE;
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1;

	const HChar* str = ((ObjectString*)v)->GetData();
	auto start = str;
	while (*start != '\0')
	{
		if (*start != PRE_SIGN)
		{
			if (*start == HChar('\\'))
			{
				if (*(++start) == HChar('n'))
				{
					start++;
					hss << std::endl;
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
			HChar Form[MAX_FORMAT];
			start = GetFormat(start, Form);

			start++;
			if (*start == HChar('d'))
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
			else if (*start == HChar('f'))
			{
				offset -= sizeof(float);

				float tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(float));
				hss << tempV;
				start++;
			}
			else if (*start == HChar('c'))
			{
				offset -= sizeof(HChar);

				HChar tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(HChar));
				hss << tempV;
				start++;
			}
			else if (*start == HChar('p'))
			{
				uint64 pointerAddress;
				offset -= sizeof(pointerAddress);

				char* address = stack->GetAddressByEBP(offset);
				memcpy(&pointerAddress, address, sizeof(float));
				hss << (void*)pointerAddress;
				start++;
			}
			else if (*start == HChar('s'))
			{
				uint64 tempAddress;

				offset -= sizeof(tempAddress);

				char* Address = stack->GetAddressByEBP(offset);
				memcpy(&tempAddress, Address, sizeof(tempAddress));

				const HChar* tempStr = ((ObjectString*)tempAddress)->GetData();
				hss << tempStr;
				start++;
			}
		}
	}

	HazePrintfCall(hss.str().c_str());
}

void HazeStream::HazePrintfCall(const HChar* V)
{
	std::wcout << V;
}

void HazeStream::HazeScanf(HAZE_STD_CALL_PARAM)
{
#define PRE_SIGN HChar('%')

	uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE;
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1;

	const HChar* str = ((ObjectString*)v)->GetData();
	auto Start = str;

	uint64 Address = 0;

	while (*Start != '\0')
	{
		if (*Start != PRE_SIGN)
		{
			/*if (*Start == HChar('\\'))
			{
				if (*(++Start) == HChar('n'))
				{
					Start++;
					HSS << std::endl;
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
			HChar Form[MAX_FORMAT];
			Start = GetFormat(Start, Form);

			Start++;
			if (*Start == HChar('d'))
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
			else if (*Start == HChar('f'))
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