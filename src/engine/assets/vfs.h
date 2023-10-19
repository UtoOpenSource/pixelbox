#pragma once
#include <stdio.h>
#include <stddef.h>

typedef long int file_pos;

#define IO_BUFF_SIZE 255

typedef struct VFILE VFILE;

#define MODE_READ 1
#define MODE_WRITE 2
#define MODE_READWRITE 3

#ifdef __cplusplus
extern "C" {
#endif

VFILE* vfopen(const char* path, int mode);

VFILE* vfwrap(FILE* ff, int noclose); // wrap existed file
FILE*  vfraw(VFILE* f);

void     vfclose(VFILE* strm);
file_pos vfread(void *dst, file_pos sz, VFILE* strm);
file_pos vfwrite(const void *src, file_pos sz, VFILE* strm);
file_pos vftell (VFILE* strm);
file_pos vfseek (VFILE* strm, file_pos pos, int mode);
int    vfeof (VFILE* strm);
int    vfflush (VFILE* strm);


int vfgetc(VFILE* strm);
int vfungetc(VFILE* strm, int c);
int vferror(VFILE* strm);

void vfclearerror(VFILE* strm);

int    vflog(const void *src, ...);

#ifdef __cplusplus
};
#endif
