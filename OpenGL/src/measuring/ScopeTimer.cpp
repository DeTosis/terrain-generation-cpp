#include "Timer.h"

ScopeTimer::ScopeTimer(TimeStamp* timeStamp)
	: m_TimeStamp(timeStamp)
{
	m_StartTimePoint = std::chrono::high_resolution_clock::now();
}

ScopeTimer::~ScopeTimer()
{
	Stop();
}

void ScopeTimer::Stop()
{
	auto endTimePoint = std::chrono::high_resolution_clock::now();

	auto startTimeMs = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();
	auto endTimeMs = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

	auto elapsedTime = endTimeMs - startTimeMs;

	m_TimeStamp->microseconds = elapsedTime;
	m_TimeStamp->milliseconds = elapsedTime * 0.001;
	m_TimeStamp->seconds = elapsedTime * 0.001 * 0.001;
}
