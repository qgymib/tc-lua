#ifndef __TC_LUA_UTILS_DEFS_H__
#define __TC_LUA_UTILS_DEFS_H__

/**
 * @brief Get the number of element in array.
 * @param[in] a Array.
 * @return      Element count.
 */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#if defined(_WIN32)
#   define strdup(s) _strdup(s)
#endif

#endif
