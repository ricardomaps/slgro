border_active          = 0xffffffff
border_normal          = 0xffffffff
border_width           = 2
motion_throttle_hz     = 85

terminal    = "foot"
menu        = "neumenu-run"

binds = {
    { mods="MOD4",       key="q",      action="spawn",          arg=terminal },
    { mods="MOD4",       key="d",      action="spawn",          arg=menu },
    { mods="MOD4",       key="Tab",    action="focus_next" },
    { mods="MOD4",       key="f",      action="fullscreen" },
    { mods="MOD4",       key="c",      action="kill" },
    { mods="MOD4",       key="Right",  action="move_x",         arg=75 },
    { mods="MOD4",       key="Left",   action="move_x",         arg=-75 },
    { mods="MOD4",       key="Down",   action="move_y",         arg=75 },
    { mods="MOD4",       key="Up",     action="move_y",         arg=-75 },
    { mods="MOD4",       key="space",  action="center" },
    { mods="MOD4|SHFT",  key="Right",  action="resize_width",   arg=25 },
    { mods="MOD4|SHFT",  key="Left",   action="resize_width",   arg=-25 },
    { mods="MOD4|SHFT",  key="Down",   action="resize_height",  arg=25 },
    { mods="MOD4|SHFT",  key="Up",     action="resize_height",  arg=-25 },
    { mods="MOD4",       key="h",      action="snap_left" },
    { mods="MOD4",       key="l",      action="snap_right" },
    { mods="MOD4",       key="1",      action="workspace_goto",   arg=1 },
    { mods="MOD4",       key="2",      action="workspace_goto",   arg=2 },
    { mods="MOD4",       key="3",      action="workspace_goto",   arg=3 },
    { mods="MOD4",       key="4",      action="workspace_goto",   arg=4 },
    { mods="MOD4",       key="5",      action="workspace_goto",   arg=5 },
    { mods="MOD4",       key="6",      action="workspace_goto",   arg=6 },
    { mods="MOD4",       key="7",      action="workspace_goto",   arg=7 },
    { mods="MOD4",       key="8",      action="workspace_goto",   arg=8 },
    { mods="MOD4",       key="9",      action="workspace_goto",   arg=9 },
    { mods="MOD4|SHFT",  key="1",      action="workspace_moveto", arg=1 },
    { mods="MOD4|SHFT",  key="2",      action="workspace_moveto", arg=2 },
    { mods="MOD4|SHFT",  key="3",      action="workspace_moveto", arg=3 },
    { mods="MOD4|SHFT",  key="4",      action="workspace_moveto", arg=4 },
    { mods="MOD4|SHFT",  key="5",      action="workspace_moveto", arg=5 },
    { mods="MOD4|SHFT",  key="6",      action="workspace_moveto", arg=6 },
    { mods="MOD4|SHFT",  key="7",      action="workspace_moveto", arg=7 },
    { mods="MOD4|SHFT",  key="8",      action="workspace_moveto", arg=8 },
    { mods="MOD4|SHFT",  key="9",      action="workspace_moveto", arg=9 },
}
