#include <asm-generic/errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <swc.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include "config.h"
#include "include/types.h"
#include "include/util.h"
#include "include/slgro.h"

static void focus(struct client* c);
static struct client* first_client(struct screen* s);
static bool is_ws_client(const struct client* c, const struct screen* s);
static void on_screen_destroy(void* data);
static void on_win_destroy(void* data);
static void on_win_entered(void* data);
static void setup(void);
static void setup_binds(void);
static void sync_window_visibility(void);

struct wm wm;
const struct swc_manager manager = {
	.new_screen = new_screen, .new_window = new_window, .new_device = new_device,
};
struct swc_window_handler window_handler = {
	.destroy = on_win_destroy, .entered = on_win_entered,
};
struct swc_screen_handler screen_handler = {
	.destroy = on_screen_destroy,
};

static void focus(struct client* c)
{
	if (wm.sel_client)
		swc_window_set_border(
			wm.sel_client->win,
			cfg.border_col_normal, cfg.border_width,
			0, 0
		);

	if (c)
		swc_window_set_border(
			c->win,
			cfg.border_col_active, cfg.border_width,
			0, 0
		);

	swc_window_focus(c ? c->win : NULL);
	wm.sel_client = c;
}

static struct client* first_client(struct screen* s)
{
	struct client* c;

	wl_list_for_each(c, &wm.clients, link) {
		if (is_ws_client(c, s))
			return c;
	}

	return NULL;
}

static bool is_ws_client(const struct client* c, const struct screen* s)
{
	return c && c->ws == wm.ws && (!s || c->scr == s);
}

static void on_screen_destroy(void* data)
{
	struct screen* s = data;

	if (!s)
		return;

	wl_list_remove(&s->link);

	if (wm.sel_screen == s) {
		if (wl_list_empty(&wm.screens))
			wm.sel_screen = NULL;
		else
			wm.sel_screen = wl_container_of(wm.screens.next, wm.sel_screen, link);
	}

	free(s);
}

static void on_win_destroy(void* data)
{
	struct client* c = data;
	struct client* next;

	if (!c)
		return;

	if (wm.grab.active && wm.grab.c == c) {
		wm.grab.active = false;
		wm.grab.c = NULL;
	}

	if (wm.sel_client == c) {
		wm.sel_client = NULL;
	}

	wl_list_remove(&c->link);
	free(c);

	next = first_client(wm.sel_screen);
	if (!next)
		next = first_client(NULL);
	focus(next);
}

static void on_win_entered(void* data)
{
	if (wm.grab.active)
		return;

	struct client* c = data;
	if (!is_ws_client(c, NULL))
		return;

	focus(c);
}

static void setup(void)
{
	/* display */
	wm.dpy = wl_display_create();
	if (!wm.dpy)
		die(EXIT_FAILURE, "wl_display_create failed");

	/* variables */
	wl_list_init(&wm.screens);
	wl_list_init(&wm.clients);
	wm.sel_client = NULL;
	wm.sel_screen = NULL;
	wm.grab.active = false;
	wm.grab.resize = false;
	wm.grab.c = NULL;
	wm.ws = 1;

	/* event loop */
	wm.ev_loop = wl_display_get_event_loop(wm.dpy);
	if (!swc_initialize(wm.dpy, wm.ev_loop, &manager))
		die(EXIT_FAILURE, "swc_initialize failed\n");

	setup_binds();

	/* display socket */
	const char* sock;
	sock = wl_display_add_socket_auto(wm.dpy);
	if (!sock)
		die(EXIT_FAILURE, "wl_display_add_socket_auto failed\n");
	setenv("WAYLAND_DISPLAY", sock, 1);
	_log(stderr, "WAYLAND_DISPLAY=%s\n", sock);

	/* signals */
	signal(SIGINT,  sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT, sig_handler);
}

static void setup_binds(void)
{
	for (size_t i = 0; i < LENGTH(binds); i++) {
		const struct bind* b = &binds[i];
		swc_add_binding(b->type, b->mods, b->ksym, b->fn, (void*)&b->arg);
	}
}

static void sync_window_visibility(void)
{
	struct client* c;

	wl_list_for_each(c, &wm.clients, link) {
		if (c->ws == wm.ws)
			swc_window_show(c->win);
		else
			swc_window_hide(c->win);
	}
}

