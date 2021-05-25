/* CPP_DEBUGGER.cpp
 *   by Lut99
 *
 * Created:
 *   19/12/2020, 16:32:34
 * Last edited:
 *   19/12/2020, 16:32:34
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

#include <algorithm>
#include <stdexcept>
#ifdef _WIN32
#include "windows.h"
#endif

#include "CppDebugger.hpp"

using namespace CppDebugger;


/***** GLOBALS *****/
#ifndef NDEBUG
/* Global instance of the debug class, used for debugging. */
Debugger CppDebugger::debugger;
#endif





/***** HELPER FUNCTIONS *****/
/* Returns whether or not the associated terminal supports ANSI color codes. */
static bool terminal_supports_colours() {
    #ifdef _WIN32
    // For Windows, we check
    DWORD modes;
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &modes);
    return modes & ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    #else
    // Let's assume the rest does
    return true;
    #endif
}





/***** DEBUGGER CLASS *****/
/* Default constructor for the Debugger class. */
Debugger::Debugger() :
    colour_enabled(terminal_supports_colours()),
    auxillary_msg("          "),
    info_msg(this->colour_enabled ? "[" BOLD "  LOG  " RESET "] " : "[  LOG  ] "),
    warning_msg(this->colour_enabled ? "[" YELLOW "WARNING" RESET "] " : "[WARNING] "),
    nonfatal_msg(this->colour_enabled ? "[" RED " ERROR " RESET "] " : "[ ERROR ] "),
    fatal_msg(this->colour_enabled ? "[" RED REVERSED " ERROR " RESET "] " : "[ ERROR ] "),
    vulkan_warning_msg(this->colour_enabled ? "[" YELLOW "VK WARN" RESET "] " : "[VK WARN] "),
    vulkan_error_msg(this->colour_enabled ? "[" RED "VKERROR" RESET "] " : "[VKERROR] "),
    reset_msg(this->colour_enabled ? RESET : ""),
    main_tid(std::this_thread::get_id())
{}



/* Prints a given string over multiple lines, pasting n spaces in front of each one and linewrapping on the target width. Optionally, a starting x can be specified. */
void Debugger::print_linewrapped(std::ostream& os, size_t& x, size_t width, const std::string& message) {
    // Get the string to be pasted in front of every new line
    std::string prefix = std::string(Debugger::prefix_size + this->indent_level[std::this_thread::get_id()] * Debugger::indent_size, ' ');
    // Loop to print each character
    bool ignoring = false;
    for (size_t i = 0; i < message.size(); i++) {
        // If we're seeing a '\033', ignore until an 'm' is reached
        if (!ignoring && message[i] == '\033') { ignoring = true; }
        else if (ignoring && message[i] == 'm') { ignoring = false; }

        // Otherwise, check if we should print a newline (only when we're not printing color codes)
        if (!ignoring && ++x >= width) {
            os << std::endl << prefix;
            x = 0;
        }

        // Print the character itself
        os << message[i];
    }
}

