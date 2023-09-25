#pragma once

#include "Haze.h"

#define DEBUGGER_BUFFER_NODE_SIZE 1024

struct DebuggerBufferNode
{
	char m_Data[DEBUGGER_BUFFER_NODE_SIZE];
	DebuggerBufferNode* NextNode;
};

class HazeDebuggerBuffer
{
public:
	HazeDebuggerBuffer();
	~HazeDebuggerBuffer();

	void ReadBuffer(uint64 SocketClient);

private:
	uint32 CurrIndex;

	DebuggerBufferNode* HeadNode;
	DebuggerBufferNode* TailNode;

	std::vector<DebuggerBufferNode*> Vector_Node;
};
