#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeLog.h"
#include "HazeStack.h"
#include "HazeStream.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ HAZE_TEXT("��ӡ"), &HazeStream::HazePrintf },
	{ HAZE_TEXT("����"), &HazeStream::HazeScanf },
};

static bool Z_NoUse_HazeStream = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeStream"), &s_HashMap_Functions);

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
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("��׼��"), &s_HashMap_Functions);
}

void HazeStream::HazePrintf(HAZE_STD_CALL_PARAM)
{
#define PRE_SIGN HAZE_CHAR('%')

	uint64 v = 0;

	int offset = -(int)sizeof(v) - HAZE_ADDRESS_SIZE;
	memcpy(&v, stack->GetAddressByEBP(offset), sizeof(v));

	HAZE_STRING_STREAM hss;

	int argNum = 1;

	const HAZE_STRING* str = (HAZE_STRING*)v;
	auto start = str->cbegin();
	while (start != str->cend())
	{
		if (*start != PRE_SIGN)
		{
			if (*start == HAZE_CHAR('\\'))
			{
				if (*(++start) == HAZE_CHAR('n'))
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
			if (++argNum > multiParamNum) //��ջ��������
			{
				HAZE_LOG_ERR(HAZE_TEXT("����<��ӡ>����������������!\n"));
				return;
			}
			HAZE_CHAR Form[MAX_FORMAT];
			start._Seek_to(GetFormat(start._Unwrapped(), Form));

			start++;
			if (*start == HAZE_CHAR('d'))
			{
				auto ins = stack->GetVM()->GetInstruction()[stack->GetCurrPC() - argNum - 1];
				int size = GetSizeByType(ins.Operator[0].Variable.Type, stack->GetVM());

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
			else if (*start == HAZE_CHAR('f'))
			{
				offset -= sizeof(float);

				float tempV;
				char* address = stack->GetAddressByEBP(offset);
				memcpy(&tempV, address, sizeof(float));
				hss << tempV;
				start++;
			}
			else if (*start == HAZE_CHAR('s'))
			{
				uint64 tempAddress;

				offset -= sizeof(tempAddress);

				char* Address = stack->GetAddressByEBP(offset);
				memcpy(&tempAddress, Address, sizeof(tempAddress));

				const HAZE_STRING* tempStr = (HAZE_STRING*)tempAddress;
				hss << *tempStr;
				start++;
			}
		}
	}

	HazePrintfCall(hss.str().c_str());
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
	memcpy(&V, stack->GetAddressByEBP(Offset), sizeof(V));

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

			HAZE_LOG_ERR(HAZE_TEXT("���뺯��ֻ�������ʽ�ַ�!\n"));
			return;
		}
		else if (*(++Start) == PRE_SIGN)
		{
			HSS << *(Start++);
		}
		else
		{
			if (++ArgNum > multiParamNum) //��ջ��������
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

				char* Addr = stack->GetAddressByEBP(Offset);
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

				char* Addr = stack->GetAddressByEBP(Offset);
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