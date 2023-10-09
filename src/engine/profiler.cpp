/*
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>
 */

#include "profiler.h"
#include <thread>
#include <mutex>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// my lovely clocksource :D
namespace engine {
	double getTime();
};

namespace prof {

using Mutex = std::mutex;
using Thread = std::thread;
using ThreadID = std::thread::id;

static Mutex threads_mutex;

double prof_clock() {
	return ::engine::getTime();
}

struct prof_item {
	double time;
	int entry;
};

struct prof_thread {
	struct prof_item stack[255];
	struct prof_stats data[ENTRIES_COUNT];
	int stackpos = -1;

	const char* const* entries_names = nullptr;
	int entries_count = 0; // must not be > than ENTRIES_COUNT

	Mutex mutex;
};

static prof_thread *prof_threads_data[THREADS_MAX];
// some atomic assumptions...
static ThreadID    prof_threads_ids[THREADS_MAX]; 

// summary history
static struct prof_stats prof_history[THREADS_MAX]
																		 [ENTRIES_COUNT]
																		 [HISTORY_LEN];
static int prof_history_pos = 0;


static void onfree() {
	for (int i = 0; i < THREADS_MAX; i++) {
		prof_thread *t = prof_threads_data[i];
		if (t) {
			t->mutex.lock();
			t->mutex.unlock();
			delete t;
		}
		prof_threads_ids[i] = ThreadID();
		prof_threads_data[i] = nullptr;
	}
}

static int prof_initialized = 0;

// RACE CONDITION : First call of the profiler MUST happen
// in main thread only. Afterwards, it's safe to use :p
static void init() {
	if (prof_initialized) return;
	atexit(onfree);
	prof_initialized = 1;
}

void register_thread(const char* const* names) {
	init();

	assert(names != nullptr);
	ThreadID id = ::std::this_thread::get_id();

	// LOCKABLE
	// we are don't care about too much locking here, since
	// locks are happening ONLY during register/unregister!
	threads_mutex.lock();

	for (int i = 0; i < THREADS_MAX; i++) {
		if (prof_threads_ids[i] == id) {
			threads_mutex.unlock();
			assert("Attemt to register same thread twice!");
			return;
		}
	}

	// get number of entries!
	int count_of_entries = 0;
	for (; count_of_entries < ENTRIES_COUNT; count_of_entries++) {
		if (!names[count_of_entries]) break;
	}

	for (int i = 0; i < THREADS_MAX; i++) {
		if (prof_threads_data[i] == nullptr) {
			prof_thread* t = new prof_thread();
			prof_threads_ids[i] = id;
			t->stackpos = -1;
			t->entries_count = count_of_entries;
			t->entries_names = names;
			prof_threads_data[i] = t;
			threads_mutex.unlock();
			return;
		}
	}
	threads_mutex.unlock();
	assert(0 && "Too many threads are already registered in profiler!");
}

void unregister_thread() {
	ThreadID id = ::std::this_thread::get_id();

	threads_mutex.lock();
	for (int i = 0; i < THREADS_MAX; i++) {
		if (prof_threads_data[i] && prof_threads_ids[i] == id) {
			delete prof_threads_data[i];
			prof_threads_ids[i] = ThreadID();
			prof_threads_data[i] = nullptr;
			threads_mutex.unlock();
			return;
		}
	}
	threads_mutex.unlock();
	assert(0 && "Thread was not registered!");
}

static struct prof_thread* getctx(int *thread_index) {
	ThreadID id = ::std::this_thread::get_id();

