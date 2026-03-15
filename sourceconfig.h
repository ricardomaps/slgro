#ifndef CONFIG_H
#define CONFIG_H

#include <xkbcommon/xkbcommon-keysyms.h>


#include "include/types.h"
#include "include/slgro.h"

static const struct config cfg = {
	.motion_throttle_hz = 85,
	.border_col_active = 0xe8e0d8ff,
	.border_col_normal = 0x00000000,
	.border_width = 2,
};

static const char* termcmd[]       = { "havoc", NULL };
static const char* menucmd[]       = { "/home/dani/Scripts/launcher.sh", NULL };
static const char* browsercmd[]    = { "librewolf-bin", NULL };
static const char* filescmd[]      = { "pcmanfm", NULL };
static const char* btcmd[]         = { "blueman-manager", NULL };
static const char* volcmd[]        = { "pavucontrol", NULL };
static const char* screenshotcmd[] = { "/home/dani/Scripts/screenshot.sh", NULL };

static struct bind binds[] = {
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_q,      { .v = termcmd },       spawn },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_d,      { .v = menucmd },       spawn },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_w,      { .v = browsercmd },    spawn },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_e,      { .v = filescmd },      spawn },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_b,      { .v = btcmd },         spawn },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_v,      { .v = volcmd },        spawn },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_s,      { .v = screenshotcmd }, spawn },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Tab,    { .v = NULL },    focus_next },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_f,      { .v = NULL },    fullscreen },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_c,      { .v = NULL },    kill_sel },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Right,  { .i = 75 },     kb_move_x },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Left,   { .i = -75 },     kb_move_x },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Down,   { .i = 75 },     kb_move_y },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Up,     { .i = -75 },     kb_move_y },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_space,  { .v = NULL },    center_window },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Right,  { .i = 25 },     kb_resize_width },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Left,   { .i = -25 },     kb_resize_width },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Down,   { .i = 25 },     kb_resize_height },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Up,     { .i = -25 },     kb_resize_height },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_h,      { .v = NULL },    snap_left_half },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_l,      { .v = NULL },    snap_right_half },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_1,      { .ui = 1 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_2,      { .ui = 2 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_3,      { .ui = 3 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_4,      { .ui = 4 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_5,      { .ui = 5 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_6,      { .ui = 6 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_7,      { .ui = 7 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_8,      { .ui = 8 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_9,      { .ui = 9 },      workspace_goto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_1,      { .ui = 1 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_2,      { .ui = 2 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_3,      { .ui = 3 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_4,      { .ui = 4 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_5,      { .ui = 5 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_6,      { .ui = 6 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_7,      { .ui = 7 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_8,      { .ui = 8 },      workspace_moveto },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_9,      { .ui = 9 },      workspace_moveto },

};

#endif /* CONFIG_H */