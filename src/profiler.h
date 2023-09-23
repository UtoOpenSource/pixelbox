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

#pragma once
#include <raylib.h>

#define PROF_HISTORY_LEN 255

/*
 * This is what differs this profiler from any another in the internet :
 * You have limited amount of the profiler entries, setted at the compile-time.
 *
 * Some sort of hashmap in profiler is not really bad as doing direct strcmp(),
 * but it still TOO SLOW, that profiler will affect results very much!
 *
 * But even with this simplification, please, don't forget, that profiling is
 * still NOT FREE, and using it just in any small function like std::vector::get() is pretty useless and worth it!
 *
 * Apply profiling only for significant algoritms, like garbage collection, 
 * physic update, game objects update, sorting of game objetcs for
 * proper Z order drawing, mesh building, terrain generation and etc.
 */
enum prof_entries {
	PROF_GAMETICK,
	PROF_INIT_FREE,
	PROF_DRAW,
	PROF_DRAWWORLD,
	PROF_FINDRAW,
	PROF_UPDATE,
	PROF_PHYSIC,
	PROF_GC,
	PROF_LOAD_SAVE,
	PROF_GENERATOR,
	PROF_DISK,
	PROF_ENTRIES_COUNT
};

extern const char* prof_entries_names[];

#define PROF_THREADS_MAX 5

/*
 * the SUMMARY time of every entry SHOULD be equal to sum of it's own execution
 * time AND the summary time of subentries OR the own time OF ALL SUBENTRIES 
 * AND ALL SUBENTRIES OF SUBENTRIES AND ETC. 
 *
 * Summary time basically means how long this entry was a deal, in any case.
 * Own (execution) time means only time, while only this entry itself was 
 * running, not any subentries.
 */
struct prof_stats {
	float owntime; // how long this entry was executed (subentries are EXCLUDED)
	float sumtime; // how long this entry and all CALLED subentries are executed
	int   ncalls;  // number of "calls" to this entry
};

// implementation of very presize and stable clocksource
// DON'T USE A DEFAULT clock() FUNCTION FROM LIBC! IT'S AWFUL!
// I am using the GetTime() function from raylib here.
double prof_clock(); 

/* 
 * Every thread, that going to use this profiler, must be properly
 * registered and unregistered!
 */
void prof_register_thread();
void prof_unregister_thread();

/*
 * Push-Pop-like profiling
 */
void prof_begin(int entry);
void prof_end();

/*
 * Get statictics about some entry.
 * Length of this array is specified in PROF_HISTORY_LEN macro.
 * I don't think that knowing current position of the prfiler
 * in this array is useful... THat's why i am not doing that :D
 * You can draw plot right from this, it looks okay.
 */
struct prof_stats* prof_summary(int entry, int thread);

/*
 * Call this at the end of the "Game tick", or just to finally write results
 * to the history.
 *
 * Remember about floating point presizion, don't make pauses between this
 * very long.
 *
 * THIS FUNCTION REQUIRES FOR ALL PUSHED ENTRIES TO BE POPPED OUT!
 * Aka, this function MAY be ONLY called after all profiling zones are done.
 */
void prof_step();
