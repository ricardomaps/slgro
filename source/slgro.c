#include <asm-generic/errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

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

#define WHEN_PRESSED(state) \
	do { if ((state) != WL_KEYBOARD_KEY_STATE_PRESSED) return; } while (0)

#define BIND_EVENT(time, value, state) \
	(void)(time); (void)(value); \
	WHEN_PRESSED(state);

#define BIND_ACTION(data, time, value, state) \
	BIND_EVENT(time, value, state); \
	if (!wm.sel_client) return; \
	union arg* a = (data)

/* read geometry from client window; returns true if successful */
static bool get_geometry(struct client *c, struct swc_rectangle *geom)
{
	if (!c || !c->win)
		return false;

	return swc_window_get_geometry(c->win, geom);
}

/* adjust geometry fields and write back */
static void adjust_geom(struct client *c, int dx, int dy, int dw, int dh)
{
	struct swc_rectangle geom;
	if (!get_geometry(c, &geom)) return;

	geom.x += dx;
	geom.y += dy;

	int new_w = (int)geom.width + dw;
	int new_h = (int)geom.height + dh;
	geom.width = (new_w < 1) ? 1 : (uint32_t)new_w;
	geom.height = (new_h < 1) ? 1 : (uint32_t)new_h;

	swc_window_set_geometry(c->win, &geom);
}

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

static struct client* get_focus_candidate(struct screen* s) {
	struct client* c = first_client(s);
	if (!c && s) c = first_client(NULL);
	return c;
}

static void focus(struct client* c)
{
	if (wm.sel_client && wm.sel_client != c) {
		swc_window_set_border(wm.sel_client->win, cfg.border_col_normal, cfg.border_width, 0, 0);
	}
	if (c) {
		swc_window_set_border(c->win, cfg.border_col_active, cfg.border_width, 0, 0);
	}

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
		wm.sel_screen = wl_list_empty(&wm.screens)
			? NULL
			: wl_container_of(wm.screens.next, wm.sel_screen, link);
	}

	free(s);
}

