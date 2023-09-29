// see pixelbox.h for copyright notice and license.
#include "hashmap.h"
#include "pbint.h"
#include <stdlib.h>

static inline struct chunk** next(struct chunk* c, bool g) {
	return g ? &(c->next) : &(c->next2);
}

struct chunk* findChunk(struct chunkmap* m, int16_t x, int16_t y) {
	union packpos pos;
	pos.axis[0] = x;
	pos.axis[1] = y;

	uint32_t hash = MAPHASH(pos.pack);
	struct chunk* c = m->data[hash];
	while (c) {
		if (c->pos.pack == pos.pack) return c; // FOUND
		c = *next(c, m->g);
	}
	return c;
}

void insertChunk(struct chunkmap* m, struct chunk* c) {
	if (!c) return;
	uint32_t hash = MAPHASH(c->pos.pack);
	struct chunk* n = m->data[hash];
	m->data[hash] = c;
	*next(c, m->g) = n; 
}

struct chunk* removeChunk(struct chunkmap* m, struct chunk* c) {
	uint32_t hash = MAPHASH(c->pos.pack);
	struct chunk *f = m->data[hash], *old = NULL;
	while (f != c) {
		old = f;
		f = *next(f, m->g);
	}

	if (f == NULL) {
		perror("OH NO! CAN'T REMOVE!");
		return NULL;
	}; 

	struct chunk* nn = *next(f, m->g);
	if (old) {
		*next(old, m->g) = nn;
		return nn;
	}
	m->data[hash] = nn;
	return nn;
} // returns next chunk if avail.

// functions below are always working with the GLOBAL map, so we are using next field directly here :p

// magic. We must remove and just put anything to save&free queue!
void collectAnything (void) {

	for (int i = 0; i < MAPLEN; i++) {
		World.update.data[i] = NULL; // yeah...
	}

	for (int i = 0; i < MAPLEN; i++) {
		struct chunk* c = World.map.data[i];
		while (c) {
			struct chunk* f = c;
			c = c->next;
			addSaveQueue(f); // will be freed IN!
		}
		World.map.data[i] = NULL; // optimisation for removal
	}
}

#include <assert.h>

int collectGarbage (void) {
	int limit = 0;

	// collect general
	for (int i = 0; i < MAPLEN; i++) {
		struct chunk *c = World.map.data[i], *old = NULL;
		while (c) {
			if (c->usagefactor >= 0) c->usagefactor--;
			if (c->usagefactor < 0) { // REMOVE AND COLLECT
				struct chunk* f = c;
				if (old) old->next = c->next; // remove
				else World.map.data[i] = c->next; // remove
				c = c->next;
				// old stays the same
				addSaveQueue(f); // will be freed IN (since it was removed!)!
				limit++;
			} else {
				old = c;
				c = c->next;
			}
		}
		// OK
	}

	limit = 0;
	// collect load

	for (int i = 0; i < MAPLEN; i++) {
		assert(!World.load.g);
		struct chunk *c = World.load.data[i], *old = NULL;
		while (c) {
			if (c->usagefactor >= 0) c->usagefactor--;
			if (c->usagefactor < 0) { // REMOVE AND COLLECT
				struct chunk* f = c;
				// next2, since load is minor hashmap!
				if (old) old->next2 = c->next2; // remove
				else World.load.data[i] = c->next2; // remove
				c = c->next2;
				// old stays the same
				freeChunk(f); // remove it NOW!
				limit++;
			} else {
				old = c;
				c = c->next2;
			}
		}
		// OK
	}

	return 0;
}


