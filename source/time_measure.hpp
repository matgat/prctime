#pragma once
//  ---------------------------------------------
//  A facility to measure elapsed time
//  ---------------------------------------------
#include <chrono> // std::chrono::*
//using namespace std::chrono_literals; // 1s, 2h, ...


namespace sys //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
{

/////////////////////////////////////////////////////////////////////////////
class time_measure final
{
    std::chrono::steady_clock::time_point t0;

 public:
    time_measure() noexcept
      : t0{ std::chrono::steady_clock::now() }
       {}

    void start() noexcept
       {
        t0 = std::chrono::steady_clock::now();
       }

    [[nodiscard]] double elapsed_seconds() const noexcept
       {
        const auto dt_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0);
        return std::chrono::duration<double>(dt_ns).count();
       }
};

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
