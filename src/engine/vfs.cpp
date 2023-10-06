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


#include "vfs.h"
#include "raylib.h"
#include <string.h>

extern "C" {

extern const struct archive_node {
	const char* name;
	const long unsigned int length;
	const unsigned char* value;
} __main_arcive[];

static void* readvfile(const char* fn, size_t* ressize) {
	void* data = NULL;

	for (const struct archive_node* n = __main_arcive; n->name; n++) {
		if (strcmp(fn, n->name) == 0) {
			int sz = 0;
			data = DecompressData((unsigned char*)n->value, n->length, &sz);
			*ressize = sz;
			return data;
		}
	}

	return NULL;
}

VFILE* vfwrap(FILE* ff, int noclose) {
	if (!ff) return NULL;
	VFILE* f = new VFILE;
	f->pos = noclose ? 666 : 0;
	f->size_if_buff = 0; 
	f->ptr = ff;
	return f;
}

FILE* vfraw(VFILE* f) {
	return f->size_if_buff ? NULL : (FILE*)f->ptr;
}

VFILE* vfopen(const char* path, int mode) {
	VFILE* f = new VFILE;
	f->pos = 0;
	f->size_if_buff = 0; 

	FILE*  real = NULL;

	if (mode == MODE_READ) { // find in vfs first
		f->ptr = readvfile(path, &f->size_if_buff);
		if (f->ptr) return f; // OK!

		real = fopen(path, "rb");
		f->ptr = real;
		goto regular; // regular file
	
	} else if (mode == MODE_WRITE || mode == MODE_READWRITE) {
		real = fopen(path, mode == MODE_WRITE? "wb" : "rwb");
		regular:
		if (real) { // real file !
			f->ptr = real;
		}
	}

	TraceLog(LOG_ERROR, "Can't open file %s!", path);
	delete f;
	return NULL;
}

static bool isreal(VFILE* f) {
	if (!f || !f->ptr) throw "file is misused!";
	return f->size_if_buff;
}

void   vfclose(VFILE* strm) {
	if (isreal(strm)) {
		if (strm->pos != 666) fclose((FILE*)strm->ptr);
		// ignore and delete ourselves
	} else MemFree(strm->ptr);
	delete strm;
}

int   vfread(void *dst, size_t sz, VFILE* strm) {
	if (!isreal(strm)) {
		size_t maxsz = strm->size_if_buff - strm->pos;
		if (sz > maxsz) sz = maxsz;
		memcpy(dst, (char*)strm->ptr + strm->pos, sz);
		strm->pos += sz;
		return (int)sz;
	} else return fread(dst, 1, sz, (FILE*)strm->ptr);
}

int    vfwrite(const void *src, size_t sz, VFILE* strm) {
	if (isreal(strm)) {
		return fwrite(src, 1, sz, (FILE*)strm->ptr);
	} else return -1; // can't write vfs now...
}

size_t vftell (VFILE* strm) {
	if (isreal(strm)) {
		return ftell((FILE*)strm->ptr);
	} else return strm->pos; 
}

int vfgetc(VFILE* strm) {
	char res = 0;
	int  cnt = vfread(&res, 1, strm);
	if (!cnt) return EOF;
	return res;
}

int vfungetc(VFILE* strm, int c) {
	if (isreal(strm)) return ungetc(c, (FILE*)strm->ptr);
	if (strm->pos) strm->pos--;
	else return -1;
	return 1;
}

int    vfseek (VFILE* strm, ssize_t pos, int mode) {
	if (isreal(strm)) {
		return fseek((FILE*)strm->ptr, pos, mode);
	} else {
		ssize_t res = (ssize_t)strm->pos;
		if (mode == SEEK_SET) res = pos;
		else if (mode == SEEK_CUR) res += pos;
		else if (mode == SEEK_END) res = strm->size_if_buff - (pos + 1);
		// check
		if (res < 0) res = 0;
		if (res > strm->size_if_buff) res = strm->size_if_buff;
		strm->pos = (size_t)res;
		return 0;
	}
}

int vfeof (VFILE* strm) {
	if (isreal(strm)) {
		int c = getc((FILE*)strm->ptr);
		ungetc(c, (FILE*)strm->ptr); /* no-op when c == EOF */
		return c == EOF;
	} else return strm->pos >= strm->size_if_buff; 
}

void vfclearerror(VFILE* strm) {
	if (isreal(strm)) return clearerr((FILE*)strm->ptr);
}

int    vferror (VFILE* strm) {
	if (isreal(strm)) {
		return ferror((FILE*)strm->ptr);
	} else return strm->pos >= strm->size_if_buff ? EOF : 0; 
}

int    vfflush (VFILE* strm) {
	if (isreal(strm)) return fflush((FILE*)strm->ptr);
	return 0; // no flush for vfs!
}

// VFS LOG
#include <stdarg.h>
int    vflog(const void *src, ...) {
	char tmp[512];
	va_list args;
	va_start(args, src);
	vsnprintf(tmp, 512, (char*)src, args);
	va_end(args);

	TraceLog(LOG_ERROR, "%s", tmp);
	return 0;
}

};
