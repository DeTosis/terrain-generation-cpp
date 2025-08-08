#pragma once
#include "windows.h"
#include "psapi.h"

class Memory
{
public:
	static DWORDLONG GetCurrentProcessUsage();
};