static void on_win_destroy(void* data)
{
	struct client* c = data;
	if (!c) return;

	if (wm.grab.active && wm.grab.c == c)
		wm.grab.active = false;

	if (wm.sel_client == c)
		wm.sel_client = NULL;

	wl_list_remove(&c->link);
	free(c);

	focus(get_focus_candidate(wm.sel_screen));
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


/* focus next/prev helpers */
static void focus_walk_direction(bool forward)
{
	if (wl_list_empty(&wm.clients)) return;

	if (!is_ws_client(wm.sel_client, wm.sel_screen)) {
		focus(get_focus_candidate(wm.sel_screen));
		return;
	}

	struct wl_list *start =
		forward ? wm.sel_client->link.next
				: wm.sel_client->link.prev;

	struct wl_list *it = start;
	struct client *c = NULL;

	do {
		if (it == &wm.clients)
			it = forward ? wm.clients.next : wm.clients.prev;

		if (it == &wm.clients) break;

		c = wl_container_of(it, c, link);
		if (is_ws_client(c, wm.sel_screen)) {
			focus(c);
			return;
		}

		it = forward ? it->next : it->prev;
	} while (it != start);

	focus(get_focus_candidate(wm.sel_screen));
}

void focus_next(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data; BIND_EVENT(time, value, state);
	focus_walk_direction(true);
}

void focus_prev(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data; BIND_EVENT(time, value, state);
	focus_walk_direction(false);
}

void kill_sel(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data; BIND_EVENT(time, value, state);

	if (wm.sel_client)
		swc_window_close(wm.sel_client->win);
}

void fullscreen(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	(void)data; BIND_EVENT(time, value, state);
	if (!wm.sel_client) return;

	struct client *c = wm.sel_client;
	struct swc_rectangle geom;

	if (c->fullscreen) {
		c->fullscreen = false;
		swc_window_set_stacked(c->win);

		if (c->w > 0 && c->h > 0) {
			geom = (struct swc_rectangle){ c->x, c->y, c->w, c->h };
			swc_window_set_geometry(c->win, &geom);
		}
		return;
	}

	if (!c->scr || !c->scr->scr) return;

	if (get_geometry(c, &geom)) {
		c->x = geom.x;
		c->y = geom.y;
		c->w = geom.width;
		c->h = geom.height;
	}

	c->fullscreen = true;
	swc_window_set_fullscreen(c->win, c->scr->scr);
}

void kb_move_x(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	adjust_geom(wm.sel_client, a->i, 0, 0, 0);
}

void kb_move_y(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	adjust_geom(wm.sel_client, 0, a->i, 0, 0);
}

void kb_resize_width(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	adjust_geom(wm.sel_client, 0, 0, a->i, 0);
}

void kb_resize_height(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	adjust_geom(wm.sel_client, 0, 0, 0, a->i);
}

static void apply_grid_geometry(struct client *c, float x_frac, float w_frac) {
	if (!c || !c->scr || !c->scr->scr) return;

	struct swc_rectangle geom;
	struct swc_rectangle *ug = &c->scr->scr->usable_geometry;

	geom.x = ug->x + (ug->width * x_frac);
	geom.y = ug->y;
	geom.width = ug->width * w_frac;
	geom.height = ug->height;

	swc_window_set_geometry(c->win, &geom);
}

void center_window(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	if (!wm.sel_client->scr || !wm.sel_client->scr->scr) return;

	struct swc_rectangle geom;
	if (!get_geometry(wm.sel_client, &geom)) return;

	struct swc_rectangle *ug = &wm.sel_client->scr->scr->usable_geometry;
	geom.x = ug->x + (ug->width - geom.width) / 2;
	geom.y = ug->y + (ug->height - geom.height) / 2;

	swc_window_set_geometry(wm.sel_client->win, &geom);
}

void snap_left_half(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	apply_grid_geometry(wm.sel_client, 0.0f, 0.5f);
}

void snap_right_half(void* data, uint32_t time, uint32_t value, uint32_t state) {
	BIND_ACTION(data, time, value, state);
	apply_grid_geometry(wm.sel_client, 0.5f, 0.5f);
}

void new_screen(struct swc_screen* scr)
{
	struct screen* s = calloc(1, sizeof(*s));
	if (!s) die(EXIT_FAILURE, "new screen calloc failed");

	s->scr = scr;
	wl_list_insert(&wm.screens, &s->link);

	if (!wm.sel_screen) wm.sel_screen = s;

	swc_screen_set_handler(scr, &screen_handler, s);
	_log(stderr, "new_screen=%p\n", (void*)scr);
}

void new_window(struct swc_window* win)
{
	struct client* c = calloc(1, sizeof(*c));
	if (!c) die(EXIT_FAILURE, "calloc client failed");

	win->motion_throttle_ms = 1000 / cfg.motion_throttle_hz;
	win->min_width = 1;
	win->min_height = 1;

	c->win = win;
	c->scr = wm.sel_screen;
	c->floating = true;
	c->ws = wm.ws;

	wl_list_insert(&wm.clients, &c->link);
	swc_window_set_handler(win, &window_handler, c);
	swc_window_set_stacked(win);

	int32_t cx = 0, cy = 0;
	if (swc_cursor_position(&cx, &cy))
		swc_window_set_position(win, cx / 256, cy / 256);

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
	(void)data; BIND_EVENT(time, value, state);
	wl_display_terminate(wm.dpy);
}

void spawn(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	BIND_EVENT(time, value, state);

	pid_t pid = fork();
	if (pid == 0) {
		setsid();
		if (fork() == 0) {
			union arg* a = data;
			char* const* cmd = (char* const*)a->v;

			execvp(cmd[0], cmd);
			_exit(127);
		}
		_exit(0);
	} else if (pid > 0) {
		waitpid(pid, NULL, 0);
	}
}

void workspace_goto(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	union arg* a = data;
	BIND_EVENT(time, value, state);

	if (a->ui < 1 || a->ui > 9 || a->ui == wm.ws) return;

	wm.ws = a->ui;
	sync_window_visibility();

	focus(get_focus_candidate(wm.sel_screen));
}

void workspace_moveto(void* data, uint32_t time, uint32_t value, uint32_t state)
{
	BIND_ACTION(data, time, value, state);
	if (a->ui < 1 || a->ui > 9 || wm.sel_client->ws == a->ui) return;

	struct client* c = wm.sel_client;
	c->ws = a->ui;

	if (c->ws == wm.ws)
		swc_window_show(c->win);
	else
		swc_window_hide(c->win);

	focus(get_focus_candidate(wm.sel_screen));
}

int main(void)
{
	setup();
	wl_display_run(wm.dpy);
	swc_finalize();
	wl_display_destroy(wm.dpy);
	return EXIT_SUCCESS;
}