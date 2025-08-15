#include "HazePch.h"
#include "HazeString.h"

HazeString::HazeString(const x_HChar* data) : m_Hash(0)
{
	m_Length = std::char_traits<x_HChar>::length(data);
	m_Data = (x_HChar*)malloc((m_Length + 1) * sizeof(x_HChar));
	memcpy(m_Data, data, m_Length * sizeof(x_HChar));
	m_Data[m_Length] = 0;
}

HazeString::~HazeString()
{
	free(m_Data);
}