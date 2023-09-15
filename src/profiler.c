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

double prof_clock() {
	return GetTime();
}

struct prof_item {
	double time;
	int   entry;
};

static struct prof_item prof_stack[255] = {0};
static int    prof_stack_pos = -1;

static struct prof_stats prof_history[PROF_ENTRIES_COUNT][PROF_HISTORY_LEN];
static int prof_history_pos = 0;

struct prof_stats prof_data[PROF_ENTRIES_COUNT] = {0};

#include <assert.h>

static inline void push(int i, double time) {
	prof_stack_pos++;
	assert(prof_stack_pos < 255 && "profiler stack overflow");
	prof_stack[prof_stack_pos].time = time; 
	prof_stack[prof_stack_pos].entry = i;
}

static inline struct prof_item pop() {
	struct prof_item i = prof_stack[prof_stack_pos];
	assert(prof_stack_pos >= 0 && "profiler stack underflow");
	prof_stack_pos--;
	return i;
}

static inline struct prof_item* get() {
	struct prof_item *i = prof_stack + prof_stack_pos;
	assert(prof_stack_pos >= 0 && "profiler stack underflow");
	return i;
}

static inline bool have() {
	return prof_stack_pos >= 0;
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
	double time = prof_clock();

	// set owntime and sumtime for previous entry
	if (have()) {
		struct prof_item* prev  = get();
		struct prof_stats* stat = prof_data + prev->entry;

		stat->owntime += time - prev->time;
		stat->sumtime += time - prev->time;
		prev->time     = time;
	}

	push(entry, time);
	prof_data[entry].ncalls++;
}

void prof_end() {
	struct prof_item item = pop();
	struct prof_stats* stat = prof_data + item.entry;
	double time = prof_clock();

	stat->owntime += time - item.time;
	stat->sumtime += time - item.time;

	if (have()) { // add to summary time of the current item 
		struct prof_item* prev  = get();
		stat = prof_data + prev->entry;

		stat->sumtime += time - prev->time;
		prev->time     = time;
	}
}

void prof_step() {
	assert(prof_stack_pos < 0);
	for (int i = 0; i < PROF_ENTRIES_COUNT; i++) {
		prof_history[i][prof_history_pos] = prof_data[i];
		prof_data[i] = (struct prof_stats){0};
	}
	prof_history_pos++;
	if (prof_history_pos >= PROF_HISTORY_LEN) {
		prof_history_pos = 0;
	}
} 

/*
 * the only way to get data back, but it's good enough :P
 */
struct prof_stats* prof_summary(int entry) {
	assert(entry >= 0 && entry < PROF_ENTRIES_COUNT);
	return prof_history[entry];
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
		rec.width-5, 10
	};

	item.height = (rec.y + rec.height) - item.y - 10;
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
		struct prof_stats* stat = prof_summary(i);
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