/* Actually prints the message to the given output stream. */
void Debugger::_log(std::ostream& os, Severity severity, const std::string& message, size_t extra_indent) {
    using namespace SeverityValues;

    // Acquire the lock for this function
    std::unique_lock<std::mutex> _log_lock(this->lock);

    // Get the thread ID and, optionally, its name if defined
    std::thread::id tid = std::this_thread::get_id();
    std::string thread_name;
    std::unordered_map<std::thread::id, std::string>::const_iterator iter = this->thread_names.find(tid);
    if (iter != this->thread_names.end()) {
        thread_name = (*iter).second;
    }

    // Determine the type of message to preprend to signify how it went
    switch(severity) {
        case auxillary:
            // Print as a prefix; i.e., a plain message with correct indents
            {
                // If this function is actually muted and we have a stack (so we know of this function), do not display the message
                if (this->stack.size() > 0) {
                    for (size_t i = 0; i < this->muted.size(); i++) {
                        if (this->muted[tid][i] == this->stack[tid][this->stack[tid].size() - 1].func_name) {
                            // Do not do anything
                            return;
                        }
                    }
                }

                // Otherwise, we're clear to print the message
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size - this->indent_level[tid] * Debugger::indent_size;
                os << auxillary_msg << std::string(this->indent_level[tid] * Debugger::indent_size, ' ');
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;
                return;
            }

        case info:
            // Print as info; pretty much the same, except that we now prepent with the info message
            {
                // If this function is actually muted and we have a stack, do not display the message
                if (this->stack.size() > 0) {
                    for (size_t i = 0; i < this->muted.size(); i++) {
                        if (this->muted[tid][i] == this->stack[tid][this->stack[tid].size() - 1].func_name) {
                            // Do not do anything
                            return;
                        }
                    }
                }

                // Otherwise, we're clear to print the message
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size - this->indent_level[tid] * Debugger::indent_size;
                os << info_msg << std::string(this->indent_level[tid] * Debugger::indent_size, ' ');
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;
                return;
            }

        case warning:
            // Print as warning; the same as info with a different prexif, plus we print the origin of the line
            {
                // If this function is actually muted and we have a stack, do not display the message
                if (this->stack.size() > 0) {
                    for (size_t i = 0; i < this->muted.size(); i++) {
                        if (this->muted[tid][i] == this->stack[tid][this->stack[tid].size() - 1].func_name) {
                            // Do not do anything
                            return;
                        }
                    }
                }

                // Print the new message as normal
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size - this->indent_level[tid] * Debugger::indent_size;
                os << warning_msg << std::string(this->indent_level[tid] * Debugger::indent_size, ' ');
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;

                // If there is a stack, display the stack message
                if (this->stack[tid].size() > 0) {
                    Frame f = this->stack[tid][this->stack[tid].size() - 1];
                    std::string to_print = "[in function '\033[1m" + f.func_name + "\033[0m' at \033[1m" + f.file_name + ':' + std::to_string(f.line_number) + "\033[0m]";
                    os << std::string(Debugger::prefix_size + this->indent_level[tid] * Debugger::indent_size, ' ');
                    x = 0;
                    this->print_linewrapped(os, x, width, to_print);
                    os << reset_msg << std::endl;
                }

                return;
            }

        case nonfatal:
            // Print as nonfatal error; add some extra spacing around the message, and provide the full stacktrace
            {
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size;
                os << nonfatal_msg;
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;

                // Print a stacktrace, if any
                std::string prefix_indent = std::string(Debugger::prefix_size, ' ');
                if (this->stack.size() > 0) {
                    os << prefix_indent << "\033[1mStacktrace:\033[0m" << std::endl;
                    for (size_t i = 0; i < this->stack[tid].size(); i++) {
                        x = 0;
                        Frame f = this->stack[tid][this->stack[tid].size() - 1 - i];
                        std::string prefix = i == 0 ? "in" : "from";
                        this->print_linewrapped(os, x, width, prefix_indent + prefix + " function '\033[1m" + f.func_name + "\033[0m' at \033[1m" + f.file_name + ':' + std::to_string(f.line_number) + "\033[0m");
                        os << reset_msg << std::endl;
                    }
                    os << prefix_indent << "from thread \033[1m" << tid << "\033[0m" << (thread_name.empty() ? "" : " (" + thread_name + ")") << std::endl;
                    os << std::endl;
                }

                return;
            }

        case fatal:
            // Print as fatal error; add some extra spacing around the message, provide the full stacktrace, then throw the error
            {
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size;
                os << fatal_msg;
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;

                // Print a stacktrace, if any
                std::string prefix_indent = std::string(Debugger::prefix_size, ' ');
                if (this->stack.size() > 0) {
                    os << prefix_indent << "\033[1mStacktrace:\033[0m" << std::endl;
                    for (size_t i = 0; i < this->stack[tid].size(); i++) {
                        x = 0;
                        Frame f = this->stack[tid][this->stack[tid].size() - 1 - i];
                        std::string prefix = i == 0 ? "in" : "from";
                        this->print_linewrapped(os, x, width, prefix_indent + prefix + " function '\033[1m" + f.func_name + "\033[0m' at \033[1m" + f.file_name + ':' + std::to_string(f.line_number) + "\033[0m");
                        os << reset_msg << std::endl;
                    }
                    os << prefix_indent << "from thread \033[1m" << tid << "\033[0m" << (thread_name.empty() ? "" : " (" + thread_name + ")") << std::endl;
                    os << std::endl;
                }

                // Return by throwing
                throw CppDebugger::Fatal(message);
            }
        
        case vulkan_warning:
            // Print as vulkan warning message
            {
                // If this function is actually muted and we have a stack, do not display the message
                if (this->stack.size() > 0) {
                    for (size_t i = 0; i < this->muted.size(); i++) {
                        if (this->muted[tid][i] == this->stack[tid][this->stack[tid].size() - 1].func_name) {
                            // Do not do anything
                            return;
                        }
                    }
                }

                // Print the new message as normal
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size - this->indent_level[tid] * Debugger::indent_size;
                os << vulkan_warning_msg << std::string(this->indent_level[tid] * Debugger::indent_size, ' ');
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;

                // If there is a stack, display the stack message
                if (this->stack[tid].size() > 0) {
                    Frame f = this->stack[tid][this->stack[tid].size() - 1];
                    std::string to_print = "[in function '\033[1m" + f.func_name + "\033[0m' at \033[1m" + f.file_name + ':' + std::to_string(f.line_number) + "\033[0m]";
                    os << std::string(Debugger::prefix_size + this->indent_level[tid] * Debugger::indent_size, ' ');
                    x = 0;
                    this->print_linewrapped(os, x, width, to_print);
                    os << reset_msg << std::endl;
                }

                return;
            }

        case vulkan_error:
            // Print as nonfatal vulkan error
            {
                size_t x = 0;
                size_t width = max_line_width - Debugger::prefix_size;
                os << vulkan_error_msg;
                this->print_linewrapped(os, x, width, message);
                os << reset_msg << std::endl;

                // Print a stacktrace, if any
                std::string prefix_indent = std::string(Debugger::prefix_size, ' ');
                if (this->stack.size() > 0) {
                    os << prefix_indent << "\033[1mStacktrace:\033[0m" << std::endl;
                    for (size_t i = 0; i < this->stack[tid].size(); i++) {
                        x = 0;
                        Frame f = this->stack[tid][this->stack[tid].size() - 1 - i];
                        std::string prefix = i == 0 ? "in" : "from";
                        this->print_linewrapped(os, x, width, prefix_indent + prefix + " function '\033[1m" + f.func_name + "\033[0m' at \033[1m" + f.file_name + ':' + std::to_string(f.line_number) + "\033[0m");
                        os << reset_msg << std::endl;
                    }
                    os << prefix_indent << "from thread \033[1m" << tid << "\033[0m" << (thread_name.empty() ? "" : " (" + thread_name + ")") << std::endl;
                    os << std::endl;
                }

                return;
            }

        default:
            // Let's re-do as auxillary
            this->_log(os, auxillary, message, extra_indent);
    }
}



