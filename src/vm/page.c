#include "vm/page.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/threads.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

// hash key is vaddr


// return a hash value for page p
unsigned
page_hash(const struct hash_elem *p, void *aus UNUSED)
{
	const struct page *pg  = hash_entry(p, struct page, hash_elem);
	return hash_bytes(&pg->vaddr, sizeof(pg->vaddr));
}

// return true if page a precedes page b
bool
page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
	const struct page *p1 = hash_entry(a, struct page, hash_elem);
	const struct page *p2 = hash_entry(b, struct page, hash_elem);
	return a->vaddr < b->vaddr;
}

// find a supplemental page corresponding to given virtual address
struct page*
find_page(struct hash *suppagetable, const void *addr)
{
	struct page *p;
	struct hash_elem *e;

	p.vaddr = addr;
	e = hash_find(suppagetable, &p.hash_elem);

	return e != NULL ? hash_entry(e, struct page, hash_elem) : NULL;
}

// init supplement page table
void
init_sup_page(struct hash *suppagetable)
{
	hash_init(suppagetable, page_hash, page_less, NULL);
}

// insert new supplement page to table
void
insert_sup_page(struct hash *suppagetable, void *vaddr, void *paddr)
{
	struct page *p = (struct page*)malloc(sizeof(struct page));
	
	if(p == NULL)
		PANIC("no enough memory for page");
	else{
		p->vaddr = vaddr;
		p->paddr = paddr;
		p->valid_page = true;
		p->swap_index = -1;
		p->thread = thread_current();
		// hash insert return NULL if success, else return a member if it already exists
		if( hash_insert(suppagetable, &p->hash_elem) != NULL )
			PANIC("same virtual address");
	}
}

// remove corresponding supplement page from table
void
remove_sup_page(struct hash *suppagetable)
{
	hash_clear(suppagetable, free_page);
}

void
free_page(struct hash_elem *e, void *aux UNUSED)
{
	struct page *p = hash_entry(e, struct page, hash_elem);
	free(p);
}
