#pragma once

class GCObject
{
public:
	GCObject(x_uint32 gcIndex) : m_GCIndex(gcIndex) {}
	
	x_uint32 m_GCIndex;
};