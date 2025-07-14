#include "HazePch.h"
#include <thread>

#include "HazeDebuggerServer.h"
#include "HazeLog.h"
#include "HazeDebugger.h"

#include "XJson.h"

#ifdef _WIN32
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#endif

Unique<HazeDebugger> g_Debugger;
x_uint64 g_SocketServer;
x_uint64 g_SocketClient;
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

	//初始化WSA
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		HAZE_LOG_ERR_W("Haze调试器Socket初始化失败!\n");
		return;
	}

	//创建套接字
	g_SocketServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_SocketServer == INVALID_SOCKET)
	{
		HAZE_LOG_ERR_W("Haze调试器Socket创建失败!\n");
		return;
	}

	//绑定IP和端口
	sockaddr_in sin;						//ipv4的指定方法是使用struct sockaddr_in类型的变量
	sin.sin_family = AF_INET;
	sin.sin_port = htons(11003);			//设置端口。htons将主机的unsigned short int转换为网络字节顺序
	sin.sin_addr.S_un.S_addr = INADDR_ANY;	//IP地址设置成INADDR_ANY，让系统自动获取本机的IP地址
	
	//bind函数把一个地址族中的特定地址赋给scket。
	if (bind(g_SocketServer, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		HAZE_LOG_ERR_W("Haze调试器Socket绑定失败!\n");
		return;
	}

	//开始监听
	if (listen(g_SocketServer, 5) == SOCKET_ERROR)
	{
		HAZE_LOG_ERR_W("Haze调试器Socket监听失败!\n");
		return;
	}

	//循环接收数据

	while (m_VM)
	{
		sockaddr_in remoteAddr;				//sockaddr_in常用于socket定义和赋值,sockaddr用于函数参数
		int nAddrlen = sizeof(remoteAddr);

		HAZE_LOG_INFO(H_TEXT("Haze调试器等待Socket连接...\n"));
		g_SocketClient = accept(g_SocketServer, (sockaddr*)&remoteAddr, &nAddrlen);
		if (g_SocketClient == INVALID_SOCKET)
		{
			HAZE_LOG_INFO(H_TEXT("Haze调试器Socket连接失败!\n"));
			break;
		}
		HAZE_LOG_INFO(H_TEXT("Haze调试器接收到一个连接<%s>!\n"), String2WString(inet_ntoa(remoteAddr.sin_addr)).c_str());

		if (!g_Debugger)
		{
			g_Debugger = MakeUnique<HazeDebugger>(m_VM, &CloseServer);
		}
		break;
	}

	//接收数据
	Recv();
}

static bool HandleMessage(char* Message)
{
	Message = UTF8_2_GB2312(Message);
	std::string Str;
	Str += Message[0];
	x_uint32 m_Type = StringToStandardType<x_uint32>(Str);
	HAZE_LOG_INFO("handle message %d %s\n", m_Type, Message + 1);
	switch ((HazeDebugOperatorType)m_Type)
	{
	case HazeDebugOperatorType::None:
		HAZE_LOG_ERR_W("Haze调试接收到<空调试>操作\n");
		return false;
	case HazeDebugOperatorType::Start:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<开始>操作\n"));
		g_Debugger->Start();
		return true;
	case HazeDebugOperatorType::End:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<退出>操作\n"));
		g_Debugger->End();
		return true;
	case HazeDebugOperatorType::StepOver:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<单步断点>操作\n"));
		g_Debugger->StepOver();
		return true;
	case HazeDebugOperatorType::StepIn:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<单步进入>操作\n"));
		g_Debugger->StepIn();
		return true;
	case HazeDebugOperatorType::StepInstruction:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<单步指令>操作\n"));
		g_Debugger->StepInstruction();
		return true;
	case HazeDebugOperatorType::AddBreakPoint:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<添加断点>操作\n"));
		g_Debugger->AddBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteBreakPoint:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<删除断点>操作\n"));
		g_Debugger->DeleteBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteModuleAllBreakPoint:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<删除模块所有断点>操作\n"));
		g_Debugger->DeleteModuleAllBreakPoint(Message + 1);
		return true;
	case HazeDebugOperatorType::DeleteAllBreakPoint:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<删除所有断点>操作\n"));
		g_Debugger->DeleteAllBreakPoint();
		return true;
	case HazeDebugOperatorType::Continue:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<继续>操作\n"));
		g_Debugger->Continue();
		return true;
	case HazeDebugOperatorType::GetLocalVariable:
		HAZE_LOG_INFO(H_TEXT("Haze调试接收到<请求临时变量数据>操作\n"));
		{
			XJson json;
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
				HAZE_LOG_ERR_W("接收的Socket数据大小超过1024!\n");
			}
		}
		else
		{
			in_addr Addr;
			Addr.S_un.S_addr = (decltype(Addr.S_un.S_addr))g_SocketClient;
			HAZE_LOG_INFO(H_TEXT("关闭调试器Socket<%s>!\n"), String2WString(inet_ntoa(Addr)).c_str());
			break;
		}
		//发送数据
	}

	closesocket(g_SocketClient);
	WSACleanup();
}