void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	struct client* c = NULL;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (wl_list_empty(&wm.clients))
		return;

	if (!wm.sel_client || !is_ws_client(wm.sel_client, wm.sel_screen)) {
		c = first_client(wm.sel_screen);
		if (!c)
			c = first_client(NULL);
		focus(c);
		return;
	}

	struct wl_list* start = wm.sel_client->link.next;
	struct wl_list* it = start;

	do {
		if (it == &wm.clients)
			it = wm.clients.next;
		if (it == &wm.clients)
			break;

		c = wl_container_of(it, c, link);
		if (is_ws_client(c, wm.sel_screen)) {
			focus(c);
			return;
		}
		it = it->next;
	} while (it != start);

	c = first_client(wm.sel_screen);
	if (!c)
		c = first_client(NULL);
	focus(c);
}

void focus_prev(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	struct client* c = NULL;

	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (wl_list_empty(&wm.clients))
		return;

	if (!wm.sel_client || !is_ws_client(wm.sel_client, wm.sel_screen)) {
		c = first_client(wm.sel_screen);
		if (!c)
			c = first_client(NULL);
		focus(c);
		return;
	}

	struct wl_list* start = wm.sel_client->link.prev;
	struct wl_list* it = start;

	do {
		if (it == &wm.clients)
			it = wm.clients.prev;
		if (it == &wm.clients)
			break;

		c = wl_container_of(it, c, link);
		if (is_ws_client(c, wm.sel_screen)) {
			focus(c);
			return;
		}
		it = it->prev;
	} while (it != start);

	c = first_client(wm.sel_screen);
	if (!c)
		c = first_client(NULL);
	focus(c);
}

void kill_sel(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	swc_window_close(wm.sel_client->win);
}

void fullscreen(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	struct swc_rectangle geom;

	(void)data;
	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (wm.sel_client->fullscreen) {
		wm.sel_client->fullscreen = false;
		swc_window_set_stacked(wm.sel_client->win);

		if (wm.sel_client->w > 0 && wm.sel_client->h > 0) {
			geom.x = wm.sel_client->x;
			geom.y = wm.sel_client->y;
			geom.width = wm.sel_client->w;
			geom.height = wm.sel_client->h;
			swc_window_set_geometry(wm.sel_client->win, &geom);
		}
		return;
	}

	if (!wm.sel_client->scr || !wm.sel_client->scr->scr)
		return;

	if (swc_window_get_geometry(wm.sel_client->win, &geom)) {
		wm.sel_client->x = geom.x;
		wm.sel_client->y = geom.y;
		wm.sel_client->w = geom.width;
		wm.sel_client->h = geom.height;
	}

	wm.sel_client->fullscreen = true;
	swc_window_set_fullscreen(wm.sel_client->win, wm.sel_client->scr->scr);
}

/* slgro - im sure i could have made this better but atleast this works rn so ig that's what matters,, ill work on simplifying this another time,.,.*/
void kb_move_x(void* data, uint32_t time, uint32_t value, uint32_t state) {

	(void)time;
	(void)value;

	union arg* a = data;
	struct swc_rectangle geom;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (swc_window_get_geometry(wm.sel_client->win, &geom)) {
		geom.x += a->i;
		swc_window_set_geometry(wm.sel_client->win, &geom);
	}

}

void kb_move_y(void* data, uint32_t time, uint32_t value, uint32_t state) {

	(void)time;
	(void)value;
	
	union arg* a = data;
	struct swc_rectangle geom;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (swc_window_get_geometry(wm.sel_client->win, &geom)) {
		geom.y += a->i;
		swc_window_set_geometry(wm.sel_client->win, &geom);
	}

}

/* slgro - same thing here, ill figure out how to simplify this into one function another time, just focusing on making this work to begin with for now*/
void kb_resize_width(void* data, uint32_t time, uint32_t value, uint32_t state) {

	(void)time;
	(void)value;

	union arg* a = data;
	struct swc_rectangle geom;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (swc_window_get_geometry(wm.sel_client->win, &geom)) {
		geom.width += a->i;
		swc_window_set_geometry(wm.sel_client->win, &geom);
	}

}

void kb_resize_height(void* data, uint32_t time, uint32_t value, uint32_t state) {

	(void)time;
	(void)value;

	union arg* a = data;
	struct swc_rectangle geom;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (swc_window_get_geometry(wm.sel_client->win, &geom)) {
		geom.height += a->i;
		swc_window_set_geometry(wm.sel_client->win, &geom);
	}

}

/* slgro - idk if this is THAT useful but its kinda useful for me so that's what matters i suppose,,*/
void center_window(void* data, uint32_t time, uint32_t value, uint32_t state) {

	(void)data;
	(void)value;
	(void)state;
	(void)time;

	struct swc_rectangle geom;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (swc_window_get_geometry(wm.sel_client->win, &geom)) {
		geom.x = (wm.sel_client->scr->scr->usable_geometry.width / 2) - (geom.width / 2);
		geom.y = (wm.sel_client->scr->scr->usable_geometry.height / 2) - (geom.height / 2);
		swc_window_set_geometry(wm.sel_client->win, &geom);
	}

}

