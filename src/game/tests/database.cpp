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
#include "doctest.h"
#include "stdio.h"

TEST_CASE("Database") {
	pb::Database db;
	CHECK(db.open(":memory:") == true);
	db.exec("CREATE TABLE test (name string primary key, value string);");

	const char* pairs[8] = {
		"amogus", "suspicious person",
		"abobus", "very strange person",
		"abulbus", "he likes swimming",
		"utobus",  "shit maker v2"
	};

	for (int i = 0; i < 8; i += 2) {
		auto q = db.query("insert or ignore into test values(?1, ?2);");
		q.bind(1, pairs[i+0]);
		q.bind(2, pairs[i+1]);
		while (q.iterator()) {}
	}

	printf("RESULT :\n");
	auto q = db.query("SELECT * from test;");
	while (q.iterator()) {
		int cnt = q.column_count();
		printf("\t");
		for (int i = 0; i < cnt; i++) {
			printf("%s |", q.column<std::string>(i).c_str());
		}
		printf("\n");
	}
}

