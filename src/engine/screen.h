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

#pragma once
#include "stdbool.h"

struct screen {
	struct screen* back;
	void (*draw)(void);
	void (*update)(void);
	void (*create)(void);
	void (*destroy)(void);
	void (*onclose)(void);
};

extern struct screen ScrNull, ScrMainMenu, ScrLicense, ScrWorldList,
		ScrNewWorld, ScrGamePlay, ScrSettings;

void SetNextScreen(struct screen*);	 // and sets as current
void SetRootScreen(
		struct screen*);	// => sets as current. Cleanups stack
int SetPrevScreen(struct screen*);	// fallback screen in arg if no
																		// more screen in stack exist!

int UpdateScreenSystem(bool should_close);

