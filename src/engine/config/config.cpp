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

#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

namespace conf {

Value::Value() {}
Value::~Value() {};

Flag::Flag(bool& f) {value = &f;}
Flag::~Flag() {};

bool   Flag::deserialize(const char* src, size_t maxlen) {
	int tmp = 0;
	int stat = sscanf(src, "%1i", &tmp);
	if (stat <= 0) return false;
	*value = tmp;
	return true;
}

size_t Flag::serialize  (char* dst, size_t maxlen) {
	int i = snprintf(dst, maxlen, "%1i", int(*value));
	return i > 0 ? size_t(i) : 0;
}

Integer::Integer(int& i, int a, int b) {
	value = &i;
	min = a; max = b;
}

Integer::~Integer() {}

bool   Integer::deserialize(const char* src, size_t maxlen) {
	int tmp = 0;
	int stat = sscanf(src, "%i", &tmp);
	if (stat <= 0) return false;
	*value = tmp;
	if (*value < min) *value = min;
	else if (*value > max) *value = max;
	fprintf(stderr, "readed valur %i\n", tmp);
	return true;
}

size_t Integer::serialize  (char* dst, size_t maxlen) {
	int i = snprintf(dst, maxlen, "%i", int(*value));
	return i > 0 ? size_t(i) : 0;
}

Unsigned::Unsigned(unsigned int& i, unsigned int a, unsigned int b) {
	value = &i;
	min = a; max = b;
}

Unsigned::~Unsigned() {}

bool Unsigned::deserialize(const char* src, size_t maxlen) {
	unsigned int tmp = 0;
	int stat = sscanf(src, "%u", &tmp);
	if (stat <= 0) return false;
	*value = tmp;
	if (*value < min) *value = min;
	else if (*value > max) *value = max;
	return true;
}

size_t Unsigned::serialize  (char* dst, size_t maxlen) {
	int i = snprintf(dst, maxlen, "%u", *value);
	return i > 0 ? size_t(i) : 0;
}

// high level api

Register::Register(Manager& m, const char* id, Value& v) {
	m.add(id, v);
}

Register::Register(Manager& m, const char* id, Value* v) {
	m.add(id, *v);
}

Register::Register(Manager& m, const char* id, bool& v) {
	ptr = new Flag(v);
	m.add(id, *ptr);
}

Register::Register(Manager& m, const char* id, int& i, int a, int b) {
	ptr = new Integer(i, a, b);
	m.add(id, *ptr);
}

Register::Register(Manager& m, const char* id, unsigned int& i, unsigned int a, unsigned int b) {
	ptr = new Unsigned(i, a, b);
	m.add(id, *ptr);
}

Register::~Register() {
	if (ptr) delete ptr;
	ptr = nullptr;
}

// TODO : move theese implementations in separate file!

void   Flag::showGUI(const char* name, float x, float y, float w, float h) {
	
}

void   Unsigned::showGUI(const char* name, float x, float y, float w, float h) {

}

void   Integer::showGUI(const char* name, float x, float y, float w, float h) {

}

// keep all settings in one list
// TODO: change on hashmap somewhat later

#include <string.h>
#include <ctype.h>

Manager::Manager(const char* path, const char* fn) {
	if (!fn) fn = "";
	if (!path) path = "./";

	size_t pathlen = strlen(path);
	size_t filelen = strlen(fn);
	size_t len = pathlen + filelen + 1;
	this->filename = new char[len];
	::memcpy(filename, path, pathlen);
	::memcpy(filename + pathlen, fn, filelen);
	filename[len-1] = 0;
}

Manager::~Manager() {
	delete[] filename;
	clear();
}

void Manager::add(const char* id, Value& value) {
	Parameter* p = new Parameter();

	int len = int(strlen(id))+1;
	for (int i = 0; i < len; i++) {
		if (isspace(id[i]) || id[i] == '=') {
			throw "space or equal sign in config params are not allowed";
		}
	}	

	p->id = id;
	p->value = &value;
	p->next = nullptr;

	// insert
	if (last) {
		last->next = p;
	} else {
		list = p;
	}
	last = p;
}

Parameter* Manager::getList() {
	return list;
}

Parameter* Manager::find(const char* id) {
	Parameter* p = list;
	while (p && strcmp(p->id, id) != 0) {
		p = p->next;
	}
	return p;
}

static int getconfline(char* buff, int len, int* eqpos, FILE* f) {
	int pos = 0;
	int c = 0;

	int stage = 0;
	*eqpos = -1;

	while ((c = getc(f)) != EOF && c != '\n') {
		if (pos < len && stage >= 0) { // skip until newline
			if (isspace(c)) continue;
			if (c == '=' && stage == 0) { // special case
				*eqpos = pos;
				stage = 1;
			} else if (c == '#' && pos == 0 && stage == 0) { // comment
				stage = -1;
			} 
			buff[pos++] = c;
		}
	}

	buff[pos] = '\0'; // append trailing zero
	if (stage == 0) {
		return int(c != EOF) - 1; // -1 => EOF, 0 => bad line
	} else {
		return stage == 1; // else it's a comment!
	}
}

void Manager::reload() {
	const char* fn = filename ? filename : "config.ini";

	FILE* f = fopen(fn, "r");
	if (!f) {
		perror("Can't read config file!");
		return;
	}

	char buff[512];
	int  line = 0;
	int  eqpos = 0;
	int  stat = 0;

	while((stat = getconfline(buff, 511, &eqpos, f)) >= 0) {
		line++;
	
		if (!stat) { // if no length - comment or empty line
			if (strlen(buff) > 0)
				fprintf(stderr, "config file error : bad line %i!\n", line);
			continue;
		}

		buff[eqpos] = '\0';
		eqpos++;

		Parameter *p = find(buff);
		if (!p) {
			fprintf(stderr, "config file error : bad perameter %s at line %i!\n", buff, line);
			continue;
		}
		if (!p->value->deserialize(buff+eqpos, strlen(buff+eqpos)+1)) {
			fprintf(stderr, "config file error : can't load parameter %s at line %i!\n", p->id, line);
		};
	}

	fclose(f);
}

void Manager::save() {
	const char* fn = filename ? filename : "config.ini";

	FILE* f = fopen(fn, "w");
	if (!f) {
		perror("Can't save config file!");
		return;
	}

	char buff[512];
	int  len = 0;

	Parameter* p = list;
	while (p) {
		len = snprintf(buff, 511, "%s=", p->id);
		if (len < 0) continue; // btw

		int add = int(p->value->serialize(buff+len, size_t(511-len)));

		if (add <= 0) {
			fprintf(stderr, "config save error : can't save parameter %s!\n", p->id);
		}
		if (add > 511-len) add = 511-len;
		len += add;
		if (add < 0 || len > 511 || len < 0) {
			fclose(f);
			throw "something bad happened";
		}
		buff[len] = '\0';

		fprintf(f, "%s\n", buff);

		p = p->next;
	}

	fclose(f);
}

void Manager::clear() {
	while (list != nullptr) {
		Parameter* f = list;
		list = list->next;
		delete f;
	}
	list = nullptr;
	last = nullptr;
}

};
