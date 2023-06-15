#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeStream.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> HashMap_Function =
{
	{ HAZE_TEXT("打印"), &HazeStream::HazePrintf },
	{ HAZE_TEXT("输入"), &HazeStream::HazeScanf },
};

static bool Z_NoUse_HazeStream = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeStream"), &HashMap_Function);

static const HAZE_CHAR* GetFormat(const HAZE_CHAR* strfrmt, HAZE_CHAR* form) 
{
#define L_FMTFLAGSF	HAZE_TEXT("-+#0 123456789.")
#define MAX_FORMAT 32

	/* spans flags, width, and precision ('0' is included as a flag) */
	size_t len = wcsspn(strfrmt, L_FMTFLAGSF "123456789.");
	if (len >= MAX_FORMAT - 10)
	{
		return nullptr;
	}

	*(form++) = HAZE_CHAR('%');
	memcpy(form, strfrmt, len * sizeof(HAZE_CHAR));
	*(form + len) = '\0';
	return strfrmt + len - 1;
}

void HazeStream::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeStream"), &HashMap_Function);
}

void HazeStream::HazePrint(HAZE_STD_CALL_PARAM)
{
	int V = 0;
	int Size = 0;
	for (int i = (int)Data->Vector_Param.size() - 1; i >= 0; --i)
	{
		Size = GetSizeByHazeType(Data->Vector_Param[i].Type.PrimaryType);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_ADDRESS_SIZE), Size);
	}

	HazePrintCall(V);
}

void HazeStream::HazePrintCall(int V)
{
	std::cout << V << std::endl;
}

void HazeStream::HazePrintf(HAZE_STD_CALL_PARAM)
{
#define PRE_SIGN HAZE_CHAR('%')
	
	uint64 V = 0;

	int Offset = -(int)sizeof(V) - HAZE_ADDRESS_SIZE;
	memcpy(&V, Stack->GetAddressByEBP(Offset), sizeof(V));

	HAZE_STRING_STREAM HSS;

	int ArgNum = 1;

	const HAZE_STRING* String = (HAZE_STRING*)V;
	auto Start = String->cbegin();
	while (Start != String->cend())
	{
		if (*Start != PRE_SIGN)
		{
			if (*Start == HAZE_CHAR('\\'))
			{
				if (*(++Start) == HAZE_CHAR('n'))
				{
					Start++;
					HSS << std::endl;
				}
			}
			else
			{
				HSS << *(Start++);
			}
		}
		else if (*(++Start) == PRE_SIGN)
		{
			HSS << *(Start++);
		}
		else
		{
			if (++ArgNum > MultiParamNum) //入栈参数个数
			{
				HAZE_LOG_ERR(HAZE_TEXT("调用<打印>函数参数个数错误!\n"));
				return;
			}
			HAZE_CHAR Form[MAX_FORMAT];
			Start._Seek_to(GetFormat(Start._Unwrapped(), Form));
		
			Start++;
			if (*Start == HAZE_CHAR('d'))
			{
				auto Ins = Stack->GetVM()->GetInstruction()[Stack->GetCurrPC() - ArgNum - 1];
				int Size = GetSizeByType(Ins.Operator[0].Variable.Type, Stack->GetVM());

				Offset -= Size;
				if (Size == 1)
				{
					char TempV;
					char* Address = Stack->GetAddressByEBP(Offset);
					memcpy(&TempV, Address, Size);
					HSS << (int)TempV;
				}
				else
				{
					int TempV;
					char* Address = Stack->GetAddressByEBP(Offset);
					memcpy(&TempV, Address, Size);
					HSS << TempV;
				}
				
				Start++;
			}
			else if (*Start == HAZE_CHAR('f'))
			{
				Offset -= sizeof(float);

				float TempV;
				char* Address = Stack->GetAddressByEBP(Offset);
				memcpy(&TempV, Address, sizeof(float));
				HSS << TempV;
				Start++;
			}
			else if (*Start == HAZE_CHAR('s'))
			{
				uint64 TempAddress;

				Offset -= sizeof(TempAddress);

				char* Address = Stack->GetAddressByEBP(Offset);
				memcpy(&TempAddress, Address, sizeof(TempAddress));

				const HAZE_STRING* Str = (HAZE_STRING*)TempAddress;
				HSS << *Str;
				Start++;
			}
		}
	}

	HazePrintfCall(HSS.str().c_str());
}

void HazeStream::HazePrintfCall(const HAZE_CHAR* V)
{
	std::wcout << V;
}


void HazeStream::HazeScanf(HAZE_STD_CALL_PARAM)
{
#define PRE_SIGN HAZE_CHAR('%')

	uint64 V = 0;

	int Offset = -(int)sizeof(V) - HAZE_ADDRESS_SIZE;
	memcpy(&V, Stack->GetAddressByEBP(Offset), sizeof(V));

	HAZE_STRING_STREAM HSS;

	int ArgNum = 1;

	const HAZE_STRING* String = (HAZE_STRING*)V;
	auto Start = String->cbegin();

	uint64 Address = 0;

	while (Start != String->cend())
	{
		if (*Start != PRE_SIGN)
		{
			/*if (*Start == HAZE_CHAR('\\'))
			{
				if (*(++Start) == HAZE_CHAR('n'))
				{
					Start++;
					HSS << std::endl;
				}
			}
			else
			{
				HSS << *(Start++);
			}*/

			HAZE_LOG_ERR(HAZE_TEXT("输入函数只能输入格式字符!\n"));
			return;
		}
		else if (*(++Start) == PRE_SIGN)
		{
			HSS << *(Start++);
		}
		else
		{
			if (++ArgNum > MultiParamNum) //入栈参数个数
			{
				return;
			}
			HAZE_CHAR Form[MAX_FORMAT];
			Start._Seek_to(GetFormat(Start._Unwrapped(), Form));

			Start++;
			if (*Start == HAZE_CHAR('d'))
			{
				Offset -= sizeof(Address);

				int TempV;
				std::cin >> TempV;
				
				char* Addr = Stack->GetAddressByEBP(Offset);
				memcpy(&Address, Addr, sizeof(Address));
				memcpy((char*)Address, &TempV, sizeof(TempV));

				HSS << TempV;
				Start++;
			}
			else if (*Start == HAZE_CHAR('f'))
			{
				Offset -= sizeof(Address);

				float TempV;
				std::cin >> TempV;

				char* Addr = Stack->GetAddressByEBP(Offset);
				memcpy(&Address, Addr, sizeof(Address));
				memcpy((char*)Address, &TempV, sizeof(TempV));

				HSS << TempV;
				Start++;
			}
			/*else if (*Start == HAZE_CHAR('s'))
			{
				uint64 TempAddress;

				Offset -= sizeof(TempAddress);

				char* Address = Stack->GetAddressByEBP(Offset);
				memcpy(&TempAddress, Address, sizeof(TempAddress));

				const HAZE_STRING* Str = (HAZE_STRING*)TempAddress;
				HSS << *Str;
				Start++;
			}*/
		}
	}
}

void HazeStream::HazeScanfCall()
{
}