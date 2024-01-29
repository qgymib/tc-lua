#ifndef __TC_LUA_TC_INIT_H__
#define __TC_LUA_TC_INIT_H__

#define _WIN32_WINNT 0x0600
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef struct tc_api
{
    const char*     name;       /**< API name */
    lua_CFunction   func;       /**< API function. */

    /**
     * @brief Whether require upvalue.
     *
     * If set, get upvalue by `lua_getupvalue(L, 1)`.
     * The KEY must be prefixed with `__u_`.
     *
     * @warning Never access infra C API which need upvalue.
     */
    int             upvalue;

    const char*     brief;      /**< Brief description. */
    const char*     document;   /**< Document. */
} tc_api_t;

extern const tc_api_t tc_api_shell_execute;

/**
 * @brief Open Lua module.
 * @param[in] L     Lua VM.
 * @return          Always 1.
 */
int luaopen_tc(lua_State* L);

/**
 * @brief Raise last error to Lua stack.
 * @param[in] L     Lua VM.
 * @return          This function does not return.
 */
int tc_raise_last_error(lua_State* L);

/**
 * @brief Maps a UTF-16 (wide character) string to a new character string, push the
 *   result on top of stack, and return it's address.
 * 
 * @param[in] str   Wide character string.
 * @return          UTF-8 string. Don't free() it, as Lua take care of
 *                  the life cycle.
 */
const char* tc_wide_to_utf8(lua_State* L, const WCHAR* str);

/**
 * @brief Maps a character string to a UTF-16 (wide character) string, push the
 *   result on top of stack, and return it's address.
 * 
 * @param[in] str   UTF-8 string.
 * @return          Wide character string. Don't free() it, as Lua take care of
 *                  the life cycle.
 */
const WCHAR* tc_utf8_to_wide(lua_State* L, const char* str);

#ifdef __cplusplus
}
#endif

#endif
