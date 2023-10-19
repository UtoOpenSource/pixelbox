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
#include <limits.h>

// because raylib sucks here, we need to do streamable decompression
//#define SINFL_IMPLEMENTATION
//#include "sinfl.h"

class AbstractIO {
	public:
	AbstractIO();
	virtual file_pos read(void *dst, file_pos sz) = 0;
	virtual file_pos write(const void *src, file_pos sz) = 0;
	virtual file_pos tell() = 0;
	virtual file_pos seek(file_pos pos, int whence) = 0;
	virtual int      iseof() = 0;
	virtual int    	 flush() = 0;
	virtual int      geterror() = 0;
	virtual void     clearerror() = 0;
	virtual void*    handle() = 0;
	virtual int      is_FILE() = 0;
	virtual ~AbstractIO();
};

struct VFILE {
 AbstractIO* ctx;
 bool readbuff; // is buffer caches reading?
 char buff[IO_BUFF_SIZE]; // IO Cache
 unsigned long pos; // positon in buffer
 unsigned long len; // amount of readed data
};

AbstractIO::~AbstractIO() {}
AbstractIO::AbstractIO() {}

struct IOException {
	int errcode;
};

extern "C" {

extern const struct archive_node {
	const char* name;
	const long unsigned int length;
	const unsigned char* value;
} __main_arcive[];



/*
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

	vflog("can't find file %s in vfs!", fn);
	return NULL;
}

// Virtual File System :D
class VFSIO : public AbstractIO {
	const struct archive_node* node;
	size_t pos = 0; // position in compressed buffer
	size_t friendly_pos = 0; // count of readed bytes
	bool eof = 0;
	bool err = 0;
	public:
	VFSIO(const char* filename) {
		for (const struct archive_node* n = __main_arcive; n->name; n++) {
			if (strcmp(filename, n->name) == 0) {
				node = n;
				pos = 0;
				vflog("errr length %s", filename);
				if (n->length > INT_MAX) {
					throw "VFS file is too big!";
				}
				return;
			}
		}
		throw IOException{-1};
	}

	virtual file_pos read(void *dst, file_pos sz) {
		int count  = 0; // position at the end of last fully readed block
		int realres = 0; // count of inflated data from fully readed blocks

		if (pos >= node->length) {
			eof = 1; // hehe
			return 0;
		}

		int a = sinflate(
			dst, sz, (node->value + pos),
			(int)(node->length - pos), &count, &realres
		);

		if (a < 0 || realres <= 0) { // bad data or block is too big
			vflog("%s in file %s", realres <= 0 ?
				"bad data" : "other errors", node->name);
			err = 1;
			return -1;
		}

		assert(realres > 0);
		pos += count; // fully readed blocks :D
		if (pos >= node->length) {
			eof = 1; // hehe
		}
		friendly_pos += realres;
		vflog("readed %i real, %i real decompressed, %i unreal", count, realres, a);
		return realres; // return size of uncompressed data
										// altrough rest is in the buffer, you must not
										// touch it!
	}

	virtual file_pos write(const void *src, file_pos sz) {
		return -1; // impossible
	}

	virtual file_pos tell() {
		return friendly_pos; // uwu
	}

	// LIMITATION : implementing full seeking is too complex,
	// so here we are nly with two kinds
	virtual file_pos seek(file_pos p, int whence) {
		eof = 0;

		if (whence == SEEK_CUR) return -1; // not implemented
		else if (whence == SEEK_END && p != 0) return -1; // too
		else if (whence != SEEK_SET && p != 0) return -1; // too
		
		if (whence == SEEK_SET) { // ez
			pos = 0;
			friendly_pos = 0;
		} else { // hard 0_0
			// plz, try full reseek
			err = 0;
			pos = 0;
			friendly_pos = 0;
#define IO_VFS_SEEK_BUFF 1024
			char* buff = new char[node->length + 32];
			file_pos i = read(buff, node->length);
			while (i > 0) {
				i = read(buff, node->length + 16);
			}

			delete[] buff;

			if (i != 0) { // ERROR
				err = 1;
				vflog("Can't SEEK_END in file %s, err %i", node->name, (int)i);
				return -1; // :(
			}
		}

		return friendly_pos;
	};
	virtual int      iseof() {
		return eof;
	};
	virtual int    	 flush() {
		return -1;
	}
	virtual int      geterror() {
		return err ? -1 : 0;
	}
	virtual void     clearerror() {
		err = 0;
	};
	virtual void*    handle() {
		return (void*)node;
	};
	virtual ~VFSIO() {}
	virtual int      is_FILE() {return 0;}
}; */

class STDIO : public AbstractIO {
	FILE* f; bool no_close = false;
	public:
	STDIO(const char* filename, const char* mode) {
		f = fopen(filename, mode);
		if (f) return;
		throw IOException{-1};
	}

	STDIO(FILE* sf, bool noclose) {
		f = sf;
		no_close = noclose;
	}

	virtual file_pos read(void *dst, file_pos sz) {
		return fread(dst, 1, sz, f);
	}

	virtual file_pos write(const void *src, file_pos sz) {
		return fwrite(src, 1, sz, f);
	};

	virtual file_pos tell() {
		return ftell(f);
	};
	virtual file_pos seek(file_pos pos, int whence) {
		return fseek(f, pos, whence);
	};
	virtual int      iseof() {
		return feof(f);
	};
	virtual int    	 flush() {
		return fflush(f);
	}
	virtual int      geterror() {
		return ferror(f);
	};
	virtual void     clearerror() {
		return clearerr(f);
	}
	virtual void*    handle() {
		return f;
	}
	virtual ~STDIO() {if (!no_close) fclose(f);}

	virtual int      is_FILE() {return 1;}
};

static VFILE* newvfile() {
	VFILE* f = new VFILE;
	if (!f) throw "OOM";
	f->ctx = nullptr;
	f->len = 0;
	f->pos = 0;
	f->readbuff = 0;
	return f;
}

VFILE* vfwrap(FILE* ff, int noclose) {
	VFILE* f = newvfile();
	f->ctx = new STDIO(ff, noclose);
	return f;
}

FILE* vfraw(VFILE* f) {
	if (f && f->ctx && f->ctx->is_FILE())
		return (FILE*)f->ctx->handle();
	return nullptr;
}

VFILE* vfopen(const char* path, int mode) {
	VFILE* f = newvfile();

	if (mode == MODE_READ) { // find in vfs first
		/*try {
			// try vfs first
			f->ctx = new VFSIO(path);
		} catch (IOException& e) {
			vflog("No file %s in VFS! Trying system FS...", path);
			*/
			try {
				// try real fs now
				f->ctx = new STDIO(path, "rb");
			} catch (IOException& e) {
				goto error;
			}
		//}
		return f;
	} else if (mode == MODE_WRITE || mode == MODE_READWRITE) {
		try {
			f->ctx = new STDIO(path, mode == MODE_WRITE? "wb" : "rwb");
		} catch (IOException& e) {
			goto error;
		}
		return f;
	}

	error:
	TraceLog(LOG_ERROR, "Can't open file %s!", path);
	delete f;
	return NULL;
}


void   vfclose(VFILE* strm) {
	if (!strm) return;
	if (strm->ctx) {
		delete strm->ctx;
	}
	delete strm;
}

#define CHECK(X) if (!strm || !strm->ctx) return X;
int    vfeof (VFILE* strm) {
	CHECK(-1);
	return strm->ctx->iseof();
}
int    vfflush (VFILE* strm) {
	CHECK(-1);
	return strm->ctx->flush();
}

int vferror(VFILE* strm) {
	CHECK(-1);
	return strm->ctx->geterror();
}

void vfclearerror(VFILE* strm) {
	CHECK();
	strm->ctx->clearerror();
}

// flush tmp buffer
static int flush_buff(VFILE* f) {
	f->pos = 0; f->len = 0;
	if (!f->readbuff) {
		if (f->len == 0) return true; // no sence to try write!
		file_pos res = 0;
		res = f->ctx->write(f->buff, f->len);
		if (res <= 0) {
			vflog("Error! Can't write file back! %li", res);
			return (int)res;
		}
	} else {
		file_pos res = 0;
		res = f->ctx->read(f->buff, IO_BUFF_SIZE);
		if (res <= 0) {
			vflog("Error! Can't read file in buffer! %li", res);
			return (int)res;
		}
		f->len = res; // hehe
	}
	return 1;
}

// check buffer is for the same purposes as we want to use it
static int check_buff(VFILE* f, bool read) {
	if (f->readbuff == read) return true;
	else {
		int t = flush_buff(f);
		if (t <= 0) return t; // error :(
		f->readbuff = read;
		return 1;
	}
}

#include <assert.h>

file_pos vfread(void *dst, file_pos sz, VFILE* strm) {
	CHECK(-1);
	file_pos old = sz;

	char* out = (char*) dst;
	int t = check_buff(strm, 1);
	assert(strm->readbuff == 1);

	if (t <= 0) {
		vflog("Can't read! %i", t);
	}
	if (t < 0) return -2; // error
	if (t == 0 && strm->ctx->iseof()) return 0; // EOF
	else if (t == 0) return -2; // suspicious
	
	while (sz > 0) {
		if (strm->pos >= strm->len) {
			if (flush_buff(strm) <= 0) return old - sz; // error :8
			assert(strm->len > 0);
		}
		*(out++) = strm->buff[strm->pos++];
		sz--;
	}
	return old; // all in :D
}

file_pos vfwrite(const void *src, file_pos sz, VFILE* strm) {
	CHECK(-1);
	file_pos old = sz;

	const char* from = (char*)src;
	int t = check_buff(strm, 1);
	if (t < 0) return -2; // error
	if (t == 0 && strm->ctx->iseof()) return 0; // EOF
	else if (t == 0) return -2; // suspicius
	
	while (sz > 0) {
		if (strm->pos >= strm->len) {
			if (flush_buff(strm) <= 0) return old - sz; // error :8
			assert(strm->len > 0);
		}
		strm->buff[strm->pos++] = *(from++);
		sz--;
	}
	return old; // all in :D
}

file_pos vftell (VFILE* strm) {
	CHECK(-1);
	return strm->ctx->tell();
}

file_pos vfseek (VFILE* strm, file_pos pos, int mode) {
	CHECK(-1);
	return strm->ctx->seek(pos, mode);
}

int vfgetc(VFILE* strm) {
	CHECK(-1);
	int res = 0; char c = 0;
	res = strm->ctx->read(&c, 1);
	if (res > 0) res = c;
	else res = EOF;
	return res;
}

int vfungetc(VFILE* strm, int c) {
	CHECK(-1);
	if (strm->pos && strm->len && strm->readbuff) {
		strm->pos--;
		return 0;
	}
	return -1;
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
