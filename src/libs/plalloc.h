/* 
 * PLALLOC - dynamic linear pool allocator.
 * Copyright (C) 2023 UtoECat
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without 
 * restriction, including without limitation the rights to use, 
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/*
 * This allocator is my only work, that was evolved during years and years in
 * my head and in my different projects, and now... it's here. Headeronly,
 * easy to add in project and use, dependent only on the stdlib and 
 * c89threads(optional), dynamic linear allocator lib :D
 * 
 * To add this in your project, you may also need to create empty .c file with 
 * this content : ```
 * #define PLALLOC_IMPLEMENTATION
 * #include "plalloc.h"
 * ```
 * Do this only once. You may also come with different approaches, like include
 * this text in exact same rder directly to your main.c, or miniaudio.c... Does
 * not matter.
 *
 * This project activates all SANITY_CHECK() macro, if you define PLALLOC_DEBUG
 * above `#include "plalloc.h"` in your implementation file. This is useful for
 * debug builds of your program to catch bad things early and with conclusion 
 * about where(in allocator) and why it's happened. Altrough, it will make this
 * allocator a bit slower.
 * Without PLALLOC_DEBUG, some assertions will still be checked, but they are rare,
 * and located at the end of all fun, when everything may be already very broken. 
 *
 * "c89threads.h" is used by this allocator to make it threadsafe in multithreaded
 * enviroment. You can get rid of this dependency by defining PLALLOC_OMIT_THREADSAFETY
 * above `#include "plalloc.h"`.
 *
 * And for extra crazy guys : 
 * You can change default malloc() and free() functions, used by this
 * allocator, by redefining MEM_ALLOC(), MEM_FREE() and MEM_CHECK()
 * macros, inside source file with defined PLALLOC_IMPLEMENTATION, 
 * above `#include "plalloc.h"`!
 *
 * Also, you can direcly edit this file and macros values for your own needs :)
 * Altrogh be careful, some values, like PAGE_SIZE_MIN and ITEMS_COUNT_DEFAULT 
 * must not be setted to ultra crazy low, or ultra crazy high values, since allcator
 * code does not make checks for that.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>

#ifndef _plalloc_h_
#define _plalloc_h_

typedef struct plalloc_s* plalloc_t;

/*
 * initializes allocaltor context. 
 *
 * First argument specifies size of the item/memory blocks
 * to allocate by this allocator (that's it, you can't
 * allocate any-sized memory block in one allocator instance). 
 *
 * Second argument is count of items PER PAGE. More items 
 * in theblock -> more "speed", but more memory usage too, 
 * if you will not take place of all theese items.
 *
 * Keep in mind, that's too big count of elements per page
 * may actually be very wasteful for memory!
 *
 * Same goes in back direction : one, two, three items per page
 * is extremely small, but hopefully this is impossible to 
 * set that small count of elements (see below).
 *
 * Negative count of elements uses "default" value. 
 * Same action performed in case page is less than 1024 bytes!
 * Also, the count of elements is capped to UINT16_MAX-1, 
 * and can't be greater.
 *
 * And look - COUNT PER PAGE!!! You can allocate as much
 * items as you want, and as much as free memory you have, 
 * without bothering yourself about implementation details!
 */
plalloc_t plalloc_initialize  (size_t itemsize, short int count);
void      plalloc_uninitialize(plalloc_t alloc);

/*
 * allocates new item with the size, specified as
 * argument for allocator constructor.
 *
 * MUST be freed later, using ONLY plalloc_free()!
 * The free() function from stdlib will not work with this!
 * 
 * Same thing about an attemts to use plalloc_free() to free content,
 * allocated by stdlib's malloc() or calloc() and etc.
 */
void*     plalloc_alloc(plalloc_t alloc);
void      plalloc_free (plalloc_t, void*);

#endif /* _plalloc_h_ */

#ifdef PLALLOC_IMPLEMENTATION

// theese are flags you may want to change
#define ITEMS_COUNT_DEFAULT 128 // default count of items per page
#define PAGE_SIZE_MIN 1024 // minimal summary size of the page

#ifndef PLALLOC_OMIT_THREADSAFETY 
#include "c89threads.h"
#define ALLOC_MUTEX_FIELD c89mtx_t mutex;
#define ALLOC_MUTEX_INIT(A) c89mtx_init(&A->mutex, 0);
#define ALLOC_MUTEX_FREE(A) c89mtx_destroy(&A->mutex);
#define ALLOC_MUTEX_LOCK(A) c89mtx_lock(&A->mutex);
#define ALLOC_MUTEX_UNLOCK(A) c89mtx_unlock(&A->mutex);
#else
#define ALLOC_MUTEX_FIELD
#define ALLOC_MUTEX_INIT(A) 
#define ALLOC_MUTEX_FREE(A)
#define ALLOC_MUTEX_LOCK(A)
#define ALLOC_MUTEX_UNLOCK(A)
#endif

