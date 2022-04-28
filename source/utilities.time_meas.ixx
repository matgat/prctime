module;
/*  ---------------------------------------------
    ©2022 matteo.gattanini@gmail.com

    OVERVIEW
    ---------------------------------------------
    A facility to measure elapsed time

    DEPENDENCIES:
    --------------------------------------------- */
    #include <stdexcept>
    #include <string>
    #include <format>
    //#include <ctime> // std::time_t, std::strftime
    #include <chrono> // std::chrono::*
    //using namespace std::chrono_literals; // 1s, 2h, ...

export module utilities.time_meas;


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace tms //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
{

//---------------------------------------------------------------------------
//  auto start = std::chrono::steady_clock::now();
//  std::cout << "Elapsed(ms)=" << since(start).count() << std::endl;
//template < class result_t   = std::chrono::milliseconds,
//           class clock_t    = std::chrono::steady_clock,
//           class duration_t = std::chrono::milliseconds >
//export auto since(std::chrono::time_point<clock_t, duration_t> const& start)
//{
//    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
//}



/////////////////////////////////////////////////////////////////////////////
export class measure final
{
 private:
    std::chrono::steady_clock::time_point t0;

 public:
    measure() noexcept
      : t0( std::chrono::steady_clock::now() ) {}

    //-------------------------------------------------------------------
    void start() noexcept
       {
        t0 = std::chrono::steady_clock::now();
       }

    //-------------------------------------------------------------------
    double elapsed_seconds() noexcept
       {
        const auto dt_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - t0);
        return std::chrono::duration<double>(dt_ns).count();
       }
};



/////////////////////////////////////////////////////////////////////////////
//  std::cout << "F runtime=" << measure<>::duration(func, arg1, arg2).count() << "ms\n";
//  auto avg = (measure<>::duration(func, arg1, arg2) + measure<>::duration(func, arg1, arg2)) / 2;
//  std::cout << "Average run time " << avg.count() << " ms\n";
//template <class TimeT  = std::chrono::milliseconds
//          class ClockT = std::chrono::steady_clock>
//export struct measure final
//{
//    template<class F, class ...Args>
//    static auto duration(F&& func, Args&&... args)
//    {
//        auto start = ClockT::now();
//        std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
//        return std::chrono::duration_cast<TimeT>(ClockT::now()-start);
//    }
//};



}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
