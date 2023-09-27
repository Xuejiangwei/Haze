#include <thread>

#include "HazeDebuggerServer.h"
#include "HazeLog.h"
#include "HazeDebugger.h"

#include "HazeJson.h"

#ifdef _WIN32
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#endif

std::unique_ptr<HazeDebugger> g_Debugger;
uint64 g_SocketServer;
uint64 g_SocketClient;
std::thread::id g_DebuggerThreadId;

void CloseServer()
{
	closesocket(g_SocketServer);
	WSACleanup();
}

void HazeDebuggerServer::InitDebuggerServer(HazeVM* m_VM)
{
	std::thread LaunchDebuggerThread(HazeDebuggerServer::Start, m_VM);
	g_DebuggerThreadId = LaunchDebuggerThread.get_id();
	LaunchDebuggerThread.detach();
}

void HazeDebuggerServer::SendData(char* m_Data, int Length, int Flags)
{
	send(g_SocketClient, m_Data, Length, Flags);
}

void HazeDebuggerServer::Start(HazeVM* m_VM)
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
	g_SocketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_SocketServer == INVALID_SOCKET)
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
	if (bind(g_SocketServer, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Haze������Socket��ʧ��!\n"));
		return;
	}

	//��ʼ����
	if (listen(g_SocketServer, 5) == SOCKET_ERROR)
	{
		HAZE_LOG_ERR(HAZE_TEXT("Haze������Socket����ʧ��!\n"));
		return;
	}

	//ѭ����������

	while (m_VM)
	{
		sockaddr_in remoteAddr;				//sockaddr_in������socket����͸�ֵ,sockaddr���ں�������
		int nAddrlen = sizeof(remoteAddr);

		HAZE_LOG_INFO(HAZE_TEXT("Haze�������ȴ�Socket����...\n"));
		g_SocketClient = accept(g_SocketServer, (sockaddr*)&remoteAddr, &nAddrlen);
		if (g_SocketClient == INVALID_SOCKET)
		{
			HAZE_LOG_INFO(HAZE_TEXT("Haze������Socket����ʧ��!\n"));
			break;
		}
		HAZE_LOG_INFO(HAZE_TEXT("Haze���������յ�һ������<%s>!\n"), String2WString(inet_ntoa(remoteAddr.sin_addr)).c_str());

		if (!g_Debugger)
		{
			g_Debugger = std::make_unique<HazeDebugger>(m_VM, &CloseServer);
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
	uint32 m_Type = StringToStandardType<uint32>(Str);
	HAZE_LOG_INFO("handle message %s\n", Message);
	switch ((HazeDebugOperatorType)m_Type)
	{
	case HazeDebugOperatorType::None:
		HAZE_LOG_ERR(HAZE_TEXT("Haze���Խ��յ�<�յ���>����\n"));
		return false;
	case HazeDebugOperatorType::Start:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<��ʼ>����\n"));
		g_Debugger->Start();
		return true;
	case HazeDebugOperatorType::StepOver:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<�����ϵ�>����\n"));
		g_Debugger->StepOver();
		return true;
	case HazeDebugOperatorType::StepIn:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<��������>����\n"));
		g_Debugger->StepIn();
		return true;
	case HazeDebugOperatorType::StepInstruction:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<����ָ��>����\n"));
		g_Debugger->StepInstruction();
		return true;
	case HazeDebugOperatorType::AddBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<��Ӷϵ�>����\n"));
		g_Debugger->AddBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<ɾ���ϵ�>����\n"));
		g_Debugger->DeleteBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteModuleAllBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<ɾ��ģ�����жϵ�>����\n"));
		g_Debugger->DeleteModuleAllBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteAllBreakPoint:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<ɾ�����жϵ�>����\n"));
		g_Debugger->DeleteAllBreakPoint();
		return true;
	case HazeDebugOperatorType::Continue:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<����>����\n"));
		g_Debugger->Continue();
		return true;
	case HazeDebugOperatorType::GetLocalVariable:
		HAZE_LOG_INFO(HAZE_TEXT("Haze���Խ��յ�<������ʱ��������>����\n"));
		{
			HazeJson json;
			json["Type"] = (int)HazeDebugOperatorType::GetLocalVariable;
			g_Debugger->SetJsonLocalVariable(json);
			auto data = json.Encode();
			HazeDebuggerServer::SendData(data.data(), (int)data.length());
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
	while (g_Debugger)
	{
		int Ret = recv(g_SocketClient, TempRecvData, 1024, MSG_PEEK);
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
						recv(g_SocketClient, RecvData, index + 1 - preIndex, 0);
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
			Addr.S_un.S_addr = (decltype(Addr.S_un.S_addr))g_SocketClient;
			HAZE_LOG_INFO(HAZE_TEXT("�رյ�����Socket<%s>!\n"), String2WString(inet_ntoa(Addr)).c_str());
			break;
		}
		//��������
	}

	closesocket(g_SocketClient);
	WSACleanup();
}