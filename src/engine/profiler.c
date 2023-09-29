/* 
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

#include "profiler.h"
#include "libs/c89threads.h"

const char* prof_entries_names[] = {
	"game_tick",
	"inif/free",
	"draw",
	"draw_world",
	"draw_fin",
	"update",
	"physics",
	"g_collection",
	"saveload",
	"worldgen",
	"disk IO",
	"error"
};

static c89mtx_t prof_threads_mutex;

#include <raylib.h>
double prof_clock() {
	return GetTime();
}

struct prof_item {
	double time;
	int   entry;
};

struct prof_thread {
	struct prof_item stack[255];
	struct prof_stats data[PROF_ENTRIES_COUNT];
	c89thrd_t id;
	int stackpos;
	c89mtx_t mutex;
} prof_threads[PROF_THREADS_MAX] = {0};

// summary history
static struct prof_stats prof_history[PROF_THREADS_MAX] [PROF_ENTRIES_COUNT][PROF_HISTORY_LEN];
static int prof_history_pos = 0;

#include <assert.h>
#include <stdlib.h>

static void onfree() {
	c89mtx_destroy(&prof_threads_mutex);
	for (int i = 0; i < PROF_THREADS_MAX; i++) {
		c89mtx_lock(&prof_threads[i].mutex);
		c89mtx_unlock(&prof_threads[i].mutex);
		c89mtx_destroy(&prof_threads[i].mutex);
	}
}

static int prof_initialized = 0;

// RACE CONDITION : First call of the profiler MUST happen
// in main thread only. Afterwards, it's safe to use :p
static void init() {
	if (prof_initialized) return;
	c89mtx_init(&prof_threads_mutex, 0);
	for (int i = 0; i < PROF_THREADS_MAX; i++) {
		c89mtx_init(&prof_threads[i].mutex, 0);
		prof_threads[i].stackpos = -1;
	}
	atexit(onfree);
	prof_initialized = 1;
}

void prof_register_thread() {
	init();

	c89thrd_t id = c89thrd_current();
	// LOCKABLE
	c89mtx_lock(&prof_threads_mutex);
	for (int i = 0; i < PROF_THREADS_MAX; i++) {
		if (prof_threads[i].id == 0) {
			prof_threads[i].id = id;
			c89mtx_unlock(&prof_threads_mutex);
			return;
		}
	}
	c89mtx_unlock(&prof_threads_mutex);
	assert(0 && "Too many threads are already registered in profiler!");
}

void prof_unregister_thread() {
	c89thrd_t id = c89thrd_current();

	// non atomic read/write...
	// but it should not cause problems...
	for (int i = 0; i < PROF_THREADS_MAX; i++) {
		if (prof_threads[i].id == id) {
			prof_threads[i].id = 0;
			return;
		}
	}
	assert(0 && "Thread is not registered!");
}

static struct prof_thread* getctx() {
	c89thrd_t id = c89thrd_current();

	// NON-ATOMIC READ! Should not make problems, surely :clueless:
	// without this assumption, we will loss A LOT in perfomance
	for (int i = 0; i < PROF_THREADS_MAX; i++) {
		if (c89thrd_equal(prof_threads[i].id, id)) {
			return prof_threads + i;
		}
	}

	assert(0 && "This thread was not registered!");
}

typedef struct prof_thread* ctx_t;

#define GETCTX() ctx_t x = getctx();
#define LOCK()   //c89mtx_lock(&x->mutex)
#define UNLOCK() //c89mtx_unlock(&x->mutex)

static inline void push(ctx_t x, int i, double time) {
	x->stackpos++;
	assert(x->stackpos < 255 && "profiler stack overflow");
	x->stack[x->stackpos].time = time; 
	x->stack[x->stackpos].entry = i;
}

static inline struct prof_item pop(ctx_t x) {
	struct prof_item i = x->stack[x->stackpos];
	assert(x->stackpos >= 0 && "profiler stack underflow");
	x->stackpos --;
	return i;
}

static inline struct prof_item* get(ctx_t x) {
	struct prof_item *i = x->stack + x->stackpos ;
	assert(x->stackpos >= 0 && "profiler stack underflow");
	return i;
}

static inline bool have(ctx_t x) {
	return x->stackpos >= 0;
}

#include <string.h>

/* How does it work?
 * Every time we push new profiler scope(entry), we set
 * summary and own time for previous as difference of 
 * the current time and previously saved in previous entry.
 * + we set time for previous entry as current. Any write interaction
 * with any entry requires to do that.
 *
 * When we pop scope(entry), we will do similar stuff with popped
 * element, BUT also we will ADD time difference for the current
 * (after pop) element, using it's own old time. 
 * NOT OWN TIME! only summary!
 *
 * also we count number of "calls" - pushes of the profiler scopes.
 * 
 * When prof_step() is called, all current processed data is pushed into
 * the profiler history and cleaned up after that.
 *
 * All this technique is minded by me, on paper, in one day. It
 * may work very ugly, but it works, and i don't need more :p
 */

