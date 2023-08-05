// see pixelbox.h for copyright notice and license.
#include "implix.h"
#include <stdlib.h>

struct chunk* findChunk(struct chunkmap* m, int16_t x, int16_t y) {
	union packpos pos;
	pos.axis[0] = x;
	pos.axis[1] = y;

	uint32_t hash = MAPHASH(pos.pack);
	struct chunk* c = m->data[hash];
	while (c) {
		if (c->pos.pack == pos.pack) return c; // FOUND
		c = c->next;
	}
	return c;
}

void insertChunk(struct chunkmap* m, struct chunk* c) {
	if (!c) return;
	uint32_t hash = MAPHASH(c->pos.pack);
	struct chunk* n = m->data[hash];
	m->data[hash] = c;
	c->next = n; 
}

struct chunk* removeChunk(struct chunkmap* m, struct chunk* c) {
	uint32_t hash = MAPHASH(c->pos.pack);
	struct chunk *f = m->data[hash], *old = NULL;
	while (f != c) {
		old = f;
		f = f->next;
	}

	if (f == NULL) {
		perror("OH NO! CAN'T REMOVE!");
		return NULL;
	}; 

	if (old) {
		old->next = f->next;
		return old->next;
	}
	m->data[hash] = f->next;
	return f->next;
} // returns next chunk if avail.

// magic. We may not remove, just put anything to save&free queue!
void collectAnything (void) {
	for (int i = 0; i < MAPLEN; i++) {
		struct chunk* c = World.Map.data[i];
		while (c) {
			struct chunk* f = c;
			c = c->next;
			addSaveQueue(f); // will be freed IN!
		}
		World.Map.data[i] = NULL;
	}
}

int collectGarbage (void) {
	int limit = 0;
	for (int i = 0; i < MAPLEN; i++) {
		struct chunk *c = World.Map.data[i], *old = NULL;
		while (c) {
			if (c->usagefactor >= 0) c->usagefactor--;
			if (c->usagefactor < 0 && limit < 5) { // REMOVE AND COLLECT
				struct chunk* f = c;
				if (old) old->next = c->next; // remove
				else World.Map.data[i] = c->next; // remove
				c = c->next;
				// old stays the same
				addSaveQueue(f); // will be freed IN!
				limit++;
			} else {
				old = c;
				c = c->next;
			}
		}
		// OK
	}
	return 0;
}


