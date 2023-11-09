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

#include "database.hpp"

namespace pb {

	bool Statement::iterator() { // return true while has data
		int attemts = 0, res = 0;
		retry:
		res = sqlite3_step(ptr);
		if (res == SQLITE_BUSY) {
			attemts++;
			if (attemts < 100) goto retry;
		}

		bool reset = false;
		if (res == SQLITE_DONE)
			reset = true;
		else if (res == SQLITE_ROW)
			reset = false;
		else {
			sqlite3_reset(ptr);
			throw StatementError(sqlite3_errmsg(sqlite3_db_handle(ptr)));
		}

		if (reset) {
			sqlite3_reset(ptr);
		}
	
		return !reset;	
	}

	bool Statement::column_blob(int index, void* rdst, size_t max) {
		unsigned char* dst = (unsigned char*) rdst;
		const char* src = (const char*)sqlite3_column_blob(ptr, index);
		size_t slen = column_bytes(index);
		if (!src || !max) return false;
		size_t len = slen > max ? max : slen;
		memcpy(dst, src, len);
		return true;
	}

	bool Statement::column_text(int index, char* dst, size_t max) {
		const char* src = (const char*)sqlite3_column_text(ptr, index);
		size_t slen = column_bytes(index);
		if (!src || !max || !(max-1)) return false;
		max--;

		size_t len = slen > max ? max : slen;
		memcpy(dst, src, len);
		dst[len] = '\0'; // end of line
		return true;
	};

};
