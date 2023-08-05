// see pixelbox.h for copyright notice and license.
#include "implix.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "c89threads.h"

#define NODE_LEN 1024
#define FREE_VALUE 32767
#define ITEM_MAGIC 0x0AFF
#define NODE_MAGIC 0xFA0A

struct alloc_item {
	uint16_t magic; // 0x0A0A0AFF
	uint16_t busy;
	struct chunk data;
};

struct alloc_node {
	struct alloc_node *next;
	uint16_t magic; // 0xFF0AFF00
	uint16_t count; // count of allocated chunks
	uint16_t empty; // index of empty slot
	struct alloc_item items[NODE_LEN];
};

static c89mtx_t alloc_mutex;
static struct alloc_node* node_list = NULL, *node_last = NULL;

#include <assert.h>

static void allocfree() {
	c89mtx_destroy(&alloc_mutex);
	struct alloc_node* f = NULL;
	while (node_list) {
		f = node_list;
		node_list = node_list->next;
		assert(f->magic == NODE_MAGIC && "heap corruption");
		if (f->count)
			fprintf(stderr, "ALLOC: leak detected! %i chunks are not freed!\n", f->count);
		else for (uint16_t i = 0; i < NODE_LEN; i++) {
			assert(f->items[i].busy == FREE_VALUE); // important too
		}
		free(f);
	}
	node_list = NULL;
	node_last = NULL;
	fprintf(stderr, "ALLOC: chunk allocator uninitialized!\n");
}

static struct alloc_node* newnode() {
	struct alloc_node* n = calloc(sizeof(struct alloc_node), 1);
	if (!n) {
		perror("NOMEM!");
		abort(); // TODO NOMEM
	}
	n->magic = NODE_MAGIC; // node magic number
	n->count = 0;
	for (uint16_t i = 0; i < NODE_LEN; i++) {
		n->items[i].magic = ITEM_MAGIC; // item magic number
		n->items[i].busy = FREE_VALUE;
	}
	if (node_last) node_last->next = n; // 'cause list may be empty
	node_last = n;
	return n;
}

static void allocinit() {
	c89mtx_init(&alloc_mutex, 0);
	node_list = newnode();
	atexit(allocfree);
	fprintf(stderr, "ALLOC: chunk allocator intialized\n");
}

static struct alloc_node* findCoolNode() { // with space available
	if (!node_list) allocinit(); // ok
	struct alloc_node* n = node_list;
	while (n && n->count == NODE_LEN-1) { // skip busy nodes
		n = n->next;
	}
	if (!n) n = newnode(); // no empty nodes? Alloc new one!
	assert(n->magic == NODE_MAGIC); // pedantic
	return n;
}

struct chunk* allocChunk(int16_t x, int16_t y) {
	c89mtx_lock(&alloc_mutex);
	struct alloc_node* n = findCoolNode();

	uint16_t i = 0;
	for (i = n->empty; i < NODE_LEN; i++) {
		if (n->items[i].busy == FREE_VALUE) { // cool
			break;
		}
	}
	n->empty = i+1; // update empty index; (overflow is ok)

	assert(i < NODE_LEN && "heap corruption! (invalid count)");
	n->count++; // one more item is busy now
	n->items[i].busy = i; // hehe
	assert(n->items[i].magic == ITEM_MAGIC && "bad magic"); // pedantic again

	c89mtx_unlock(&alloc_mutex);
	struct chunk* c = &(n->items[i].data); // well done	
	c->pos.axis[0] = x;
	c->pos.axis[1] = y;
	return c;
}

static struct alloc_item* dataToNode(void* p, struct alloc_node** dst) {
	const char* offset = &(((struct alloc_item*)0)->data);
	struct alloc_item* it = (struct alloc_item*)((char*)p - offset);

	assert(it->magic == ITEM_MAGIC && "heap corruption or region is not chunk");
	uint16_t i = it->busy;
	assert(i < NODE_LEN); // including FREE_VALUE
	struct alloc_item* fi = it - i; // first item
	
