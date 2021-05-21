/* COMMON.hpp
 *   by Lut99
 *
 * Created:
 *   02/05/2021, 17:12:22
 * Last edited:
 *   21/05/2021, 15:20:52
 * Auto updated?
 *   Yes
 *
 * Description:
 *   Contains common helper functions used throughout the project.
**/

#ifndef TOOLS_COMMON_HPP
#define TOOLS_COMMON_HPP

#include <string>

namespace Tools {
    /* Function that returns the path of the folder of the executable. */
    std::string get_executable_path();

    /* Function that returns a string more compactly describing the given number of bytes. */
    std::string bytes_to_string(size_t n_bytes);
}

#endif
