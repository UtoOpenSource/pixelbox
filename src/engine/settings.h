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

#include <string>

/*
 * All theese config values MUST be created as a globals!
 * You can also create your one values implementations, as long
 * as they are all created as globals!
 *
 * BTW, there is one exeption : a usage of the Register class.
 * You can pass pointer to a new Value() or something, if you need...
 */

namespace conf {

struct Value {

	Value();
	virtual ~Value() = 0;

	virtual bool   deserialize(const char* src, size_t maxlen) = 0;
	virtual size_t serialize  (char* dst, size_t maxlen) = 0;
	virtual void   showGUI(const char* name, float x, float y, float w, float h) = 0;
};

struct Flag : public Value {
	using valueType = bool;

	bool* value;

	Flag(bool& flag);
	virtual ~Flag();

	virtual bool   deserialize(const char* src, size_t maxlen);
	virtual size_t serialize  (char* dst, size_t maxlen);
	virtual void   showGUI(const char* name, float x, float y, float w, float h);
};

struct Integer : public Value {
	using valueType = int;

	int* value; int min, max;

	Integer(int& value, int min, int max);
	virtual ~Integer();

	virtual bool   deserialize(const char* src, size_t maxlen);
	virtual size_t serialize  (char* dst, size_t maxlen);
	virtual void   showGUI(const char* name, float x, float y, float w, float h);
};

struct Unsigned : public Value {
	using valueType = unsigned int;

	unsigned int* value; unsigned int min, max;

	Unsigned(unsigned int& value, unsigned int min, unsigned int max);
	virtual ~Unsigned();

	virtual bool   deserialize(const char* src, size_t maxlen);
	virtual size_t serialize  (char* dst, size_t maxlen);
	virtual void   showGUI(const char* name, float x, float y, float w, float h);
};

// a quick way to add getters/setters(hooks) for your values!
template <class T>
struct HookedValue : public T {
	using valueType = typename T::valueType;

	void (*setter) (T& value);
	valueType (*getter) (T& value);

	template <typename ...Args>
	HookedValue(void (*set)(T&), valueType (*get)(T&), Args&&... args) : T(std::forward<Args>(args)...) {
		setter = set;
		getter = get;
	}
	
	virtual bool   deserialize(const char* src, size_t maxlen) {
		bool res = T::deserialize(src, maxlen);
		if (res && setter) setter(*this); 
		return res;
	}
	virtual size_t serialize  (char* dst, size_t maxlen) {
		if (getter) *T::value = getter(*this); 
		size_t res = T::serialize(dst, maxlen);
		return res;
	}

	// gui callback may change value as it wish
	virtual void   showGUI(const char* name, float x, float y, float w, float h) {
		if (getter) *T::value = getter(*this); 
		valueType old = *T::value;
		T::showGUI(name, x, y, w, h);
		if (old != *T::value && setter) {
			setter(*this);
		}
	}
};

struct Parameter {
	Parameter* next;
	const char* id;
	Value* value; // Config Value
};

class Register {
	Value* ptr = nullptr;
	public:
	Register(const char* id, Value& v);
	Register(const char* id, Value* v); // pass new Object!!!
	Register(const char* id, bool&);
	Register(const char* id, int&, int, int);
	Register(const char* id, unsigned int&, unsigned int, unsigned int);
	~Register();
};

void Add(const char* id, Value& value);
Parameter* GetList();

void Reload();
void Save();

void Destroy(); // free list of the nodes

};
