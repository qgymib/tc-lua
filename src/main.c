#include <stdlib.h>
#include "tc/__init__.h"

typedef struct tc_ctx
{
    lua_State* L;	/**< Lua VM. */
} tc_ctx_t;

/**
 * @brief Global context.
 */
static tc_ctx_t G = { NULL };

/**
 * @brief Global program exit hook.
 */
static void _at_exit(void)
{
    if (G.L != NULL)
    {
        lua_close(G.L);
        G.L = NULL;
    }
}

/**
 * @brief Lua error traceback helper.
 * @param[in] L     Lua VM.
 * @return          Always 1.
 */
static int _msg_handler(lua_State* L)
{
    const char* msg = lua_tostring(L, 1);
    if (msg == NULL)
    {  /* is error object not a string? */
        if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
            lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
        {
            return 1;  /* that is the message */
        }

        msg = lua_pushfstring(L, "(error object is a %s value)",
            luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
    return 1;  /* return the traceback */
}

/**
 * @brief Get filename from path.
 * @param[in] file  Full file path.
 * @return          File name.
 */
static const char* _filename(const char* file)
{
    const char* pos = file;

    if (file == NULL)
    {
        return NULL;
    }

    for (; *file; ++file)
    {
        if (*file == '\\' || *file == '/')
        {
            pos = file + 1;
        }
    }
    return pos;
}

/**
 * @brief Handle command line arguments.
 * @param[in] L     Lua VM.
 * @return          Always 1, the path of script.
 */
static int _handle_cmd_args(lua_State* L)
{
    int argc = (int)lua_tointeger(L, 1);
    char** argv = lua_touserdata(L, 2);
    if (argc < 2)
    {
        const char* prog_name = argv != NULL ? argv[0] : "tc";
        prog_name = _filename(prog_name);
        printf("%s usage:\n"
            "  %s <file> [options]\n", prog_name, prog_name);

        exit(0);
    }

    /* Table to store command line arguments. */
    lua_newtable(L);

    /* Name of the program. */
    lua_pushstring(L, argv[0]);
    lua_seti(L, -2, -1);

    /* Name of the script. */
    lua_pushstring(L, argv[1]);
    lua_seti(L, -2, 0);

    /* Set remaining arguments. */
    int i;
    for (i = 1; i < argc; i++)
    {
        lua_pushstring(L, argv[i]);
        lua_seti(L, -2, i);
    }

    /* Set global `arg` table. */
    lua_setglobal(L, "arg");

    /* Path of script. */
    lua_pushstring(L, argv[1]);
    return 1;
}

/**
 * @brief Lua VM Entry point.
 * 
 * Lua value index requirements:
 *  1. The first argument is the number of arguments.
 *  2. The second argument is the argument list.
 * 
 * @param[in] L     Lua VM.
 * @return          Always 0.
 */
static int _pmain(lua_State* L)
{
    /* Open standard libraries. */
    luaL_openlibs(L);

    /* Open TC Lua module. */
    luaL_requiref(L, "tc", luaopen_tc, 1);
    lua_pop(L, 1);

    /* Handle command line arguments. */
    lua_pushcfunction(L, _handle_cmd_args);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_call(L, 2, 1);

    if (luaL_dofile(L, lua_tostring(L, -1)) != 0)
    {
        return lua_error(L);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    /* Set global exit hook. */
    atexit(_at_exit);

    /* Create Lua state. */
    if ((G.L = luaL_newstate()) == NULL)
    {
        abort();
    }

    /* Jump into Lua VM. */
    lua_pushcfunction(G.L, _msg_handler);   // Exception handler.
    lua_pushcfunction(G.L, _pmain);
    lua_pushinteger(G.L, argc);
    lua_pushlightuserdata(G.L, argv);
    if (lua_pcall(G.L, 2, 0, 1) != LUA_OK)
    {
        fprintf(stderr, "%s", lua_tostring(G.L, -1));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
