#include "Timer.h"

RepeatableTimer::RepeatableTimer(TimeStamp* timeStamp)
	: m_TimeStamp(timeStamp)
{
	m_StartTimePoint = std::chrono::high_resolution_clock::now();
}

RepeatableTimer::~RepeatableTimer()
{
	Stop();
}

void RepeatableTimer::Stop()
{
	auto endTimePoint = std::chrono::high_resolution_clock::now();

	auto startTimeMs = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
	auto endTimeMs = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

	auto elapsedTime = endTimeMs - startTimeMs;

	m_TimeStamp->microseconds = elapsedTime;
	m_TimeStamp->milliseconds = elapsedTime * 0.001;
	m_TimeStamp->seconds = elapsedTime * 0.001 * 0.001;
}

void RepeatableTimer::Restart()
{
	m_StartTimePoint = std::chrono::high_resolution_clock::now();
}

void RepeatableTimer::Print(std::string head)
{
	Stop();
	ConsoleLog log;
	if (head == "")
		head = "Elapsed time: ";

	log.PrintElapsedTime(head, *m_TimeStamp);
}

void RepeatableTimer::PrintAndRestart(std::string head)
{
	Stop();
	Print(head);

	m_TimeStamp->microseconds = -1;
	m_TimeStamp->milliseconds = -1;
	m_TimeStamp->seconds = -1;
	Restart();
}

void RepeatableTimer::PrintAndBreak(std::string head)
{
	Stop();
	Print(head);
	__debugbreak();
}
