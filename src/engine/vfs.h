#pragma once
#include <stdio.h>
#include <stddef.h>

typedef struct {
 void* ptr;
 size_t size_if_buff;
 bool   real_file;
 size_t pos;
} VFILE;

#define MODE_READ 1
#define MODE_WRITE 2
#define MODE_READWRITE 3

#ifdef __cplusplus
extern "C" {
#endif

VFILE* vfopen(const char* path, int mode);
void   vfclose(VFILE* strm);
int    vfread(void *dst, size_t sz, VFILE* strm);
int    vfwrite(const void *src, size_t sz, VFILE* strm);
size_t vftell (VFILE* strm);
int    vfseek (VFILE* strm, long int pos, int mode);
int    vfeof (VFILE* strm);
int    vfflush (VFILE* strm);

int    vflog(const void *src, ...);

int vfgetc(VFILE* strm);
int vfungetc(VFILE* strm, int c);
int vferror(VFILE* strm);

void vfclearerror(VFILE* strm);

VFILE* vfwrap(FILE* ff, int noclose);
FILE* vfraw(VFILE* f);

#ifdef __cplusplus
};
#endif
