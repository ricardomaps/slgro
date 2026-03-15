#ifndef SLGRO_H
#define SLGRO_H

#include <stdint.h>

#include "types.h"

extern void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void kill_sel(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void fullscreen(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void kb_move_x(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void kb_move_y(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void kb_resize_width(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void kb_resize_height(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void snap_left_half(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void snap_right_half(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void center_window(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void new_screen(struct swc_screen* scr);
extern void new_window(struct swc_window* win);
extern void new_device(struct libinput_device* dev);
extern void quit(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void spawn(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void workspace_goto(void* data, uint32_t time, uint32_t value, uint32_t state);
extern void workspace_moveto(void* data, uint32_t time, uint32_t value, uint32_t state);
extern struct wm wm;

#endif /* SLGRO_H */
