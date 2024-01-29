#include "__init__.h"

#include <shellapi.h>

static int _shell_execute_2(lua_State* L)
{
    int sp = lua_gettop(L);

    const char* lpOperation = luaL_checkstring(L, 1);
    const char* lpFile = luaL_checkstring(L, 2);
    const char* lpParameters = luaL_checkstring(L, 3);
    const char* lpDirectory = luaL_checkstring(L, 4);
    int nShowCmd = (int)luaL_checkinteger(L, 5);

    const WCHAR* lpOperationW = tc_utf8_to_wide(L, lpOperation);
    const WCHAR* lpFileW = tc_utf8_to_wide(L, lpFile);
    const WCHAR* lpParametersW = tc_utf8_to_wide(L, lpParameters);
    const WCHAR* lpDirectoryW = tc_utf8_to_wide(L, lpDirectory);

    HINSTANCE ret = ShellExecuteW(NULL,
        lpOperationW,
        lpFileW,
        lpParametersW,
        lpDirectoryW,
        nShowCmd);

    /* Restore stack. */
    lua_settop(L, sp);

    if (ret <= (HINSTANCE)32)
    {
        return tc_raise_last_error(L);
    }

    return 0;
}

static BOOL CALLBACK _on_init_shell_execute(PINIT_ONCE InitOnce,
    PVOID Parameter, PVOID *lpContext)
{
    (void)InitOnce; (void)Parameter; (void)lpContext;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    return TRUE;
}

static int _shell_execute(lua_State* L)
{
    static INIT_ONCE token = INIT_ONCE_STATIC_INIT;
    InitOnceExecuteOnce(&token, _on_init_shell_execute, NULL, NULL);

    return _shell_execute_2(L);
}

const tc_api_t tc_api_shell_execute = {
    "ShellExecute", _shell_execute, 0,
    "Performs an operation on a specified file.",

    "[SEE ALSO]\n"
    "https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew"
};
