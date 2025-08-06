#pragma once
#include <chrono>

struct TimeStamp
{
	long long microseconds = -1;
	long long milliseconds = -1;
	long long seconds = -1;
};

// *** DISPLAY ***
class ConsoleLog
{
public:
	void PrintElapsedTime(TimeStamp& stamp);
	void PrintElapsedTime(std::string head, TimeStamp& stamp);
};
// *** DISPLAY ***

// *** TIMERS ***
class ScopeTimer
{
public:
	ScopeTimer(TimeStamp* timeStamp);
	~ScopeTimer();
	void Stop();
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
	TimeStamp* m_TimeStamp;
};

class RepeatableTimer
{
public:
	RepeatableTimer(TimeStamp* timeStamp);
	~RepeatableTimer();
	void Stop();
	void Restart();
	void Print(std::string head = "");
	void PrintAndRestart(std::string head = "");
	void PrintAndBreak(std::string head = "");
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
	TimeStamp* m_TimeStamp;
};
// *** TIMERS ***