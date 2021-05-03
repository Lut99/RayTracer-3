/* COMMON.cpp
 *   by Lut99
 *
 * Created:
 *   02/05/2021, 17:12:29
 * Last edited:
 *   03/05/2021, 14:55:17
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
    WCHAR path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    
    // Next, strip the path off it using the correct windows' version's counterpart
    #if (NTDDI_VERSION >= NTDDI_WIN8)
    PathCchRemoveFileSpec(path, MAX_PATH);
    #else
    PathRemoveFileSpec(path);
    #endif

    // Done, return
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
