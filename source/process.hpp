#pragma once
//  ---------------------------------------------
//  System utilities
//  ---------------------------------------------
#include <Windows.h>
#include <realtimeapiset.h> // QueryProcessCycleTime

#include <stdexcept> // std::runtime_error
#include <string>
#include <string_view>
#include <format>


//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
namespace sys //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
{


//---------------------------------------------------------------------------
// Format system error message
[[nodiscard]] std::string get_lasterr_msg(DWORD e =0)
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
    return std::format("[0x{:X}] {}", e, std::string_view(buf, siz));
}


/////////////////////////////////////////////////////////////////////////////
class process final
{
    const std::string m_exepath;
    STARTUPINFO m_startup_info; // This is a [in] parameter
    PROCESS_INFORMATION m_prc_info; // The [out] parameter
    BOOL m_created;

 public:
    process(const std::string& pth) noexcept
      : m_exepath{pth}
      , m_startup_info{}
      , m_prc_info{}
      , m_created{false}
       {}


    ~process() noexcept
       {
        if( m_created )
           {
            close_handles();
           }
       }


    void launch(const std::string& args)
       {
        if( m_created )
           {
            throw std::runtime_error("sys::process::launch: Already running");
           }

        m_startup_info = {0};
        m_prc_info = {0};
        m_startup_info.cb = sizeof(m_startup_info);
        m_startup_info.wShowWindow = SW_SHOWDEFAULT;
        m_startup_info.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEONFEEDBACK;

        std::string cmd {m_exepath};
        if( !args.empty() )
           {
            cmd += "";
            cmd += args;
           }
        m_created = ::CreateProcess(NULL,                  // LPCTSTR lpApplicationName (better use 'lpCommandLine')
                                    (LPTSTR) cmd.c_str(),  // LPTSTR lpCommandLine [MAX_PATH]
                                    NULL,                  // LPSECURITY_ATTRIBUTES lpProcessAttributes
                                    NULL,                  // LPSECURITY_ATTRIBUTES lpThreadAttributes
                                    FALSE,                 // BOOL bInheritHandles
                                    NORMAL_PRIORITY_CLASS, // DWORD dwCreationFlags
                                    NULL,                  // LPVOID lpEnvironment
                                    NULL,                  // LPCTSTR lpCurrentDirectory
                                    &m_startup_info,        // LPSTARTUPINFO lpStartupInfo
                                    &m_prc_info);       // LPPROCESS_INFORMATION lpProcessInformation
        if( !m_created )
           {
            throw std::runtime_error( std::format("sys::process::launch: {}", sys::get_lasterr_msg()) );
           }
       }


    [[nodiscard]] int wait(int tout =2'000'000'000) // [ms]
       {
        DWORD ret = 1; // Default return code
        if( m_created )
           {
            const int dt = 100; // [ms]
            do {
                switch( ::WaitForSingleObject(m_prc_info.hProcess, dt) )
                   {
                    case WAIT_TIMEOUT : // Still executing...
                        // Could terminate when timeout expires
                        if( (tout-=dt) <= 0 )
                           {
                            ::TerminateProcess(m_prc_info.hProcess, ret);
                            return ret;
                           }
                        break;

                    case WAIT_OBJECT_0 : // Process ended
                    case WAIT_FAILED :
                    case WAIT_ABANDONED :
                        ::GetExitCodeProcess(m_prc_info.hProcess, &ret);
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


    //DWORD terminate() noexcept
    //   {
    //    DWORD ret = 0;
    //    ::TerminateProcess(m_prc_info.hProcess, ret);
    //    return ret;
    //   }


    void close_handles() noexcept
       {
        ::CloseHandle(m_prc_info.hThread);
        ::CloseHandle(m_prc_info.hProcess);
        m_created = false;
       }


    [[nodiscard]] bool is_running() const noexcept
       {
        return m_created && (WAIT_TIMEOUT==::WaitForSingleObject(m_prc_info.hProcess, 1));
       }


    struct exec_stats_t
       {
        double kernel_time_seconds;
        double user_time_seconds;
        ULONG64 CPU_clock_cycles;
       };
    [[nodiscard]] exec_stats_t get_execution_stats() const
       {
        if( !m_created )
           {
            throw std::runtime_error("sys::process::get_execution_stats: Process not created!");
           }
        exec_stats_t stats{};

        // FILETIME: A 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC)
        struct local
           {
            static double FILETIME_to_seconds(const FILETIME& ft)
               {
                const ULARGE_INTEGER ul = {ft.dwLowDateTime, ft.dwHighDateTime};
                return double(ul.QuadPart) * 100E-9; // [s]
               }
           };

        FILETIME ftCreation, ftExit, ftKernel, ftUser;
        if( !::GetProcessTimes(m_prc_info.hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser) )
           {
            throw std::runtime_error( std::format("GetProcessTimes: {}", sys::get_lasterr_msg()) );
           }

        stats.kernel_time_seconds = local::FILETIME_to_seconds(ftKernel);
        stats.user_time_seconds = local::FILETIME_to_seconds(ftUser);
        ::QueryProcessCycleTime( m_prc_info.hProcess, &(stats.CPU_clock_cycles) );

        return stats;
       }
};

}//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
