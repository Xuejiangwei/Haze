#pragma once

x_uint64 RoundUp(x_int64 byteSize);

x_uint64 RoundUpToPowerOf2(x_uint64 size);

int GetBuddyOrder(x_uint64 size);

void* GetBuddyAddress(void* address, x_uint64 size);