#define ITEM_FREE  32767   // special item index. means, that this item is not allocted
#define ITEM_MAGIC 0x0AFF  // magic value. Used to check for potencial overwrites and corruption

typedef struct alloc_head alloc_head; // header of the item used by allocator
struct alloc_page         alloc_page; // the PAGE, NODE...

// abstract header of the alloc item is :
struct alloc_head {
	uint16_t magic;
	uint16_t index; // or busy if contains a value of the FREE_ITEM
	char alignment[alignof(void*) - sizeof(uint16_t)*2];
	// data goes here;
};

typedef struct alloc_head alloc_item;

struct alloc_page { // "page"
	uint16_t count; // count of allocated items
	uint16_t empty; // index of empty item
	struct alloc_page *next; // this pointer here for proper alignment
	char     items[0];
};

struct plalloc_s {
	ALLOC_MUTEX_FIELD;
	struct alloc_page* list, *last;
	size_t itemsize;
	unsigned short int pagecount;
};

// implementation

#include <string.h>
#include <assert.h>
#include <stdio.h>
#define  MEM_ALLOC(s) malloc(s)
#define  MEM_FREE(p)  free(p)
#define  ALLOC_CHECK(p) (p) == NULL /* may replace to true, if your allocator crashes at error anyway */

#if PLALLOC_DEBUG
#define SANITY_CHECK(expr, ...) assert((expr) && __VA_ARGS__ "");
#define SANITY_CODE(...) __VA_ARGS__
#else
#define SANITY_CHECK(expr, ...) /* nothing */
#define SANITY_CODE(...) /* nothing */
#endif

static size_t getnodesize(const plalloc_t alloc) {
	SANITY_CHECK(alloc->itemsize > 0, "allocator corruption");
	return sizeof(struct alloc_head) + alloc->itemsize;
}

static size_t getpagesize(const plalloc_t alloc) {
	SANITY_CHECK(alloc->pagecount > 0 && alloc->pagecount < ITEM_FREE, "allocator corruption");
	return sizeof(struct alloc_page) +
		getnodesize(alloc) * alloc->pagecount;
}

static alloc_item* getItem(plalloc_t alloc, struct alloc_page* p, uint16_t i) {
	SANITY_CHECK(i < alloc->pagecount, "index is out of bounds");
	alloc_item* item = (alloc_item*)(p->items + getnodesize(alloc) * i);
	return item;
}

static struct alloc_page* createpage(plalloc_t alloc) {
	struct alloc_page* page = MEM_ALLOC(getpagesize(alloc));
	if (ALLOC_CHECK(page)) return NULL;
	SANITY_CODE(fprintf(stderr, "page %p created!\n", page));
	page->count = 0;
	page->empty = 0;
	page->next  = NULL;

	for (int i = 0; i < alloc->pagecount; i++) {
		alloc_item* item = getItem(alloc, page, i);
		memset(item, 0, getnodesize(alloc));
		item->index = ITEM_FREE;
		item->magic = ITEM_MAGIC;
	}

	// insert at the end
	SANITY_CHECK((alloc->list && alloc->last) || (!alloc->last && !alloc->list), "bad list state");
	if (alloc->last) alloc->last->next = page;
	alloc->last = page;

	return page;
}

plalloc_t plalloc_initialize (size_t itemsize, short int count) {
	if (!itemsize) return NULL; // error
	if (count < 1) count = ITEMS_COUNT_DEFAULT;
	SANITY_CHECK(count > 1, "logic error");

	plalloc_t alloc = MEM_ALLOC(sizeof(struct plalloc_s));
	if (ALLOC_CHECK(alloc)) return NULL;
	ALLOC_MUTEX_INIT(alloc);
	alloc->itemsize = itemsize;
	alloc->pagecount = count;
	alloc->last = NULL;
	alloc->list = NULL;

	// check for pagecount limits
	if (getpagesize(alloc) < PAGE_SIZE_MIN) {
		alloc->pagecount = (PAGE_SIZE_MIN / getnodesize(alloc)) + 1;
	}
	if (alloc->pagecount >= ITEM_FREE) {
		alloc->pagecount = ITEM_FREE - 1;
	}
	SANITY_CHECK(alloc->pagecount > 1, "pagecount len computation error");
	SANITY_CODE(fprintf(stderr, "cnt=%i, sz=%i\n", alloc->pagecount, alloc->itemsize));

	alloc->list = createpage(alloc);
	if (ALLOC_CHECK(alloc->list)) {
		alloc->last = NULL;
		plalloc_uninitialize(alloc); // free NOW!
		return NULL;
	}
	return alloc;
}

