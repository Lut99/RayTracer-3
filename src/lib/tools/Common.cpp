/* COMMON.cpp
 *   by Lut99
 *
 * Created:
 *   02/05/2021, 17:12:29
 * Last edited:
 *   25/05/2021, 18:14:13
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains common helper functions used throughout the project.
**/

#ifdef _WIN32
#include <windows.h>
#else
#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#endif
#include <sstream>
#include <CppDebugger.hpp>

#include "Common.hpp"

using namespace std;
using namespace Tools;
using namespace CppDebugger::SeverityValues;


/***** EXPORTED FUNCTIONS *****/
/* Function that returns the path of the folder of the executable. */
std::string Tools::get_executable_path() {
    DENTER("Tools::get_executable_path");

    #ifdef _WIN32
    /* For Windows machiens */

    // First, get the executable name
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    
    // Strip the executable from it to only get the directory
    for (int i = MAX_PATH - 1; i >= 0; i--) {
        if (path[i] == '\\' || path[i] == '/') {
            path[i] = '\0';
            break;
        }
    }

    // Prepare the converting of unicode to ascii and then return the converted value
    DRETURN path;

    #else
    /* For Linux machiens */

    // Get the executable path
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

    // Strip the executable from it to only get the directory
    for (ssize_t i = count - 1; i >= 0; i--) {
        if (result[i] == '/') {
            result[i] = '\0';
            break;
        }
    }

    // Done, return
    DRETURN result;

    #endif
}



/* Function that returns a string more compactly describing the given number of bytes. */
std::string Tools::bytes_to_string(size_t n_bytes) {
    DENTER("Tools::bytes_to_string");

    // Select the proper scale to write the bytes to
    std::stringstream sstr;
    if (n_bytes < 1024) {
        // Write as normal bytes
        sstr << n_bytes << " bytes";
    } else if (n_bytes < 1024 * 1024) {
        // Write as kilobytes
        sstr << ((double) n_bytes / 1024.0) << " KiB";
    } else if (n_bytes < 1024 * 1024 * 1024) {
        // Write as megabytes
        sstr << ((double) n_bytes / (1024.0 * 1024.0)) << " MiB";
    } else {
        // Write as gigabytes
        sstr << ((double) n_bytes / (1024.0 * 1024.0 * 1024.0)) << " GiB";
    }
    
    // Done, return
    DRETURN sstr.str();
}
