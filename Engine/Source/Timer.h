#pragma once
#include <chrono>

using namespace std::chrono;

class Timer
{
private:
    time_point<steady_clock> m_start;
    time_point<steady_clock> m_end;
    bool m_stopped = false;

public:
    Timer() = default;

    void Start()
    {
        m_start = steady_clock::now();
        m_stopped = false;
    }

    long long Stop()
    {
        if (!m_stopped)
        {
            m_end = steady_clock::now();
            m_stopped = true;
        }
        return Read();
    }

    long long Read() const
    {
        steady_clock::time_point end = m_stopped ? m_end : steady_clock::now();
        return duration<double, std::milli>(end - m_start).count();
    }
};