/* Registers a new name for the current thread. */
void Debugger::start(const std::string& thread_name) {
    // Check if it already exists
    std::unordered_map<std::thread::id, std::string>::iterator iter = this->thread_names.find(std::this_thread::get_id());
    if (iter != this->thread_names.end()) {
        // It already does; simply replace the name
        (*iter).second = thread_name;
    } else {
        // It doesn't; insert it
        std::unique_lock<std::mutex> _start_lock(this->lock);
        this->thread_names.insert(std::make_pair(std::this_thread::get_id(), thread_name));
    }
}



/* Enters a new function, popping it's value on the stack. */
void Debugger::push(const std::string& function_name, const std::string& file_name, size_t line_number) {
    // Create a new stack frame
    Frame frame({ function_name, file_name, line_number });

    // Push it on the stack
    this->stack[std::this_thread::get_id()].push_back(frame);
}

/* pops the top function name of the stack. */
void Debugger::pop() {
    // Only pop if there are elements left
    if (this->stack[std::this_thread::get_id()].size() > 0) {
        // Pop the vector
        this->stack[std::this_thread::get_id()].pop_back();
    }
}



/* Mutes a given function. All info-level severity messages that are called from it or from children functions are ignored. */
void Debugger::mute(const std::string& function_name) {
    // Add the given function to the vector of muted functions
    this->muted[std::this_thread::get_id()].push_back(function_name);
}

/* Unmutes a given function. All info-level severity messages that are called from it or from children functions are ignored. */
void Debugger::unmute(const std::string& function_name) {
    // Try to find the function name
    for (std::vector<std::string>::iterator iter = this->muted[std::this_thread::get_id()].begin(); iter != this->muted[std::this_thread::get_id()].end(); ++iter) {
        if (*iter == function_name) {
            // Found it; remove it and we're done
            this->muted[std::this_thread::get_id()].erase(iter);
            return;
        }
    }

    // Not found, so nothing to remove
    return;
}



/* Increases indents. Useful for when a helper function is called, for example. */
void Debugger::indent() {
    // Simply add one to the value
    ++this->indent_level[std::this_thread::get_id()]; 
}

/* Decreases indents. */
void Debugger::dedent() {
    // Subtract one from the value, but make sure it's bounded to not go below zero and overflow
    std::thread::id tid = std::this_thread::get_id();
    this->indent_level[tid] = (this->indent_level[tid] > 0 ? this->indent_level[tid] - 1 : 0); 
}



/* Logs a message to the debugger. The type of message must be specified, which also determines how the message will be printed. If the the severity is fatal, also throws a CppDebugger::Fatal with the same text. To disable that, use Severity::nonfatal otherwise. Finally, one can optionally specify extra levels of indentation to use for this message. */
void Debugger::log(Severity severity, const std::string& message, size_t extra_indent) {
    // Write to the correct stream based on the severity
    if (severity == Severity::info || severity == Severity::auxillary) {
        this->_log(std::cout, severity, message, extra_indent);
    } else {
        this->_log(std::cerr, severity, message, extra_indent);
    }
}
