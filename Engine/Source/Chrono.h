#pragma once
#include <windows.h>
#include <chrono>

//Deprecated
class Chrono
{
public:
    using rep = long long;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<Chrono>;
    static const bool is_steady = true;

    static time_point now() noexcept
    {
        static long long s_frequency = []{
            LARGE_INTEGER f;
            QueryPerformanceFrequency(&f);
            return (long long)f.QuadPart;
            }();

        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);

        return time_point(duration(
            counter.QuadPart * (1000000000ll / s_frequency)
        ));
    }
};
