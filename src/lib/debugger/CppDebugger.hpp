/* CPP_DEBUGGER.hpp
 *   by Lut99
 *
 * Created:
 *   19/12/2020, 16:32:58
 * Last edited:
 *   12/24/2020, 1:01:28 PM
 * Auto updated?
 *   Yes
 *
 * Description:
 *   This file contains a more advanced method of debugging, where we can
 *   specify the debugging type and where its timestamp is noted.
 *   Aditionally, lines are automatically linewrapped (with correct
 *   indents), and extra indentation levels can be given based on functions
 *   entered or left.
**/

#ifndef CPPDEBUGGER_HPP
#define CPPDEBUGGER_HPP

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>

/***** COLOUR CONSTANTS *****/
/* Foreground colours */
#define RED "\033[31;1m"
#define YELLOW "\033[33m"
#define GREEN "\033[32;1m"
#define CYAN "\033[36;1m"
/* Text styles */
#define BOLD "\033[1m"
/* Swaps the foreground and background colours. */
#define REVERSED "\033[7m"
/* Reset colour */
#define RESET "\033[0m"

#ifndef NDEBUG
/***** MACROS WHEN DEBUGGING IS ENABLED *****/

/* Registers a given thread to the debugger. Should therefore be called before DENTER() in the first function of a thread. */
#define DSTART(THREAD_NAME) \
    CppDebugger::debugger.start((THREAD_NAME));

/* Registers given function on the debugger's stacktrace. */
#define DENTER(FUNC_NAME) \
    CppDebugger::debugger.push((FUNC_NAME), (__FILE__), (__LINE__) - 1);
/* Pops the current frame from the stack only, but does not call return. */
#define DLEAVE \
    CppDebugger::debugger.pop();
/* Wraps the return statement, first popping the current value from the stack. */
#define DRETURN \
    CppDebugger::debugger.pop(); \
    return

/* Mutes function with the given name. */
#define DMUTE(FUNC_NAME) \
    CppDebugger::debugger.mute((FUNC_NAME));
/* Unmutes function with the given name. */
#define DUNMUTE(FUNC_NAME) \
    CppDebugger::debugger.mute((FUNC_NAME));

/* Increase the indent of the logger by N steps. */
#define DINDENT \
    CppDebugger::debugger.indent();
/* Decrease the indent of the logger by N steps. */
#define DDEDENT \
    CppDebugger::debugger.dedent();

/* Logs using the debugger. */
#define DLOG(SEVERITY, MESSAGE) \
    CppDebugger::debugger.log((SEVERITY), (MESSAGE));
/* Logs using the debugger with extra indent. */
#define DLOGi(SEVERITY, MESSAGE, INDENT) \
    CppDebugger::debugger.log((SEVERITY), (MESSAGE), (INDENT));


/* Increases the indent in the debug levels. */

#else
/***** MACROS WHEN DEBUGGING IS DISABLED *****/

/* Registers a given thread to the debugger. Should therefore be called before DENTER() in the first function of a thread. */
#define DSTART(THREAD_NAME)

/* Registers given function on the debugger's stacktrace. */
#define DENTER(FUNC_NAME)
/* Pops the current frame from the stack only, but does not call return. */
#define DLEAVE
/* Wraps the return statement, first popping the current value from the stack. */
#define DRETURN \
    return

/* Mutes function with the given name. */
#define DMUTE(FUNC_NAME)
/* Unmutes function with the given name. */
#define DUNMUTE(FUNC_NAME)

/* Increase the indent of the logger by N steps. */
#define DINDENT
/* Decrease the indent of the logger by N steps. */
#define DDEDENT

/* Logs using the debugger. */
#define DLOG(SEVERITY, MESSAGE) \
    if ((SEVERITY) == CppDebugger::Severity::nonfatal) { std::cerr << (MESSAGE) << std::endl; } \
    else if ((SEVERITY) == CppDebugger::Severity::fatal) { std::cerr << (MESSAGE) << std::endl; throw CppDebugger::Fatal((MESSAGE)); }
/* Logs using the debugger with extra indent. */
#define DLOGi(SEVERITY, MESSAGE, INDENT) \
    if ((SEVERITY) == CppDebugger::Severity::nonfatal) { std::cerr << (MESSAGE) << std::endl; } \
    else if ((SEVERITY) == CppDebugger::Severity::fatal) { std::cerr << (MESSAGE) << std::endl; throw CppDebugger::Fatal((MESSAGE)); }

#endif


