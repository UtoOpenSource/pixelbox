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
#include <base.hpp>
#include <stdexcept>
#include "sqlite3.h"
#include <string>

#include <string.h>

namespace pb {

class	StatementError : public std::runtime_error {
	public:
	using re = std::runtime_error;
	using re::re;
};

class Statement : public Default {
	sqlite3_stmt* ptr;
	public:
	Statement(sqlite3* db, const char* sql) {
		int err = sqlite3_prepare_v3(db, sql, -1, 0, &ptr, (const char**)0);
		if (err != SQLITE_OK) {
			throw StatementError(sqlite3_errmsg(db));	
		}
	}
	Statement(Statement&& src) {
		ptr = src.ptr;
		src.ptr = nullptr;
	}
	~Statement() {
		if (ptr) sqlite3_finalize(ptr);
	}
	public:
	bool iterator();
	void reset() {
		if (ptr) sqlite3_reset(ptr);
	}
	void clear() {
		if (ptr) sqlite3_clear_bindings(ptr);
	}
	int  bind_index(const char* name) {
		return sqlite3_bind_parameter_index(ptr, name);
	}

	bool bind(int index, int64_t value) {
		return sqlite3_bind_int64(ptr, index, value);
	}

	bool bind(int index, uint64_t value) {
		return sqlite3_bind_int64(ptr, index, (int64_t)value);
	}

	bool bind(int index, double value) {
		return sqlite3_bind_double(ptr, index, value);
	}

	bool bind(int index, const char* value) {
		return sqlite3_bind_text(ptr, index, value, -1, SQLITE_STATIC);
	}

	bool bind(int index, const char* value, size_t size) {
		return sqlite3_bind_blob(ptr, index,
			value, size, SQLITE_TRANSIENT);
	}

	bool unbind(int index) {
		return sqlite3_bind_null(ptr, index);
	}

	// get values
	
	int  column_count() {
		return sqlite3_data_count(ptr); 
	}

	template<typename T>
	inline T column(int index) {
		static_assert("Type is not supported");
	}

	size_t column_bytes(int index) {
		return sqlite3_column_bytes(ptr, index);
	}

	bool column_blob(int index, void* rdst, size_t max);
	bool column_text(int index, char* dst, size_t max);

};

	template<>
	inline int64_t Statement::column<int64_t>(int index) {
		return sqlite3_column_int64(ptr, index);	
	}

	template<>
	inline uint64_t Statement::column<uint64_t>(int index) {
		return (uint64_t)sqlite3_column_int64(ptr, index);	
	}

	template<>
	inline double Statement::column<double>(int index) {
		return sqlite3_column_double(ptr, index);	
	}

	template<>
	inline std::string Statement::column<std::string>(int index) {
		return std::string(
			(const char*)sqlite3_column_text(ptr, index),
			column_bytes(index)
		);
	}


class Database : public Default {
	sqlite3* db = nullptr;
	public:	
	Database() = default;
	Database(const Database&) = delete;
	Database(Database&& src) {
		db = src.db;
		src.db = nullptr;
	}
	public:
	void close() {
		if (db) sqlite3_close_v2(db);
		db = nullptr;
	}
	bool open(const char* path, bool readonly = false) {
		int flags = readonly ? SQLITE_OPEN_READONLY :
			SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
		return sqlite3_open_v2(path, &db, flags, NULL) == SQLITE_OK;	
	}
	~Database() {
		close();
	}
	Database(const char* path, bool ro = false) {
		open(path, ro);
	}
	public:
	void exec(const char* str) {
		char* msg;
		int ok = sqlite3_exec(db, str, NULL, NULL, &msg);
		if (ok != SQLITE_OK) 
			throw StatementError(msg);
	}
	Statement query(const char* sql) {
		return Statement(db, sql);
	}
};

};

