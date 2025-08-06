#include "Timer.h"
#include <print>

void ConsoleLog::PrintElapsedTime(TimeStamp& stamp)
{
	std::println("Elapsed time: [{} mic] | [{} ms] | [{} s]", stamp.microseconds, stamp.milliseconds, stamp.seconds);
}

void ConsoleLog::PrintElapsedTime(std::string head, TimeStamp& stamp)
{
	std::println("{} : [{} mic] | [{} ms] | [{} s]", head, stamp.microseconds, stamp.milliseconds, stamp.seconds);
}