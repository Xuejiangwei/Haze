#include <thread>

#include "HazeDebuggerServer.h"
#include "HazeLog.h"
#include "HazeDebugger.h"
#include "HazeDebuggerBuffer.h"

#include "OpenJson/openjson.h"

#ifdef _WIN32
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#endif

std::unique_ptr<HazeDebugger> Debugger;
uint64 SocketServer;
uint64 SocketClient;
std::thread::id DebuggerThreadId;

void CloseServer()
{
	closesocket(SocketServer);
	WSACleanup();
}

void HazeDebuggerServer::InitDebuggerServer(HazeVM* VM)
{
	std::thread LaunchDebuggerThread(HazeDebuggerServer::Start, VM);
	DebuggerThreadId = LaunchDebuggerThread.get_id();
	LaunchDebuggerThread.detach();
}

void HazeDebuggerServer::SendData(char* Data, int Length, int Flags)
{
	send(SocketClient, Data, Length, Flags);
}

void HazeDebuggerServer::Start(HazeVM* VM)
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;

	//��ʼ��WSA
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Haze������Socket��ʼ��ʧ��!\n"));
		return;
	}

	//�����׽���
	SocketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketServer == INVALID_SOCKET)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Haze������Socket����ʧ��!\n"));
		return;
	}

	//��IP�Ͷ˿�
	sockaddr_in sin;						//ipv4��ָ��������ʹ��struct sockaddr_in���͵ı���
	sin.sin_family = AF_INET;
	sin.sin_port = htons(11003);			//���ö˿ڡ�htons��������unsigned short intת��Ϊ�����ֽ�˳��
	sin.sin_addr.S_un.S_addr = INADDR_ANY;	//IP��ַ���ó�INADDR_ANY����ϵͳ�Զ���ȡ������IP��ַ
	
	//bind������һ����ַ���е��ض���ַ����scket��
	if (bind(SocketServer, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Haze������Socket��ʧ��!\n"));
		return;
	}

	//��ʼ����
	if (listen(SocketServer, 5) == SOCKET_ERROR)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Haze������Socket����ʧ��!\n"));
		return;
	}

	//ѭ����������

	while (VM)
	{
		sockaddr_in remoteAddr;				//sockaddr_in������socket����͸�ֵ,sockaddr���ں�������
		int nAddrlen = sizeof(remoteAddr);

		HAZE_LOG_INFO(HAZE_TEXT("Haze�������ȴ�Socket����...\n"));
		SocketClient = accept(SocketServer, (sockaddr*)&remoteAddr, &nAddrlen);
		if (SocketClient == INVALID_SOCKET)
		{
			HAZE_LOG_INFO(HAZE_TEXT("Haze������Socket����ʧ��!\n"));
			break;
		}
		HAZE_LOG_INFO(HAZE_TEXT("Haze���������յ�һ������<%s>!\n"), String2WString(inet_ntoa(remoteAddr.sin_addr)).c_str());

		if (!Debugger)
		{
			Debugger = std::make_unique<HazeDebugger>(VM, &CloseServer);
		}
		break;
	}

	//��������
	Recv();
}

static bool HandleMessage(char* Message)
{
	Message = UTF8_2_GB2312(Message);
	std::string Str;
	Str += Message[0];
	uint32 Type = StringToStandardType<uint32>(Str);
	HAZE_LOG_INFO("handle message %s\n", Message);
	switch ((HazeDebugOperatorType)Type)
	{
	case HazeDebugOperatorType::None:
		HAZE_LOG_ERR(HAZE_TEXT("Haze���Խ��յ�<�յ���>����\n"));
		return false;
	case HazeDebugOperatorType::Start:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<��ʼ>����\n"));
		Debugger->Start();
		return true;
	case HazeDebugOperatorType::StepOver:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<�����ϵ�>����\n"));
		Debugger->StepOver();
		return true;
	case HazeDebugOperatorType::StepIn:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<��������>����\n"));
		Debugger->StepIn();
		return true;
	case HazeDebugOperatorType::StepInstruction:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<����ָ��>����\n"));
		Debugger->StepInstruction();
		return true;
	case HazeDebugOperatorType::AddBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<��Ӷϵ�>����\n"));
		Debugger->AddBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<ɾ���ϵ�>����\n"));
		Debugger->DeleteBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteModuleAllBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<ɾ��ģ�����жϵ�>����\n"));
		Debugger->DeleteModuleAllBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteAllBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<ɾ�����жϵ�>����\n"));
		Debugger->DeleteAllBreakPoint();
		return true;
	case HazeDebugOperatorType::Continue:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<����>����\n"));
		Debugger->Continue();
		return true;
	case HazeDebugOperatorType::GetLocalVariable:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<������ʱ��������>����\n"));
		{
			open::OpenJson json;
			json["Type"] = (int)HazeDebugOperatorType::GetLocalVariable;
			Debugger->SetJsonLocalVariable(json);
			auto data = json.encode();
			HazeDebuggerServer::SendData(data.data(), data.length());
		}
		return true;
	default:
		break;
	}

	return false;
}

void HazeDebuggerServer::Recv()
{
	char RecvData[1024];
	char TempRecvData[1024];
	while (Debugger)
	{
		int Ret = recv(SocketClient, TempRecvData, 1024, MSG_PEEK);
		if (Ret > 0)
		{
			if (Ret < 1024)
			{
				TempRecvData[Ret] = 0x00;
				int index = 0;
				int preIndex = 0;
				while (index < Ret)
				{
					while (TempRecvData[index] != 13 && index < Ret)
					{
						index++;
					}

					if (index < Ret && TempRecvData[index] == 13)
					{
						recv(SocketClient, RecvData, index + 1 - preIndex, 0);
						RecvData[index - preIndex] = 0x00;
						preIndex = index + 1;
						HandleMessage(RecvData);
					}

					index++;
				}
			}
			else
			{
				HAZE_LOG_ERR(HAZE_TEXT("���յ�Socket���ݴ�С����1024!\n"));
			}
		}
		else
		{
			in_addr Addr;
			Addr.S_un.S_addr = SocketClient;
			HAZE_LOG_INFO(HAZE_TEXT("�رյ�����Socket<%s>!\n"), String2WString(inet_ntoa(Addr)).c_str());
			break;
		}
		//��������
	}

	closesocket(SocketClient);
	WSACleanup();
}