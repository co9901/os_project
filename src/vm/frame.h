#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H
#include "threads/thread.h"
#include "threads/palloc.h"
#include <list.h>
#include "threads/synch.h"


struct frame_entry{
	//has physical address
	void *frame;
	//has page
	void *page;
	//thread belongs to
	struct thread *t;
	struct list_elem elem;
};

struct frame_table{
	struct list frame_list;
};

void init_frame_table (void);
void *get_frame (int palloc_flags);
bool free_frame (void *frame);
void set_page_in_frame(void *kpage, void *upage);
#endif
