#ifndef CONFIG_H
#define CONFIG_H

#include <xkbcommon/xkbcommon-keysyms.h>


#include "include/types.h"
#include "include/slgro.h"

static const struct config cfg = {
	.motion_throttle_hz = 85,
	.border_col_active = 0xffed953e,
	.border_col_normal = 0xff444444,
	.border_width = 1,
	.gaps = 0,
};

static const char* termcmd[] = { "st-wl", NULL };
static struct bind binds[] = {
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Return, { .v = termcmd }, spawn },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_q,      { .v = NULL },    quit },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Tab,    { .v = NULL },    focus_next },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_f,      { .v = NULL },    fullscreen },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_q,      { .v = NULL },    kill_sel },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Right,  { .i =  100 },     kb_move_x },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Left,   { .i = -100 },     kb_move_x },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Down,   { .i =  100 },     kb_move_y },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_Up,     { .i = -100 },     kb_move_y },
	{ SWC_BINDING_KEY,    MOD4,        XKB_KEY_space,  { .v = NULL },    center_window },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Right,  { .i =  50 },     kb_resize_width },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Left,   { .i = -50 },     kb_resize_width },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Down,   { .i =  50 },     kb_resize_height },
	{ SWC_BINDING_KEY,    MOD4|SHFT,   XKB_KEY_Up,     { .i = -50 },     kb_resize_height },
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
