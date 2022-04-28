module;
/*  ---------------------------------------------
    ©2022 matteo.gattanini@gmail.com

    OVERVIEW
    ---------------------------------------------
    Some system utilities

    DEPENDENCIES:
    --------------------------------------------- */
    #include <Windows.h>
    #include <realtimeapiset.h> // QueryProcessCycleTime

    #include <stdexcept>
    #include <string>
    #include <format>


export module utilities.system;

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
export class process final
{
 private:
    const std::string i_exepath;
    STARTUPINFO i_StartupInfo; // This is a [in] parameter
    PROCESS_INFORMATION i_ProcessInfo; // The [out] parameter
    BOOL i_Created;

 public:
    process(const std::string& pth) noexcept
      : i_exepath{pth}
      , i_StartupInfo{}
      , i_ProcessInfo{}
      , i_Created{false} {}


    ~process() noexcept
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
            throw std::runtime_error("sys::process::run: Already running");
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
                                    (LPTSTR) cmd.c_str(),  // LPTSTR lpCommandLine [MAX_PATH]
                                    NULL,                  // LPSECURITY_ATTRIBUTES lpProcessAttributes
                                    NULL,                  // LPSECURITY_ATTRIBUTES lpThreadAttributes
                                    FALSE,                 // BOOL bInheritHandles
                                    NORMAL_PRIORITY_CLASS, // DWORD dwCreationFlags
                                    NULL,                  // LPVOID lpEnvironment
                                    NULL,                  // LPCTSTR lpCurrentDirectory
                                    &i_StartupInfo,        // LPSTARTUPINFO lpStartupInfo
                                    &i_ProcessInfo);       // LPPROCESS_INFORMATION lpProcessInformation
        if( !i_Created )
           {
            throw std::runtime_error( std::format("sys::process::run: {}", sys::get_lasterr_msg()) );
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
                        throw std::runtime_error("sys::process::wait: Spawn error");
                   }
               }
            while( true );
           }
        // If here already closed (or never started)
        return ret;
       }


    //-------------------------------------------------------------------
    void close_handles() noexcept
       {
        ::CloseHandle(i_ProcessInfo.hThread);
        ::CloseHandle(i_ProcessInfo.hProcess);
        i_Created = false;
       }


    //-------------------------------------------------------------------
    bool is_running() const noexcept
       {
        return i_Created && (WAIT_TIMEOUT==::WaitForSingleObject(i_ProcessInfo.hProcess, 1));
       }


    //-------------------------------------------------------------------
    struct exec_stats_t
       {
        double kernel_time_seconds;
        double user_time_seconds;
        ULONG64 CPU_clock_cycles;
        //std::string creation_time;
        //std::string exit_time;
       };
    exec_stats_t get_execution_stats() const
       {
        if(!i_Created) throw std::runtime_error("get_time: process not created!");
        exec_stats_t stats{};

        // FILETIME: A 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC)
        struct
           {
            static double FILETIME_to_seconds(const FILETIME& ft)
               {
                const ULARGE_INTEGER ul = {ft.dwLowDateTime, ft.dwHighDateTime};
                //                ul.LowPart = ft.dwLowDateTime; ul.HighPart = ft.dwHighDateTime;
                return double(ul.QuadPart) * 100E-9; // [s]
               }

            //static std::string FILETIME_to_string(const FILETIME& ft)
            //   {
            //    SYSTEMTIME stUTC;
            //    ::FileTimeToSystemTime(&ft, &stUTC);
            //    SYSTEMTIME stLoc;
            //    ::SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLoc);
            //    return std::format("{:02d}:{:02d}:{:02d}.{:03d}", stLoc.wHour, stLoc.wMinute, stLoc.wSecond, stLoc.wMilliseconds);
            //   }

           } convert;

        FILETIME ftCreation, ftExit, ftKernel, ftUser;
        if( !::GetProcessTimes(i_ProcessInfo.hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser) )
            throw std::runtime_error( std::format("GetProcessTimes: {}", sys::get_lasterr_msg()) );

        stats.kernel_time_seconds = convert.FILETIME_to_seconds(ftKernel);
        stats.user_time_seconds = convert.FILETIME_to_seconds(ftUser);
        ::QueryProcessCycleTime( i_ProcessInfo.hProcess, &(stats.CPU_clock_cycles) );
        //stats.creation_time = convert.FILETIME_to_string(ftCreation);
        //stats.exit_time = convert.FILETIME_to_string(ftExit);

        return stats;
       }
};


/////////////////////////////////////////////////////////////////////////////
// Wraps the performance counter
// sys::perf_counter cnt(1.0); // Measures time in [s]
// sys::perf_counter cnt(1E3); // Measures time in [ms]
export class perf_counter final
{
 public:
    perf_counter(const double ks) noexcept : i_dt( ks * get_s_per_tick() ) {}

    [[nodiscard]] static LONGLONG count() noexcept // [ticks] Counter current value
       {
        LARGE_INTEGER x;
        ::QueryPerformanceCounter(&x);
        return x.QuadPart;
       }

    [[nodiscard]] double now() const noexcept { return count()*i_dt; } // [unt] Time elapsed from system start

    //[[nodiscard]] double dt() const noexcept {return i_dt;} // [unt/count] resolution
    //[[nodiscard]] bool available() const noexcept {return i_dt>0.0;} // Availability

 private:
    const double i_dt; // Resolution [unt/count]

    [[nodiscard]] static double get_s_per_tick() noexcept // Get resolution [s/count]
       {
        LARGE_INTEGER x;
        x.QuadPart = 0;
        ::QueryPerformanceFrequency(&x);
        return x.QuadPart!=0 ? 1.0/x.QuadPart : 0.0;
       } 
};


}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
