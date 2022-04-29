/*  ---------------------------------------------
    prctime
    ©2022 matteo.gattanini@gmail.com

    OVERVIEW
    ---------------------------------------------
    Measures process execution time

    DEPENDENCIES:
    --------------------------------------------- */
#include <iostream>
#include <string>
#include <stdexcept>
using namespace std::literals; // "..."sv

import utilities.system; // sys::*
import utilities.time_meas; // tms::measure


/////////////////////////////////////////////////////////////////////////////
class Arguments final
{
 public:
    Arguments(int argc, const char* argv[])
       {
        // The first argument is the executable
        if(argc < 2) throw std::invalid_argument("Executable not provided");
        i_exe = std::string{ argv[1] };

        // The remaining are the executable arguments
        i_exeargs.reserve( 20 * argc );
        for( int i=2; i<argc; ++i )
           {
            i_exeargs += " ";
            i_exeargs += argv[i];
           }
       }

    static void print_help() noexcept
       {
        std::cout << "\nMeasures the time taken by a process:\n"
                     "   .Uses GetProcessTimes() api\n"
                     "   .Prints result to stdout\n"
                     "\n";
       }

    static void print_usage() noexcept
       {
        std::cerr << "\nUsage:\n"
                     "   prctime prg.exe prg-arg1 prg-arg2\n"
                     "\n";
       }

    const auto& exe() const noexcept { return i_exe; }
    const auto& exeargs() const noexcept { return i_exeargs; }
    //bool verbose() const noexcept { return i_verbose; }

 private:
    std::string i_exe;
    std::string i_exeargs;
    //bool i_verbose = false;
};



//---------------------------------------------------------------------------
int main( int argc, const char* argv[] )
{
    try{
        Arguments args(argc, argv);

        tms::measure t;
        //sys::perf_counter perf_cnt(1.0); // Measures time in [s]

        sys::process prc( args.exe() );
        prc.launch( args.exeargs() );

        t.start();
        //const double t_start_s = perf_cnt.now();
        
        const int ret = prc.wait();

        const double actual_time_seconds = t.elapsed_seconds();
        //const double actual_perf_time_seconds = perf_cnt.now() - t_start_s;
        const auto prc_stats = prc.get_execution_stats();

        //std::cout.imbue(std::locale("")); // #include <locale>
        std::cout << std::fixed; std::cout.precision(6);
        std::cout << "actual: " << actual_time_seconds << " s\n"
                     //"actual perf: " << actual_perf_time_seconds << " s\n"
                     "system: " << (prc_stats.kernel_time_seconds + prc_stats.user_time_seconds) << " s"
                                   " (kernel:" << prc_stats.kernel_time_seconds << " + user:" << prc_stats.user_time_seconds << ")\n"
                     "cpu-cycles: " << prc_stats.CPU_clock_cycles << '\n';

        return ret;
       }

    catch(std::invalid_argument& e)
       {
        std::cerr << "!! " << e.what() << '\n';
        Arguments::print_usage();
       }

    catch(std::exception& e)
       {
        std::cerr << "!! Error: " << e.what() << '\n';
       }

    return 1;
}