	const char* offset2 = &(((struct alloc_node*)0)->items);
	struct alloc_node* n = (struct alloc_node*)((char*)fi - offset2);
	assert(n->magic == NODE_MAGIC); // pedantic
	assert(n->items + i == it); // must be same
	assert((void*)(&it->data) == p); // may be removed?

	// return
	*dst = n; 
	return it;
}

#include <string.h>

void freeChunk(struct chunk* orig) {
	if (!orig) return;
	c89mtx_lock(&alloc_mutex);

	struct alloc_node* n = NULL;
	struct alloc_item* it = dataToNode((void*)orig, &n);
	assert(n && it);

	n->count--;
	if (n->empty > it->busy) n->empty = it->busy; // set new empty node index
	it->busy = FREE_VALUE; // yeah
	memset(&it->data, 0, sizeof(struct chunk)); // important

	c89mtx_unlock(&alloc_mutex); // done
}

uint8_t* getChunkData(struct chunk* c, const bool mode) {
	return c->atoms + (mode == World.wIndex ? 32*32 : 0);
}

#include <string.h>
#include <math.h>

void generateChunk(struct chunk* c) {
	uint8_t* data = getChunkData(c, MODE_READ);
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {
			int ax = (x + (int)c->pos.axis[0]*CHUNK_WIDTH);
			int ay = (y + (int)c->pos.axis[1]*CHUNK_WIDTH);
			float v;

			float c = noise2(ax/512.0, ay/512.0);
			v = c;
			v -= noise2(ax/1024.0, ay/1024.0);
			if ((v < 0.05 || v > 0.9)) {
				c *= 0.5;
				v = noise2(ax/124.0, ay/124.0) + c;
				v = v > 1.0 ? v : 1.0 - v;
				v += 0.02;
			} else v = 0; 
			data[x + y * CHUNK_WIDTH] = v*255;
			//data2[x + y * CHUNK_WIDTH] = randomNumber();
		}
	}
}

void generateChunkFlat(struct chunk* c) {
	uint8_t* data = getChunkData(c, MODE_READ);
	for (int x = 0; x < CHUNK_WIDTH; x++) {
		for (int y = 0; y < CHUNK_WIDTH; y++) {
			int ax = (x + (int)c->pos.axis[0]*CHUNK_WIDTH);
			int ay = (y + (int)c->pos.axis[1]*CHUNK_WIDTH);
			int v;
	
			v = ((ax & 1023) == 64) + ((ay & 1023) == 64);
			data[x + y * CHUNK_WIDTH] = v ? (v+1)<<2 : 0;
		}
	}
}

void addSaveQueue(struct chunk* c) {
	saveChunk(c);
	freeChunk(c);
}

void addLoadQueue(struct chunk* c) {
	if (loadChunk(c) <= 0) {
		World.mode ? generateChunkFlat(c) : generateChunk(c) ;
	}
	c->usagefactor = CHUNK_USAGE_VALUE;
	insertChunk(&World.Map, c); // OK
}

// SQLITE PART 2
#include <sqlite3.h>

int  loadChunk(struct chunk* c) {
	if (World.database) {
		bool loaded = false;
		sqlite3_stmt* stmt = create_statement(
				"SELECT value FROM WCHUNKS WHERE id = ?1;");
		if (!stmt) return -2;
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		while (statement_iterator(stmt) > 0) {
			const uint8_t* data = (uint8_t*)sqlite3_column_blob(stmt, 0);
			if (data) {
				memcpy(getChunkData(c, false), data, 32*32);
				loaded = true;
			}
		}
		sqlite3_finalize(stmt);
		return loaded;
	}
	return -1;
}

void saveChunk(struct chunk* c) {
	if (World.database) {
sqlite3_stmt* stmt = create_statement(
				"INSERT OR REPLACE INTO WCHUNKS VALUES(?1, ?2);");
		if (!stmt) return;
		sqlite3_bind_int64(stmt, 1, c->pos.pack);
		sqlite3_bind_blob(stmt, 2, getChunkData(c, MODE_READ), 32*32, SQLITE_STATIC);
		while (statement_iterator(stmt) > 0) {}
		sqlite3_finalize(stmt);
	}
}

