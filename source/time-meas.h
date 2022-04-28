#include <chrono>

std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;

0 - Delta
Use a delta function to compute time differences:

auto start = std::chrono::steady_clock::now();
std::cout << "Elapsed(ms)=" << since(start).count() << std::endl;
since accepts any timepoint and produces any duration (milliseconds is the default). It is defined as:

template <
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}
Demo

1 - Timer
Use a timer based on std::chrono:

Timer clock; // Timer<milliseconds, steady_clock>

clock.tick();
/* code you want to measure */
clock.tock();

cout << "Run time = " << clock.duration().count() << " ms\n";
Demo

Timer is defined as:

template <class DT = std::chrono::milliseconds,
          class ClockT = std::chrono::steady_clock>
class Timer
{
    using timep_t = typename ClockT::time_point;
    timep_t _start = ClockT::now(), _end = {};

public:
    void tick() { 
        _end = timep_t{}; 
        _start = ClockT::now(); 
    }
    
    void tock() { _end = ClockT::now(); }
    
    template <class T = DT> 
    auto duration() const { 
        gsl_Expects(_end != timep_t{} && "toc before reporting"); 
        return std::chrono::duration_cast<T>(_end - _start); 
    }
};
As Howard Hinnant pointed out, we use a duration to remain in the chrono type-system and perform operations like averaging or comparisons (e.g. here this means using std::chrono::milliseconds). When we just do IO, we use the count() or ticks of a duration (e.g. here number of milliseconds).

2 - Instrumentation
Any callable (function, function object, lambda etc.) can be instrumented for benchmarking. Say you have a function F invokable with arguments arg1,arg2, this technique results in:

cout << "F runtime=" << measure<>::duration(F, arg1, arg2).count() << "ms";
Demo

measure is defined as:

template <class TimeT  = std::chrono::milliseconds
          class ClockT = std::chrono::steady_clock>
struct measure
{
    template<class F, class ...Args>
    static auto duration(F&& func, Args&&... args)
    {
        auto start = ClockT::now();
        std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        return std::chrono::duration_cast<TimeT>(ClockT::now()-start);
    }
};
As mentioned in (1), using the duration w/o .count() is most useful for clients that want to post-process a bunch of durations prior to I/O, e.g. average:

auto avg = (measure<>::duration(func) + measure<>::duration(func)) / 2;
std::cout << "Average run time " << avg.count() << " ms\n";