/* slgro - very experimental snapping, will prob refactor this at some point */ 
void snap_left_half(void* data, uint32_t time, uint32_t value, uint32_t state) {

(void)data;
(void)value;
(void)state;
(void)time;

struct swc_rectangle geom;

if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
return;

if (!wm.sel_client)
return;

	geom.x = wm.sel_client->scr->scr->usable_geometry.x;
	geom.y = wm.sel_client->scr->scr->usable_geometry.y;
	geom.width = wm.sel_client->scr->scr->usable_geometry.width / 2;
	geom.height = wm.sel_client->scr->scr->usable_geometry.height;
	swc_window_set_geometry(wm.sel_client->win, &geom);

}

void snap_right_half(void* data, uint32_t time, uint32_t value, uint32_t state) {

(void)data;
(void)value;
(void)state;
(void)time;

struct swc_rectangle geom;

if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
return;

if (!wm.sel_client)
return;

geom.x = wm.sel_client->scr->scr->usable_geometry.width / 2;
geom.y = wm.sel_client->scr->scr->usable_geometry.y;
geom.width = wm.sel_client->scr->scr->usable_geometry.width / 2;
geom.height = wm.sel_client->scr->scr->usable_geometry.height;
swc_window_set_geometry(wm.sel_client->win, &geom);

}

void new_screen(struct swc_screen* scr)
{
	struct screen* s;

	s = malloc(sizeof(*s));
	if (!s)
		die(EXIT_FAILURE, "new screen calloc failed");

	s->scr = scr;

	s->x = 0;
	s->y = 0;
	s->w = 0;
	s->h = 0;

	wl_list_insert(&wm.screens, &s->link);

	if (!wm.sel_screen)
		wm.sel_screen = s;

	swc_screen_set_handler(scr, &screen_handler, s);

	_log(stderr, "new_screen=%p\n", (void*)scr);
}

void new_window(struct swc_window* win)
{
	struct client* c;

	c = malloc(sizeof(*c));
	if (!c)
		die(EXIT_FAILURE, "malloc client failed");

	win->motion_throttle_ms = 1000 / cfg.motion_throttle_hz;
	win->min_width = 1;
	win->min_height = 1;
	win->max_width = 0;
	win->max_height = 0;

	c->win = win;
	c->scr = wm.sel_screen;
	c->mapped = 0;
	c->floating = true;
	c->fullscreen = 0;
	c->ws = wm.ws;
	c->x = 0;
	c->y = 0;
	c->w = 0;
	c->h = 0;

	wl_list_insert(&wm.clients, &c->link);
	swc_window_set_handler(win, &window_handler, c);
	swc_window_set_stacked(win);
	{
		/* swc reports cursor coordinates in wl_fixed_t (24.8) units */
		int32_t cx_fixed = 0;
		int32_t cy_fixed = 0;

		if (swc_cursor_position(&cx_fixed, &cy_fixed))
			swc_window_set_position(win, cx_fixed / 256, cy_fixed / 256);
	}
	swc_window_show(win);
	focus(c);

	_log(stderr, "new_window=%p\n", (void*)win);
}

void new_device(struct libinput_device* dev)
{
	(void)dev;
}

void quit(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data;
	(void)time;
	(void)value;
	(void)state;

	wl_display_terminate(wm.dpy);
}

void spawn(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	union arg* a = data;
	char* const* cmd = (char* const*)a->v;

	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (fork() == 0) {
		execvp(cmd[0], cmd);
		_exit(127);
	}
}

void workspace_goto(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	union arg* a = data;
	struct client* c;

	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (a->ui < 1 || a->ui > 9 || a->ui == wm.ws)
		return;

	wm.ws = a->ui;
	sync_window_visibility();

	c = first_client(wm.sel_screen);
	if (!c)
		c = first_client(NULL);
	focus(c);
}

void workspace_moveto(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	union arg* a = data;
	struct client* c;
	struct client* next;

	(void)time;
	(void)value;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (!wm.sel_client)
		return;

	if (a->ui < 1 || a->ui > 9)
		return;

	c = wm.sel_client;
	if (c->ws == a->ui)
		return;

	c->ws = a->ui;
	if (c->ws == wm.ws)
		swc_window_show(c->win);
	else
		swc_window_hide(c->win);

	next = first_client(wm.sel_screen);
	if (!next)
		next = first_client(NULL);
	focus(next);
}

int main(void)
{
	setup();
	wl_display_run(wm.dpy);
	swc_finalize();
	wl_display_destroy(wm.dpy);
	return EXIT_SUCCESS;
}
