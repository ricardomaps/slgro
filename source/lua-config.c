#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "include/types.h"
#include "include/slgro.h"

struct config cfg;
struct bind *binds = NULL;
size_t nbinds = 0;

static char *spawn_args[128][2];
static size_t nspawn = 0;

static uint32_t parse_mods(const char *s)
{
    uint32_t mods = 0;
    char buf[64];
    strncpy(buf, s, sizeof(buf) - 1);
    char *tok = strtok(buf, "|");
    while (tok) {
        if      (!strcmp(tok, "MOD4")) mods |= SWC_MOD_LOGO;
        else if (!strcmp(tok, "MOD1")) mods |= SWC_MOD_ALT;
        else if (!strcmp(tok, "SHFT")) mods |= SWC_MOD_SHIFT;
        else if (!strcmp(tok, "CTRL")) mods |= SWC_MOD_CTRL;
        tok = strtok(NULL, "|");
    }
    return mods;
}

static uint32_t parse_key(const char *s)
{
    if (s[1] == '\0') {
        if (s[0] >= 'a' && s[0] <= 'z') return XKB_KEY_a + (s[0] - 'a');
        if (s[0] >= '1' && s[0] <= '9') return XKB_KEY_1 + (s[0] - '1');
        if (s[0] == '0') return XKB_KEY_0;
    }
    if (!strcmp(s, "Return")) return XKB_KEY_Return;
    if (!strcmp(s, "Tab"))    return XKB_KEY_Tab;
    if (!strcmp(s, "space"))  return XKB_KEY_space;
    if (!strcmp(s, "Right"))  return XKB_KEY_Right;
    if (!strcmp(s, "Left"))   return XKB_KEY_Left;
    if (!strcmp(s, "Up"))     return XKB_KEY_Up;
    if (!strcmp(s, "Down"))   return XKB_KEY_Down;
    fprintf(stderr, "slgro: sorry, unknown key >.< '%s'\n", s);
    return XKB_KEY_VoidSymbol;
}

static void resolve_action(const char *action, const char *arg_str, int arg_int, struct bind *b)
{
    b->type = SWC_BINDING_KEY;
    if (!strcmp(action, "spawn")) {
        spawn_args[nspawn][0] = strdup(arg_str);
        spawn_args[nspawn][1] = NULL;
        b->arg.v = spawn_args[nspawn++];
        b->fn = spawn;
    }
    else if (!strcmp(action, "kill"))             b->fn = kill_sel;
    else if (!strcmp(action, "focus_next"))       b->fn = focus_next;
    else if (!strcmp(action, "fullscreen"))       b->fn = fullscreen;
    else if (!strcmp(action, "center"))           b->fn = center_window;
    else if (!strcmp(action, "snap_left"))        b->fn = snap_left_half;
    else if (!strcmp(action, "snap_right"))       b->fn = snap_right_half;
    else if (!strcmp(action, "quit"))             b->fn = quit;
    else if (!strcmp(action, "move_x"))         { b->fn = kb_move_x;        b->arg.i  = arg_int; }
    else if (!strcmp(action, "move_y"))         { b->fn = kb_move_y;        b->arg.i  = arg_int; }
    else if (!strcmp(action, "resize_width"))   { b->fn = kb_resize_width;  b->arg.i  = arg_int; }
    else if (!strcmp(action, "resize_height"))  { b->fn = kb_resize_height; b->arg.i  = arg_int; }
    else if (!strcmp(action, "workspace_goto"))   { b->fn = workspace_goto;   b->arg.ui = arg_int; }
    else if (!strcmp(action, "workspace_moveto")) { b->fn = workspace_moveto; b->arg.ui = arg_int; }
    else fprintf(stderr, "slgro: sorry, unknown action >.< '%s'\n", action);
}

void load_config(void)
{
    const char *home = getenv("HOME");
    char path[256];
    snprintf(path, sizeof(path), "%s/.config/slgro/config.lua", home ? home : ".");

    cfg.motion_throttle_hz   = 85;

    cfg.border_col_active    = 0xffffffff;
    cfg.border_col_normal    = 0xffffffff;
    cfg.border_width         = 2;

    cfg.border_col_active_outer = 0;
    cfg.border_col_normal_outer = 0;
    cfg.border_width_outer      = 0;

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, path) != LUA_OK) {
        if (luaL_dofile(L, "/usr/share/slgro/config.lua") != LUA_OK) {
            fprintf(stderr, "slgro: %s\n", lua_tostring(L, -1));
            fprintf(stderr, "slgro: using defaults :P\n");
            lua_close(L);
            return;
        }
    }

    lua_getglobal(L, "border_active");
    if (lua_isnumber(L, -1)) cfg.border_col_active = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "border_normal");
    if (lua_isnumber(L, -1)) cfg.border_col_normal = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "border_width");
    if (lua_isnumber(L, -1)) cfg.border_width = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "border_active_outer");
    if (lua_isnumber(L, -1)) cfg.border_col_active_outer = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "border_normal_outer");
    if (lua_isnumber(L, -1)) cfg.border_col_normal_outer = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "border_width_outer");
    if (lua_isnumber(L, -1)) cfg.border_width_outer = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "motion_throttle_hz");
    if (lua_isnumber(L, -1)) cfg.motion_throttle_hz = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getglobal(L, "binds");
    if (!lua_istable(L, -1)) {
        fprintf(stderr, "slgro: couldn't find the binds table :<\n");
        lua_close(L);
        return;
    }

    int binds_index = lua_gettop(L);
    size_t n = lua_objlen(L, binds_index);
    binds = calloc(n, sizeof(struct bind));

    for (size_t i = 1; i <= n; i++) {
        lua_rawgeti(L, binds_index, i);
        struct bind b = {0};

        lua_getfield(L, -1, "mods");
        if (lua_isstring(L, -1)) b.mods = parse_mods(lua_tostring(L, -1));
        lua_pop(L, 1);

        lua_getfield(L, -1, "key");
        if (lua_isstring(L, -1)) b.ksym = parse_key(lua_tostring(L, -1));
        lua_pop(L, 1);

        lua_getfield(L, -1, "action");
        const char *action = lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "arg");
        const char *arg_str = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
        int         arg_int = lua_isnumber(L, -1) ? (int)lua_tonumber(L, -1) : 0;
        lua_pop(L, 1);

        if (action) resolve_action(action, arg_str, arg_int, &b);
        binds[nbinds++] = b;
        lua_pop(L, 1);
    }

    lua_close(L); /* man i love lua :3 */
}
