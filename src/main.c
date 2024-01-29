#include <stdlib.h>
#include "tc/__init__.h"

typedef struct tc_ctx
{
    lua_State* L;
} tc_ctx_t;

static tc_ctx_t G = { NULL };

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

static int _pmain(lua_State* L)
{
	/* Open standard libraries. */
    luaL_openlibs(L);

	/* Open TC Lua module. */
	luaL_requiref(L, "tc", luaopen_tc, 1);
	lua_pop(L, 1);

	int argc = (int)lua_tointeger(L, 1);
    char** argv = lua_touserdata(L, 2);
	if (argc < 2)
	{
		return luaL_error(L, "missing argument as file.");
	}

	const char* path = argv[1];
	if (luaL_dofile(L, path) != 0)
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

    lua_pushcfunction(G.L, _msg_handler);
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
