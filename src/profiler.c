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
	"draw",
	"draw_fin",
	"update",
	"physics",
	"g_collection",
	"load/init",
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
