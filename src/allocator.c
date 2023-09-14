/* 
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>
 */

#include "allocator.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "c89threads.h"

#define NODE_LEN   128
#define FREE_INDEX 65000
#define ITEM_MAGIC 0x0AFF
#define NODE_MAGIC 0xFA0A
#define ALLOCATOR_DEBUG 1

struct alloc_item {
	uint16_t magic; // 0x0AFF
	
	// index in the array or FREE_INDEX, if item is freed
	uint16_t index; 
	struct chunk data;
};

struct alloc_node {
	struct alloc_node *next;
	uint16_t magic; // 0xFA0A
	uint16_t count; // count of allocated chunks
	uint16_t empty; // index of empty slot
	struct alloc_item items[NODE_LEN];
};

static c89mtx_t alloc_mutex;
static struct alloc_node* node_list = NULL, *node_last = NULL;

#include <assert.h>

/* Why we need a custom allocator here?
 * Chunks are the most used objects, by design.
 * They are allocated/freed quite often, and they 
 * have fixed size. That's why allocation process
 * NEEDS to be optimized, and can be optimized.
 */
static void allocfree() {
	c89mtx_destroy(&alloc_mutex);
	struct alloc_node* f = NULL;

	while (node_list) {
		f = node_list;
		node_list = node_list->next;

#if ALLOCATOR_DEBUG
		assert(f->magic == NODE_MAGIC && "heap corruption");
		if (f->count)
			fprintf(stderr, "ALLOC: leak detected! %i chunks are not freed!\n", f->count);
		else for (uint16_t i = 0; i < NODE_LEN; i++) {
			assert(f->items[i].index == FREE_INDEX); // important too
		}
#endif

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

#if ALLOCATOR_DEBUG
	for (uint16_t i = 0; i < NODE_LEN; i++) {
		n->items[i].magic = ITEM_MAGIC; // item magic number
		n->items[i].index = FREE_INDEX;
	}
#endif

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

#if ALLOCATOR_DEBUG
	assert(n->magic == NODE_MAGIC); // pedantic
#endif

	return n;
}

static inline void checkimagic(struct alloc_item* it) {
#if ALLOCATOR_DEBUG
	assert(it->magic == ITEM_MAGIC); // pedantic
#else
	(void)it;
#endif
}

struct chunk* allocChunk(int16_t x, int16_t y) {
	c89mtx_lock(&alloc_mutex);
	struct alloc_node* n = findCoolNode();

	uint16_t i = 0;
	for (i = n->empty; i < NODE_LEN; i++) {
		if (n->items[i].index == FREE_INDEX) { // cool
			break;
		}
	}
	n->empty = i+1; // update empty index; (overflow is ok)

	assert(i < NODE_LEN && "heap corruption! (invalid count)");
	n->count++; // one more item is busy now
	n->items[i].index = i; // hehe

	checkimagic(&(n->items[i])); // pedantic again

	c89mtx_unlock(&alloc_mutex);

	// small initialization.
	// keep in mind, that all chunks are memsetted-zero by default and after free!
	struct chunk* c = &(n->items[i].data); // well done	
	c->pos.axis[0] = x;
	c->pos.axis[1] = y;
	return c;
}

static struct alloc_item* dataToNode(void* p, struct alloc_node** dst) {
	const char* offset = &(((struct alloc_item*)0)->data);
	struct alloc_item* it = (struct alloc_item*)((char*)p - offset);

	checkimagic(it);
	uint16_t i = it->index;
	assert(i != FREE_INDEX && "attempt to double free");
	assert(i < NODE_LEN); // including FREE_INDEX
	struct alloc_item* fi = it - i; // first item
	
	const char* offset2 = &(((struct alloc_node*)0)->items);
	struct alloc_node* n = (struct alloc_node*)((char*)fi - offset2);
	
#if ALLOCATOR_DEBUG
	assert(n->magic == NODE_MAGIC); // pedantic
#endif

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

	assert(orig != &empty);

	struct alloc_node* n = NULL;
	struct alloc_item* it = dataToNode((void*)orig, &n);
	assert(n && it);

	n->count--;
	if (n->empty > it->index) n->empty = it->index; // set new empty node index
	it->index = FREE_INDEX; // yeah
	checkimagic(it);

	// allocated chunk excepted to be zeroed later on.
	memset(&it->data, 0, sizeof(struct chunk));

	c89mtx_unlock(&alloc_mutex); // done
}