// general puprose all in one page test
SANITY_CODE(
static void sanitize_page(plalloc_t alloc, struct alloc_page* p, int check_all) {
	int first_check = 0;
	int real_count = 0;
	SANITY_CHECK(p->count <= alloc->pagecount, "invalid items count");
	SANITY_CHECK(p->empty <= alloc->pagecount, "invalid empty index");
	
	if (!check_all) return;
	int empty = p->empty;

	for (int i = 0; i < alloc->pagecount; i++) {
		alloc_item* item = getItem(alloc, p, i);
		SANITY_CHECK(item->magic == ITEM_MAGIC, "magic is corrupted!");
		if (item->index != ITEM_FREE) {
			SANITY_CHECK(item->index == i, "invalid item index!");
			real_count++;
			if (i == empty) empty++;
		} else if (!first_check) {
			if (empty != i) 
				fprintf(stderr, "got at %i, expected at %i\n", p->empty, i);
			SANITY_CHECK(empty == i, "invalid empty index");
			first_check = 1;
		}
		assert(item->magic == ITEM_MAGIC); // NOT SANITY, since may be corrupted by app
	}
}
)

void destroypage(plalloc_t alloc, struct alloc_page* p) {
	int leaks = 0;

	for (int i = 0; i < alloc->pagecount; i++) {
		alloc_item* item = getItem(alloc, p, i);
		if (item->index != ITEM_FREE) {
			leaks++;
		}
		assert(item->magic == ITEM_MAGIC); // NOT SANITY, since may be corrupted by app
	}

	if (leaks) {
		fprintf(stderr, "[plalloc] : %i leaks detected at page %p!", leaks, p);
	}
	MEM_FREE(p);
}

void plalloc_uninitialize(plalloc_t alloc) {
	if (!alloc) return;
	struct alloc_page* n = alloc->list, *f = NULL;
	SANITY_CHECK(alloc->itemsize && alloc->pagecount, "finalizing not complete allocator");

	while (n) {
		f = n;
		n = n->next;
		SANITY_CODE(sanitize_page(alloc, f, 1));
		destroypage(alloc, f);
	}

	alloc->list = NULL;
	alloc->last = NULL;
	ALLOC_MUTEX_FREE(alloc);
	MEM_FREE(alloc);
}

struct alloc_page* findFreePage(plalloc_t alloc) {
	SANITY_CHECK(alloc);
	struct alloc_page* page = alloc->last;
	SANITY_CHECK(alloc->last);
	while (page && page->count >= alloc->pagecount) {
		SANITY_CODE(sanitize_page(alloc, page, 0));
		page = page->next;
	}
	if (!page) return createpage(alloc);
	return page;
}

void*     plalloc_alloc(plalloc_t alloc) {
	if (!alloc) return NULL; // protect API

	ALLOC_MUTEX_LOCK(alloc);
	struct alloc_page* page = findFreePage(alloc);
	if (ALLOC_CHECK(page)) return NULL;

	// a bit overkill, but it's testing, we can do that :D
	SANITY_CODE(sanitize_page(alloc, page, 1));

	for (int i = page->empty; i < alloc->pagecount; i++) {
		alloc_item* item = getItem(alloc, page, i);
		if (item->index == ITEM_FREE) {
			item->index = i;
			page->empty = i + 1;
			page->count++;
			SANITY_CHECK(item->magic == ITEM_MAGIC);
			ALLOC_MUTEX_UNLOCK(alloc);
			return (char*)item + sizeof(struct alloc_head);
		}
	}
	assert(0 && "bad allocator page retrieved!");
	ALLOC_MUTEX_UNLOCK(alloc);
	return NULL;
}

void      plalloc_free (plalloc_t alloc, void* p) {
	if (!p) return;
	ALLOC_MUTEX_LOCK(alloc);
	alloc_item* item = (alloc_item*)((char*)p - sizeof(struct alloc_head));

	assert(item->magic == ITEM_MAGIC && "item is corrupted or not plalloc'd"); // not sanity
	SANITY_CHECK(item->index != ITEM_FREE, "double free or corruption"); // sanity
	SANITY_CHECK(item->index < alloc->pagecount); // sanity

	// goto first item in page
	alloc_item* first = (void*)((char*)item - (item->index *
		getnodesize(alloc))); 
	SANITY_CHECK(first->magic == ITEM_MAGIC, "bad magic on first item. Maybe corruption");
	SANITY_CHECK(first->index == 0 || first->index == ITEM_FREE, "first item excepted. Maybe corruption");

	struct alloc_page* page = (void*)((char*)first - offsetof(struct alloc_page, items));
	SANITY_CHECK(page->count <= alloc->pagecount, "page corruption or logic error");
	SANITY_CHECK(page->empty <= alloc->pagecount, "page corruption or logic error");

	page->count--;
	if (page->empty > item->index) page->empty = item->index;
	memset(item, 0, getnodesize(alloc));
	item->index = ITEM_FREE;
	item->magic = ITEM_MAGIC;

	// overkill
	SANITY_CODE(sanitize_page(alloc, page, 1));

	ALLOC_MUTEX_UNLOCK(alloc);
}

#endif