/***** DEBUG NAMESPACE *****/
namespace CppDebugger {
    /* Enum that defines the possible debug message types. */
    namespace SeverityValues {
        enum type {
            /* Prints a message without any markings, just indents. */
            auxillary,
            /* Prints a message marked as general information. */
            info,
            /* Prints a message marked as a warning. */
            warning,
            /* Prints a message marked as an error. */
            nonfatal,
            /* Prints a message marked as an error, then throws a std::runtime_error. */
            fatal,
            /* Prints a message marked as a Vulkan warning. */
            vulkan_warning,
            /* Prints a message marked as a Vulkan error. */
            vulkan_error
        };
    }
    using Severity = SeverityValues::type;



    /* Exception that is thrown whenever a fatal message is logged. Derives from the standard std::exception class. */
    class Fatal: public std::exception {
    public:
        /* The message passed to the fatal error message. */
        std::string message;

        /* Constructor for the Fatal exception, which takes a message describing what happened. */
        Fatal(const std::string& message):
            message(message)
        {}

        /* Overload of the what() function, which is used to pretty-print the message. */
        const char* what() const noexcept { return this->message.c_str(); }
    };



    #ifndef NDEBUG
    
    /* Struct used to refer to a stack frame. */
    struct Frame {
        /* The name of the function we entered. */
        std::string func_name;
        /* The file where the function resides. */
        std::string file_name;
        /* The line number where the function is defined (i.e., the line above DENTER). */
        size_t line_number;
    };



    /* The main debug class, which is used to keep track of where we are and whether or not prints are accepted etc. It should be thread-safe, albeit probably quite slow. */
    class Debugger {
    public:
        /* The maximum linewidth before the debugger breaks lines. */
        static constexpr size_t max_line_width = 100;
        /* The size of each indent. */
        static constexpr size_t indent_size = 3;
        /* The length of the first prefix indent. */
        static constexpr size_t prefix_size = 10;

    private:
        /* Maps thread ids to programmer-readable names. */
        std::unordered_map<std::thread::id, std::string> thread_names;
        /* The stack of frames we're currently in. */
        std::unordered_map<std::thread::id, std::vector<Frame>> stack;
        /* List of currently muted functions. */
        std::unordered_map<std::thread::id, std::vector<std::string>> muted;
        /* Indents for each of the threads. */
        std::unordered_map<std::thread::id, size_t> indent_level;

        /* Flags if the current terminal supports color codes. */
        bool colour_enabled;
        /* The string that will be appended before all auxillary messages. */
        const std::string auxillary_msg;
        /* The string that will be appended before all info messages. */
        const std::string info_msg;
        /* The string that will be appended before all warning messages. */
        const std::string warning_msg;
        /* The string that will be appended before all error messages. */
        const std::string nonfatal_msg;
        /* The string that will be appended before all error messages. */
        const std::string fatal_msg;
        /* The string that will be appended before all Vulkan warnings. */
        const std::string vulkan_warning_msg;
        /* The string that will be appended before all Vulkan errors. */
        const std::string vulkan_error_msg;
        /* The string that will be appended to reset all colours. */
        const std::string reset_msg;

        /* Lock used to make the Debugger thread-safe. */
        std::mutex lock;
        /* Used to identify the 'main' thread. */
        std::thread::id main_tid;

        /* Prints a given string over multiple lines, pasting n spaces in front of each one and linewrapping on the target width. Optionally, a starting x can be specified. */
        void print_linewrapped(std::ostream& os, size_t& x, size_t width, const std::string& message);
        /* Actually prints the message to a given stream. */
        void _log(std::ostream& os, Severity severity, const std::string& message, size_t extra_indent);

    public:
        /* Default constructor for the Debugger class. */
        Debugger();

        /* Registers a new name for the current thread. */
        void start(const std::string& thread_name);
        
        /* Enters a new function, popping it's value on the stack. */
        void push(const std::string& function_name, const std::string& file_name, size_t line_number);
        /* pops the top function name of the stack. */
        void pop();

        /* Mutes a given function. All info-level severity messages that are called from it or from children functions are ignored. */
        void mute(const std::string& function_name);
        /* Unmutes a given function. All info-level severity messages that are called from it or from children functions are ignored. */
        void unmute(const std::string& function_name);

        /* Increases indents. Useful for when a helper function is called, for example. */
        void indent();
        /* Decreases indents. */
        void dedent();

        /* Logs a message to stdout. The type of message must be specified, which also determines how the message will be printed. If the the severity is fatal, also throws a std::runtime_error with the same text. To disable that, use Severity::nonfatal otherwise. Finally, one can optionally specify extra levels of indentation to use for this message. */
        void log(Severity severity, const std::string& message, size_t extra_indent = 0);

    };



    /* Tell the compiler that there is an global debugger instance. */
    extern Debugger debugger;
    #endif
};

#endif
