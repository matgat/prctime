#ifndef GUARD_system_hpp
#define GUARD_system_hpp
/*  ---------------------------------------------
    ©2022 matteo.gattanini@gmail.com

    OVERVIEW
    ---------------------------------------------
    Some system utilities

    DEPENDENCIES:
    --------------------------------------------- */
    #include <Windows.h>
    #include <realtimeapiset.h>

    #include <stdexcept>
    #include <string>
    #include <format>
    //#include <ctime> // std::time_t, std::strftime
    //#include <chrono> // std::chrono::system_clock
    //using namespace std::chrono_literals; // 1s, 2h, ...
    #include <filesystem> // std::filesystem
    namespace fs = std::filesystem;


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace sys //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
{


//---------------------------------------------------------------------------
// Format system error message
std::string get_lasterr_msg(DWORD e =0) noexcept
{
    if(e==0) e = ::GetLastError(); // ::WSAGetLastError()
    const DWORD buf_siz = 1024;
    TCHAR buf[buf_siz];
    const DWORD siz =
        ::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS|
                         FORMAT_MESSAGE_MAX_WIDTH_MASK,
                         nullptr,
                         e,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         buf,
                         buf_siz,
                         nullptr );
    return std::format("[{}] {}", e, std::string(buf, siz));
}


/////////////////////////////////////////////////////////////////////////////
class process
{
 public:

    process(const std::string& pth)
      : i_exepath(pth)
      , i_StartupInfo({0})
      , i_ProcessInfo({0})
      , i_Created(false) {}


    ~process()
       {
        if(i_Created)
           {
            //if(must_always_wait) wait();
            close_handles();
           }
       }


    //-------------------------------------------------------------------
    void run(const std::string& args)
       {
        if(i_Created)
           {
            throw std::runtime_error("sys::process::run: already running");
           }

        i_StartupInfo = {0};
        i_ProcessInfo = {0};
        i_StartupInfo.cb = sizeof(i_StartupInfo);
        i_StartupInfo.wShowWindow = SW_SHOWDEFAULT;
        i_StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;

        std::string cmd {i_exepath};
        if( !args.empty() )
           {
            cmd += "";
            cmd += args;
           }
        i_Created = ::CreateProcess(NULL,                  // LPCTSTR lpApplicationName (better use 'lpCommandLine')
                                    (LPTSTR) cmd.c_str(),           // LPTSTR lpCommandLine [MAX_PATH]
                                    NULL,                  // LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                    NULL,                  // LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                    FALSE,                 // BOOL bInheritHandles,
                                    NORMAL_PRIORITY_CLASS, // DWORD dwCreationFlags,
                                    NULL,                  // LPVOID lpEnvironment,
                                    NULL,                  // LPCTSTR lpCurrentDirectory,
                                    &i_StartupInfo,        // LPSTARTUPINFO lpStartupInfo,
                                    &i_ProcessInfo);       // LPPROCESS_INFORMATION lpProcessInformation
        if( !i_Created )
           {
            throw std::runtime_error( std::format("sys::process::run: %s", sys::get_lasterr_msg()) );
           }
       }


    //-------------------------------------------------------------------
    DWORD terminate() noexcept
       {
        DWORD ret = 0;
        ::TerminateProcess(i_ProcessInfo.hProcess, ret);
        return ret;
       }


    //-------------------------------------------------------------------
    int wait(int tout =2000000000) // [ms]
       {
        DWORD ret = 1; // Default return code
        if( i_Created )
           {
            const int dt = 100; // [ms]
            do {
                switch( ::WaitForSingleObject(i_ProcessInfo.hProcess, dt) )
                   {
                    case WAIT_TIMEOUT : // Still executing...
                        // Could terminate when timeout expires
                        if( (tout-=dt) <= 0 )
                           {
                            ::TerminateProcess(i_ProcessInfo.hProcess, ret);
                            return ret;
                           }
                        //else Application->ProcessMessages();
                        break;

                    case WAIT_OBJECT_0 : // Process ended
                    case WAIT_FAILED :
                    case WAIT_ABANDONED :
                        ::GetExitCodeProcess(i_ProcessInfo.hProcess, &ret);
                        return ret;

                    default :
                        throw std::runtime_error("sys::process::wait: spawn error");
                   }
               }
            while( true );
           }
        // If here already closed (or never started)
        return ret;
       }


    //-------------------------------------------------------------------
    void close_handles()
       {
        ::CloseHandle(i_ProcessInfo.hThread);
        ::CloseHandle(i_ProcessInfo.hProcess);
        i_Created = false;
       }


    //-------------------------------------------------------------------
    bool is_running() const
       {
        return i_Created && (WAIT_TIMEOUT==::WaitForSingleObject(i_ProcessInfo.hProcess, 1));
       }


    //-------------------------------------------------------------------
    const std::string& path() const { return i_exepath; }


    //-------------------------------------------------------------------
    std::string get_time() const
       {
        if(!i_Created) throw std::runtime_error("get_time: process not created!");


        // FILETIME: A 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC)
        struct
           {
            static std::string FILETIME_to_string(const FILETIME& ft)
               {
                SYSTEMTIME stUTC;
                ::FileTimeToSystemTime(&ft, &stUTC);
                SYSTEMTIME stLoc;
                ::SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLoc);
                return std::format("{:02d}:{:02d}:{:02d}.{:03d}", stLoc.wHour, stLoc.wMinute, stLoc.wSecond, stLoc.wMilliseconds);
               }

            static double FILETIME_to_seconds(const FILETIME& ft)
               {
                const ULARGE_INTEGER ul = {ft.dwLowDateTime, ft.dwHighDateTime};
                //ul.LowPart = ft.dwLowDateTime;
                //ul.HighPart = ft.dwHighDateTime;
                return double(ul.QuadPart) * 100E-9; // [s]
               }
           } convert;

        FILETIME ftCreation, ftExit, ftKernel, ftUser;
        if( !::GetProcessTimes(i_ProcessInfo.hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser) )
            throw std::runtime_error( std::format("GetProcessTimes: %s", sys::get_lasterr_msg()) );

        ULONG64 CPU_clock_cycles = 0;
        ::QueryProcessCycleTime( i_ProcessInfo.hProcess, &CPU_clock_cycles );

        const double kernel_time_s = convert.FILETIME_to_seconds(ftKernel);
        const double user_time_s = convert.FILETIME_to_seconds(ftUser);
        return std::format("{} s (={}+{}) cpu clock cycles: %ld", kernel_time_s+user_time_s, kernel_time_s, user_time_s, CPU_clock_cycles);

        //const std::string s_create_time = convert.FILETIME_to_string(ftCreation);
        //const std::string s_exit_time = convert.FILETIME_to_string(ftExit);
        //return std::format("Created: {}  Duration: {} s (={}+{})", s_create_time, kernel_time_s+user_time_s, kernel_time_s, user_time_s);
       }


 private:
    const std::string i_exepath;
    STARTUPINFO i_StartupInfo; // This is a [in] parameter
    PROCESS_INFORMATION i_ProcessInfo; // The [out] parameter
    BOOL i_Created;
};


}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



//---- end unit -------------------------------------------------------------
#endif
