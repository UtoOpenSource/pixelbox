/* Clock Source is used everywhere in pixelbox, even in server side,
 * but we CAN'T use GetTime() function from raylib for that :(
 *
 * (because server may be runned from CLI, where we can't provide
 * Window Creation and Raylib initialisation)
 *
 * That's why this clocksource is implemented there, using the most
 * reliable way for OS we are running on as possible...
 * At least i thinked so, until i found std::chrono LMAO :D
 * 
 * Of course, std::chrono sucks on some platforms, but it good enough
 * on linux and mingw, so yea...
 *
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
#include <chrono>

extern class ClockSource {
	double oldtime = 0; 
	float  frame_time = 0;
	void init();

	public:
	double time();
	float  delta();
	float  tick();
	ClockSource();
	~ClockSource(); 

} __clocksource;

class ClockSource __clocksource;
	
void ClockSource::init() {

}

double ClockSource::time() {
		using namespace std::chrono;
		using Clock = std::chrono::high_resolution_clock;
    auto now = Clock::now();
    auto duration = now.time_since_epoch();
		const std::chrono::duration<double, std::nano> seconds{duration};
		return seconds / 1s;
}	

float  ClockSource::delta() { // frame time
		return frame_time;
}

float  ClockSource::tick() {  // start new frame
		double t = time();
		frame_time = t - oldtime;
		oldtime = t;
		return frame_time;
}

ClockSource::ClockSource() {
		init();
		frame_time = 0;
		oldtime = time();
	}

ClockSource::~ClockSource() {

	}
