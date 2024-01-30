#include <assert.h>

#include "__init__.h"
#include "utils/defs.h"

/**
 * @brief List of imported windows values.
 */
#define TC_IMPORT_WIN_VALUES(xx)    \
    /* Values from ShowWindow(). */\
    /* https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow */\
    xx(SW_HIDE)             \
    xx(SW_SHOWNORMAL)       \
    xx(SW_NORMAL)           \
    xx(SW_SHOWMINIMIZED)    \
    xx(SW_SHOWMAXIMIZED)    \
    xx(SW_MAXIMIZE)         \
    xx(SW_SHOWNOACTIVATE)   \
    xx(SW_SHOW)             \
    xx(SW_MINIMIZE)         \
    xx(SW_SHOWMINNOACTIVE)  \
    xx(SW_SHOWNA)           \
    xx(SW_RESTORE)          \
    xx(SW_SHOWDEFAULT)      \
    xx(SW_FORCEMINIMIZE)

typedef struct tc_win_value
{
    const char* name;
    int         value;
} tc_win_value_t;

/* TC APIs. */
static const tc_api_t* s_api[] = {
    &tc_api_shell_execute,
    &tc_api_split_path,
};

static void _luaopen_tc_api(lua_State* L, int idx)
{
    idx = lua_absindex(L, idx);

    size_t i;
    for (i = 0; i < ARRAY_SIZE(s_api); i++)
    {
        const tc_api_t* api = s_api[i];
        if (api->upvalue)
        {
            lua_pushvalue(L, idx);
            lua_pushcclosure(L, api->func, 1);
        }
        else
        {
            lua_pushcfunction(L, api->func);
        }

        lua_setfield(L, idx, api->name);
    }
}

static void _luaopen_tc_value(lua_State* L, int idx)
{
    idx = lua_absindex(L, idx);

    static tc_win_value_t s_vals[] = {
#define EXPAND_VALUE_AS_MAP(x) { #x, x },
        TC_IMPORT_WIN_VALUES(EXPAND_VALUE_AS_MAP)
#undef EXPAND_VALUE_AS_MAP
    };

    size_t i;
    for (i = 0; i < ARRAY_SIZE(s_vals); i++)
    {
        const tc_win_value_t* val = &s_vals[i];
        lua_pushinteger(L, val->value);
        lua_setfield(L, idx, val->name);
    }
}

int luaopen_tc(lua_State* L)
{
    lua_newtable(L);
    _luaopen_tc_api(L, -1);
    _luaopen_tc_value(L, -1);

    return 1;
}

int tc_raise_last_error(lua_State* L)
{
    DWORD errcode = GetLastError();

    LPWSTR lpMsgBuf = NULL;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_ENGLISH_US),
        (LPWSTR) &lpMsgBuf,
        0, NULL );

    tc_wide_to_utf8(L, lpMsgBuf);
    LocalFree(lpMsgBuf);

    return lua_error(L);
}

int tc_raise_error(lua_State* L, int errcode)
{
    const char* code = "Unknown error";
    switch(errcode)
    {
    case ERANGE: code = "Result too large"; break;
    case EINVAL: code = "Invalid argument"; break;
    default: break;
    }

    return luaL_error(L, "%s (%d).", code, errcode);
}

const char* tc_wide_to_utf8(lua_State* L, const WCHAR* str)
{
    if (str == NULL)
    {
        lua_pushnil(L);
        return NULL;
    }

    int utf8_sz = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char* utf8 = malloc(utf8_sz + 1);
    if (utf8 == NULL)
    {
        abort();
    }

    int ret = WideCharToMultiByte(CP_UTF8, 0, str, -1, utf8, utf8_sz, NULL, NULL);
    assert(ret == utf8_sz);
    utf8[utf8_sz] = '\0';

    lua_pushlstring(L, utf8, utf8_sz);
    free(utf8); utf8 = NULL;

    return (char*)lua_tostring(L, -1);
}

const WCHAR* tc_utf8_to_wide(lua_State* L, const char* str)
{
    if (str == NULL)
    {
        lua_pushnil(L);
        return NULL;
    }

    int multi_byte_sz = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    size_t buf_sz = multi_byte_sz * sizeof(WCHAR);

    WCHAR* buf = (WCHAR*)malloc(buf_sz);
    int ret = MultiByteToWideChar(CP_UTF8, 0, str, -1, (WCHAR*)buf, multi_byte_sz);
    assert(ret == multi_byte_sz);

    lua_pushlstring(L, (const char*)buf, buf_sz);
    free(buf); buf = NULL;

    return (WCHAR*)lua_tostring(L, -1);
}
