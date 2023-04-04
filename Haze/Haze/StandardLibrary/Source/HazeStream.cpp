#include <stdio.h>
#include <windows.h>

#include "HazeVM.h"
#include "HazeStack.h"
#include "HazeStream.h"

static std::unordered_map<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> HashMap_Function =
{
	{ HAZE_TEXT("��ӡ"), &HazeStream::HazePrintf },
	{ HAZE_TEXT("��ӡ�ַ�"), &HazeStream::HazePrintString },
	{ HAZE_TEXT("��ӡС��"), &HazeStream::HazePrintFloat },
};

static bool Z_NoUse_HazeStream = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("HazeStream"), &HashMap_Function);

//void HazeStream::HazePrint(const HAZE_CHAR* Format, ...)
//{
//	HAZE_STRING WS;
//	WS = Format;
//	std::string S = WString2String(WS);
//
//	va_list st;
//	va_start(st, Format);
//	vfprintf(stdout, S.c_str(), st);
//	va_end(st);
//}

static const HAZE_CHAR* GetFormat(const HAZE_CHAR* strfrmt, HAZE_CHAR* form) {

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

void HazeStream::HazePrintFloat(HAZE_STD_CALL_PARAM)
{
	float V = 0;
	int Size = 0;
	for (int i = (int)Data->Vector_Param.size() - 1; i >= 0; --i)
	{
		Size = GetSizeByHazeType(Data->Vector_Param[i].Type.PrimaryType);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_ADDRESS_SIZE), Size);
	}

	HazePrintFloatCall(V);
}

void HazeStream::HazePrintFloatCall(float V)
{
	std::wcout << V << std::endl;
}

void HazeStream::HazePrintString(HAZE_STD_CALL_PARAM)
{
	int V = 0;
	int Size = 0;
	for (int i = (int)Data->Vector_Param.size() - 1; i >= 0; --i)
	{
		Size = GetSizeByHazeType(Data->Vector_Param[i].Type.PrimaryType);
		memcpy(&V, (void*)Stack->GetAddressByEBP(-Size - HAZE_ADDRESS_SIZE), Size);
	}

	HazePrintStringCall(Stack->GetVM()->GetHazeStringByIndex(V));
}

void HazeStream::HazePrintStringCall(const HAZE_STRING& V)
{
	std::wcout << V << std::endl;
}

void HazeStream::HazePrintf(HAZE_STD_CALL_PARAM)
{
#define PRE_SIGN HAZE_CHAR('%')
	
	decltype(InstructionData::Extra::Index) V = 0;

	int Offset = -(int)sizeof(V) - HAZE_ADDRESS_SIZE;
	memcpy(&V, Stack->GetAddressByEBP(Offset), sizeof(V));

	HAZE_STRING_STREAM HSS;

	int ArgNum = 1;

	const HAZE_STRING& String = Stack->GetVM()->GetHazeStringByIndex(V);
	auto Start = String.cbegin();
	while (Start != String.cend())
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
			if (++ArgNum > MultiParamNum) //��ջ��������
			{
				return;
			}
			HAZE_CHAR Form[MAX_FORMAT];
			Start._Ptr = GetFormat(Start._Ptr, Form);
		
			Start++;
			if (*Start == HAZE_CHAR('d'))
			{
				Offset -= sizeof(int);
				
				int TempV;
				char* Address = Stack->GetAddressByEBP(Offset);
				memcpy(&TempV, Address, sizeof(int));
				HSS << TempV;
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
				size_t TempAddress;

				Offset -= sizeof(TempAddress);

				char* Address = Stack->GetAddressByEBP(Offset);
				memcpy(&TempAddress, Address, sizeof(TempAddress));

				const HAZE_STRING& Str = Stack->GetVM()->GetHazeStringByIndex(TempAddress);
				HSS << Str;
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