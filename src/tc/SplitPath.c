#include "__init__.h"
#include "utils/defs.h"

typedef struct split_path_helper
{
    char    drive[_MAX_DRIVE];
    char    dir[4096];
    char    fname[4096];
    char    ext[_MAX_EXT];
} split_path_helper_t;

static int _split_path(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);

    split_path_helper_t* helper = malloc(sizeof(split_path_helper_t));
    errno_t errcode = _splitpath_s(path,
        helper->drive, sizeof(helper->drive),
        helper->dir, sizeof(helper->dir),
        helper->fname, sizeof(helper->fname),
        helper->ext, sizeof(helper->ext));
    if (errcode != 0)
    {
        free(helper);
        return tc_raise_error(L, errcode);
    }

    lua_pushfstring(L, "%s", helper->drive);
    lua_pushfstring(L, "%s", helper->dir);
    lua_pushfstring(L, "%s", helper->fname);
    lua_pushfstring(L, "%s", helper->ext);
    free(helper);

    return 4;
}

const tc_api_t tc_api_split_path = {
"SplitPath", _split_path, 0,
"Break a path into components.",

"[SYNOPSIS]\n"
"string,string SplitPath(string s)\n"
"\n"
"[DESCRIPTION]\n"
"SplitPath() breaks a path into 4 parts:\n"
"    + drive\n"
"    + dir\n"
"    + fname\n"
"    + ext\n"
};
