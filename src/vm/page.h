#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H
#include "threads/thread.h"
#include "threads/palloc.h"
#include <list.h>
#include "threads/synch.h"


struct page{
	// hash table
	struct hash_elem hash_elem;
	// virtual address
	void *vaddr;
	// physical address
	void *paddr;
	// is page in memory or disk
	bool valid_page;
	// which thread has this page;
	struct thread *t;
	// location where it swapped
	// if this page is not swapped, then index is negative, esp. -1
	int swap_index;
};

unsigned page_hash(const struct hash_elem *p, void *aux UNUSED);
bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED);
struct page* find_page(struct hash *suppagetable, const void *addr);
void init_sup_page(struct hash *suppagetable);
void insert_sup_page(struct hash *suppagetable, void *vaddr, void *paddr);
void remove_sup_page(struct hash *suppagetable);
void free_page(struct hash_elem *e, void *aux UNUSED);