void prof_begin(int entry) {
	assert(entry >= 0 && entry < PROF_ENTRIES_COUNT);
	GETCTX();
	LOCK();
	double time = prof_clock();

	// set owntime and sumtime for previous entry
	if (have(x)) {
		struct prof_item* prev  = get(x);
		struct prof_stats* stat = x->data + prev->entry;

		stat->owntime += time - prev->time;
		stat->sumtime += time - prev->time;
		prev->time     = time;
	}

	push(x, entry, time);
	x->data[entry].ncalls++;
	UNLOCK();
}

void prof_end() {
	GETCTX();
	LOCK();
	struct prof_item item = pop(x);
	struct prof_stats* stat = x->data + item.entry;
	double time = prof_clock();

	stat->owntime += time - item.time;
	stat->sumtime += time - item.time;

	if (have(x)) { // add to summary time of the current item 
		struct prof_item* prev = get(x);
		stat = x->data + prev->entry;

		stat->sumtime += time - prev->time;
		prev->time     = time;
	}
	UNLOCK();
}

void prof_step() {
	GETCTX();
	LOCK();
	
	int thread = x - prof_threads;

	for (int i = 0; i < PROF_ENTRIES_COUNT; i++) {
		prof_history[thread][i][prof_history_pos] = x->data[i];
		x->data[i] = (struct prof_stats){0};
	}	
	UNLOCK();
	prof_history_pos++;
	if (prof_history_pos >= PROF_HISTORY_LEN) {
		prof_history_pos = 0;
	}
} 

/*
 * the only way to get data back, but it's good enough :P
 */
struct prof_stats* prof_summary(int entry, int thread) {
	assert(entry >= 0 && entry < PROF_ENTRIES_COUNT);
	assert(thread >= 0 && thread < PROF_THREADS_MAX);
	return prof_history[thread][entry];
}

#include "raygui.h"


static Color prof_color(int entry) {
	entry = entry + 1;

	int r = !!(entry & 1);
	int g = !!(entry & (1 << 1));
	int b = !!(entry & (1 << 2));
	int h = !!(entry & (1 << 3));
	h = h * 55;
	r = r * 200 + h;
	g = g * 200 + h;
	b = b * 200 + h;
	return (Color) {r, g, b, 255};
}

void drawProfiler(Rectangle rec) {
	Rectangle item = (Rectangle){
		rec.x, rec.y,
		100, 10
	};

	static int active_thrd = 0;
	active_thrd = GuiComboBox(item, "main;2;3;4;5", active_thrd); 

	item.y += item.height;

	item.height = (rec.y + rec.height) - item.y - 20;
	item.width  = rec.width - 5; 
	DrawRectangleRec(item, (Color){0, 0, 0, 255});

	for (int i = 0; i < PROF_ENTRIES_COUNT; i++) {
		Rectangle o = (Rectangle){
			item.x, item.y + i*8,
			6, 6
		};
		DrawRectangleRec(o, prof_color(i));
		o.x += 8;
		o.width = rec.width - 5 - 8;
		DrawText(prof_entries_names[i], o.x, o.y, 4, prof_color(i));	
	}
	
	item.width  = rec.width - 75; 
	item.x      = rec.x + 70; 
	item.height = (rec.y + rec.height) - item.y - 10;

	float max_value = 1.0/75.0;
	/*for (int i = 0; i < PROF_HISTORY_LEN; i++) {
		float v = prof_summary(PROF_GAMETICK)[i].sumtime;
		if (v > max_value) max_value = v;
	}*/

	float plot_scale = item.height / max_value;

	for (int i = 0; i < PROF_ENTRIES_COUNT; i++) {
		struct prof_stats* stat = prof_summary(i, active_thrd);
		Color color = prof_color(i);
		for (int ix = 0; ix < PROF_HISTORY_LEN-1; ix++) {
			float x = item.x + ix * item.width / PROF_HISTORY_LEN;
			float x2 = item.x + (ix+1) * item.width / PROF_HISTORY_LEN;
			float y  = stat[ix].owntime*plot_scale;
			y = item.y + item.height - y;
			float y2 = stat[ix+1].owntime*plot_scale;
			y2 = item.y + item.height - y2;
			DrawLine(x, y, x2, y2, color);
		}
	}
}

