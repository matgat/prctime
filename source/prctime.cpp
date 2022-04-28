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

#include "system.hpp" // sys::*


using namespace std::literals; // "..."sv



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

        sys::process prc( args.exe() );
        prc.run( args.exeargs() );
        const int ret = prc.wait();

        std::cout << prc.get_time() << '\n';

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
