#include <string>
#include <stdexcept>
#include <print>

#include "process.hpp" // sys::process
#include "time_measure.hpp" // sys::time_measure


/////////////////////////////////////////////////////////////////////////////
class Arguments final
{
 private:
    std::string m_exe;
    std::string m_exeargs;

 public:
    [[nodiscard]] const auto& exe() const noexcept { return m_exe; }
    [[nodiscard]] const auto& exeargs() const noexcept { return m_exeargs; }

    Arguments(const int argc, const char* const argv[])
       {
        // The first argument is the executable
        if(argc < 2) throw std::invalid_argument("Executable not provided");
        m_exe = std::string{ argv[1] };

        // The remaining are the executable arguments
        m_exeargs.reserve( 20u * argc );
        for( int i=2; i<argc; ++i )
           {
            m_exeargs += ' ';
            m_exeargs += argv[i];
           }
       }

    static void print_usage() noexcept
       {
        std::print( "\nUsage:\n"
                     "   prctime prg.exe [prg-arg1 prg-arg2 ...]\n"
                     "\n" );
       }
};



//---------------------------------------------------------------------------
int main( const int argc, const char* const argv[] )
{
    try{
        const Arguments args(argc, argv);

        sys::time_measure t;

        sys::process prc( args.exe() );
        prc.launch( args.exeargs() );

        t.start();

        const int ret = prc.wait();

        const double actual_time_seconds = t.elapsed_seconds();
        const auto prc_stats = prc.get_execution_stats();

        std::print( "actual: {:.6f} s\n"
                    "system: {:.6f} s (kernel:{:.6f} + user:{:.6f})\n"
                    "cpu-cycles: {}\n",
                    actual_time_seconds,
                    prc_stats.kernel_time_seconds + prc_stats.user_time_seconds, prc_stats.kernel_time_seconds, prc_stats.user_time_seconds,
                    prc_stats.CPU_clock_cycles );
        return ret;
       }

    catch(std::invalid_argument& e)
       {
        std::print("!! {}\n", e.what());
        Arguments::print_usage();
       }

    catch(std::exception& e)
       {
        std::print("!! Error: {}\n", e.what());
       }

    return 2;
}
