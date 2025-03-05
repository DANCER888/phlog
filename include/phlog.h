#ifndef _PHLOG_H
#define _PHLOG_H

#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <ctime>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

// Enable this option for more vibrant colors if your virtual console supports 256-color mode
//#define _PHLOG_USE_256_COLORS

namespace phlog
{
    /*
        Warning levels
    */

    constexpr int LOGGER_NONE = 0;
    constexpr int LOGGER_SUCCESS = 1;
    constexpr int LOGGER_WARNING = 2;
    constexpr int LOGGER_ERROR = 3;
    constexpr int LOGGER_CRITICAL = 4;

    /*
        Color codes -  https://talyian.github.io/ansicolors/
    */
#ifdef _PHLOG_USE_256_COLORS
    //   256-color Mode
    // \x1b[38;2;r;g;bm - foreground
    // \x1b[48;2;r;g;bm - background

    constexpr const char* COLOR_DEFAULT =   "\x1b[38;2;255;255;255m";
    constexpr const char* COLOR_NONE =      "\x1b[38;2;211;211;211m";
    constexpr const char* COLOR_SUCCESS =   "\x1b[38;2;40;130;30m";
    constexpr const char* COLOR_WARNING =   "\x1b[38;2;130;130;30m";
    constexpr const char* COLOR_ERROR =     "\x1b[38;2;130;30;30m";
    constexpr const char* COLOR_CRITICAL =  "\x1b[38;2;70;4;4m";
    constexpr const char* COLOR_DARK =      "\x1b[38;2;89;89;89m";
#else
    constexpr const char* COLOR_DEFAULT   = "\x1b[38;5;15m";
    constexpr const char* COLOR_NONE      = "\x1b[38;5;7m";
    constexpr const char* COLOR_SUCCESS   = "\x1b[38;5;2m";
    constexpr const char* COLOR_WARNING   = "\x1b[38;5;11m";
    constexpr const char* COLOR_ERROR     = "\x1b[38;5;1m";
    constexpr const char* COLOR_CRITICAL  = "\x1b[38;5;9m";
    constexpr const char* COLOR_DARK      = "\x1b[38;5;8m";
#endif
    /*
        Forward function declarations
    */

    inline std::string get_log_filename();
    inline void log(const char* file, int line, int warning_level, const char* format, ...);
    inline std::string get_short_filename(const char* filepath);

    std::ofstream log_file;

    inline bool setup() {
        std::string filename = get_log_filename();
        log_file.open(filename);
        if (!log_file.is_open()) {
            log(__FILE__,__LINE__,LOGGER_CRITICAL, "Failed to initialize phlog");
            return 1;
        }
        return 0;
    }

    // Function to get short filename (without path)
    inline std::string get_short_filename(const char* filepath) {
        std::string fullpath(filepath);
        size_t found = fullpath.find_last_of("/\\");
        if (found != std::string::npos) {
            return fullpath.substr(found + 1);
        }
        return fullpath;
    }

    inline std::string get_log_filename() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::tm local_tm;
#ifdef _WIN32
        // Use localtime_s on Windows
        localtime_s(&local_tm, &time);
#else
        // Use localtime_r on POSIX systems
        localtime_r(&time, &local_tm);
#endif

        char buffer[80];
        std::strftime(buffer, 80, "log_%Y%m%d_%H%M%S.txt", &local_tm);

        return std::string(buffer);
    }
    
    /*
        Main logging function
    */

    inline void log_impl(const char* file, int line, int logger_level, const char* format, va_list args)
    {
#ifdef _WIN32
        // Enable ANSI escape codes in Windows Command Prompt
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (hConsole == INVALID_HANDLE_VALUE || !GetConsoleMode(hConsole, &mode)) {
            return;
        }
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

        // Print the warning level and set the color
        switch (logger_level)
        {
        case LOGGER_NONE:
            printf("%s[PHLOG]    %s", COLOR_DEFAULT,COLOR_NONE);
            break;
        case LOGGER_SUCCESS:
            printf("%s[SUCCESS] %s ", COLOR_SUCCESS, COLOR_NONE);
            break;
        case LOGGER_WARNING:
            printf("%s[WARNING] %s ", COLOR_WARNING, COLOR_NONE);
        break;
        case LOGGER_ERROR:
            printf("%s[ERROR] %s   ", COLOR_ERROR, COLOR_NONE);
            break;
        case LOGGER_CRITICAL:
            printf("%s[CRITICAL] %s", COLOR_CRITICAL, COLOR_NONE);
            break;
        default:
            printf("%s[UNKNOWN]    ", COLOR_NONE);
            break;
        }

        printf("%s (%s %d)%s ",COLOR_DARK, get_short_filename(file).c_str(), line,COLOR_NONE);
        // Write to file
        if (phlog::log_file.is_open()) {
            switch (logger_level)
            {
            case LOGGER_NONE:
                phlog::log_file << "[PHLOG]     ";
                break;
            case LOGGER_SUCCESS:
                phlog::log_file << "[SUCCESS]  ";
                break;
            case LOGGER_WARNING:
                phlog::log_file << "[WARNING]  ";
            break;
            case LOGGER_ERROR:
                phlog::log_file << "[ERROR]    ";
                break;
            case LOGGER_CRITICAL:
                phlog::log_file << "[CRITICAL] ";
                break;
            default:
                phlog::log_file << "[" << logger_level << "] ";
                break;
            }

            char buffer[1024];
            vsnprintf(buffer, sizeof(buffer), format, args);

            // Write to file
            log_file << " (" << get_short_filename(file).c_str() << " " << line << ") " << buffer  << std::endl;
        }

        vprintf(format, args);

        printf("%s\n", COLOR_DEFAULT);
    }
    // Main variadic log function
    inline void log(const char* file, int line,int logger_level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        log_impl(file,line,logger_level, format, args);
        va_end(args);
    }

}

#define PHLOG(...) phlog::log(__FILE__,__LINE__,phlog::LOGGER_NONE, __VA_ARGS__)
#define PHLOG_SUCCESS(...) phlog::log(__FILE__,__LINE__,phlog::LOGGER_SUCCESS, __VA_ARGS__)
#define PHLOG_WARNING(...) phlog::log(__FILE__,__LINE__,phlog::LOGGER_WARNING, __VA_ARGS__)
#define PHLOG_ERROR(...) phlog::log(__FILE__,__LINE__,phlog::LOGGER_ERROR, __VA_ARGS__)
#define PHLOG_CRITICAL(...) phlog::log(__FILE__,__LINE__,phlog::LOGGER_CRITICAL, __VA_ARGS__)

#endif //_PHLOG_H