	// NON-ATOMIC READ! Should not make problems, surely :clueless:
	// without this assumption, we will loss A LOT in perfomance
	for (int i = 0; i < THREADS_MAX; i++) {
		if (prof_threads_data[i] && prof_threads_ids[i] == id) {
			*thread_index = i;
			return prof_threads_data[i];
		}
	}
	*thread_index = -1;
	assert(0 && "This thread was not registered!");
}

typedef struct prof_thread* ctx_t;

#define GETCTX() int thread_index = 0; ctx_t x = getctx(&thread_index);
#define LOCK()		// c89mtx_lock(&x->mutex)
#define UNLOCK()	// c89mtx_unlock(&x->mutex)

static inline void push(ctx_t x, int i, double time) {
	x->stackpos++;
	assert(x->stackpos < 255 && "profiler stack overflow");
	x->stack[x->stackpos].time = time;
	x->stack[x->stackpos].entry = i;
}

static inline struct prof_item pop(ctx_t x) {
	struct prof_item i = x->stack[x->stackpos];
	assert(x->stackpos >= 0 && "profiler stack underflow");
	x->stackpos--;
	return i;
}

static inline struct prof_item* get(ctx_t x) {
	struct prof_item* i = x->stack + x->stackpos;
	assert(x->stackpos >= 0 && "profiler stack underflow");
	return i;
}

static inline bool have(ctx_t x) { return x->stackpos >= 0; }


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
 * When prof_step() is called, all current processed data is pushed
 * into the profiler history and cleaned up after that.
 *
 * All this technique is minded by me, on paper, in one day. It
 * may work very ugly, but it works, and i don't need more :p
 */

void begin(int entry) {
	assert(entry >= 0 && entry < ENTRIES_COUNT);
	GETCTX();
	if (entry >= x->entries_count)
		throw "bad entry index for this thread!";
	LOCK();
	double time = prof_clock();

	// set owntime and sumtime for previous entry
	if (have(x)) {
		struct prof_item* prev = get(x);
		struct prof_stats* stat = x->data + prev->entry;

		stat->owntime += time - prev->time;
		stat->sumtime += time - prev->time;
		prev->time = time;
	}

	push(x, entry, time);
	x->data[entry].ncalls++;
	UNLOCK();
}

void end() {
	GETCTX();
	LOCK();
	struct prof_item item = pop(x);
	struct prof_stats* stat = x->data + item.entry;
	double time = prof_clock();

	stat->owntime += time - item.time;
	stat->sumtime += time - item.time;

	if (have(x)) {	// add to summary time of the current item
		struct prof_item* prev = get(x);
		stat = x->data + prev->entry;

		stat->sumtime += time - prev->time;
		prev->time = time;
	}
	UNLOCK();
}

void step() {
	GETCTX();
	LOCK();

	int thread = thread_index;
	int count  = x->entries_count;
	assert(count >= 0 && count < ENTRIES_COUNT); // corrupt?

	for (int i = 0; i < count; i++) { // small optimisation
		prof_history[thread][i][prof_history_pos] = x->data[i];
		memset(x->data + i, 0, sizeof (struct prof_stats));
	}
	UNLOCK();
	prof_history_pos++;
	if (prof_history_pos >= HISTORY_LEN) {
		prof_history_pos = 0;
	}
}

/*
 * the only way to get data back, but it's good enough :P
 */
struct prof_stats* summary(int entry, int thread) {
	assert(entry >= 0 && entry < ENTRIES_COUNT);
	assert(thread >= 0 && thread < THREADS_MAX);
	return prof_history[thread][entry];
}

const char*        get_name(int entry, int thread) {
	if (thread < 0 ||
			thread >= THREADS_MAX ||
			entry < 0 || entry >= ENTRIES_COUNT
	) return nullptr;

	threads_mutex.lock();
	if (!prof_threads_data[thread] ||
			prof_threads_ids[thread] == ThreadID()) {
		threads_mutex.unlock();
		return nullptr;
	}

	ctx_t x = prof_threads_data[thread];
	int count  = x->entries_count;
	assert(count >= 0 && count < ENTRIES_COUNT);  // corrupt?

	const char* res = nullptr;

	if (entry < count && x) {
		res = x->entries_names[entry];
	}

	threads_mutex.unlock();
	return res;
}

};

#include "raylib.h"
#include "raygui.h"

static Color prof_color(int entry) {
	entry = entry + 1;

	int r = !!(entry & 1);
	int g = !!(entry & (1 << 1));
	int b = !!(entry & (1 << 2));
	int h = !!(entry & (1 << 3));
	h = h * 70;
	r = r * 150 + h;
	g = g * 150 + h;
	b = b * 150 + h;
	return Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
}

void drawProfiler(Rectangle rec) {
	using namespace prof;
	Rectangle item = (Rectangle){rec.x, rec.y, 100, 10};

	static int active_thrd = 0;
	active_thrd = GuiComboBox(item, "1;2;3;4;5", active_thrd);

	item.y += item.height;

	item.height = (rec.y + rec.height) - item.y - 20;
	item.width = rec.width - 5;
	DrawRectangleRec(item, (Color){0, 0, 0, 255});

	prof::ctx_t x = prof_threads_data[active_thrd];
	if (!x) return;
	int count = x->entries_count;
	const char* const* names = x->entries_names;

	for (int i = 0; i < x->entries_count; i++) {
		Rectangle o = (Rectangle){item.x, item.y + i * 8, 6, 6};
		DrawRectangleRec(o, prof_color(i));
		o.x += 8;
		o.width = rec.width - 5 - 8;
		DrawText(names[i], o.x, o.y, 4, prof_color(i));
	}

	item.width = rec.width - 75;
	item.x = rec.x + 70;
	item.height = (rec.y + rec.height) - item.y - 10;

	float max_value = 1.0 / 75.0;
	for (int i = 0; i < HISTORY_LEN; i++) {
		float v = summary(0, active_thrd)[i].sumtime;
		if (v > max_value) max_value = v;
	}

	float plot_scale = item.height / max_value;

	for (int i = 0; i < count; i++) {
		struct prof_stats* stat = summary(i, active_thrd);
		Color color = prof_color(i);
		for (int ix = 0; ix < HISTORY_LEN - 1; ix++) {
			float x = item.x + ix * item.width / HISTORY_LEN;
			float x2 = item.x + (ix + 1) * item.width / HISTORY_LEN;
			float y = stat[ix].owntime * plot_scale;
			y = item.y + item.height - y;
			float y2 = stat[ix + 1].owntime * plot_scale;
			y2 = item.y + item.height - y2;
			DrawLine(x, y, x2, y2, color);
		}
	}
}

