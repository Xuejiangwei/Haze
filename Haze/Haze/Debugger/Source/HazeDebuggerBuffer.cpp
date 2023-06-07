#include "HazeDebuggerBuffer.h"

#ifdef _WIN32
	#include <winsock.h>
	#pragma comment(lib,"ws2_32.lib")
#endif

HazeDebuggerBuffer::HazeDebuggerBuffer() : CurrIndex(0)
{
	HeadNode = new DebuggerBufferNode();
	TailNode = HeadNode;
}

HazeDebuggerBuffer::~HazeDebuggerBuffer()
{
	auto Node = HeadNode;
	while (Node)
	{
		auto NextNode = Node->NextNode;
		delete Node;

		Node = NextNode;
	}

	for (auto& Iter : Vector_Node)
	{
		delete Iter;
	}

	Vector_Node.clear();
}

void HazeDebuggerBuffer::ReadBuffer(uint64 SocketClient)
{
	int Ret = recv(SocketClient, &TailNode->Data[CurrIndex], DEBUGGER_BUFFER_NODE_SIZE, MSG_PEEK);
	if (Ret > 0)
	{
		if (Ret < DEBUGGER_BUFFER_NODE_SIZE - CurrIndex)
		{
			
		}
	}
}
