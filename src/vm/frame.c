#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

// frame table
struct frame_table *frame_tb;

void
init_frame_table(void)
{
	frame_tb = (struct frame_table*)malloc(sizeof(struct frame_table));
	lock_init( &(frame_tb->lock) );
	list_init( &(frame_tb->frame_list) );
}

void*
get_frame(int flags)
{
	lock_acquire( &(frame_tb->lock) );
	
	struct thread *t = thread_current();
	void *page = palloc_get_page (flags);

	// no free page
	if(page == NULL){
		//replacement should be implemented
		PANIC("no enough frames!");
	}
	else{ // there is a free page
		struct frame_entry *frame = (struct frame_entry*)malloc(sizeof(struct frame_entry));
		if(!frame){
			PANIC("no enought memory for malloc");
		}
		frame->frame = page;
		frame->t = t;
		frame->page = NULL; // initiating later
		list_push_back ( &(frame_tb->frame_list), &(frame->elem) );
	}

	lock_release( &(frame_tb->lock) );

	return page;
}

bool
free_frame(void *frame)
{
	lock_acquire( &(frame_tb->lock) );
	
	struct list_elem *e;
	struct frame_entry *fe;
	
	for(e = list_begin( &(frame_tb->frame_list) ); e != list_end( &(frame_tb->frame_list) ); e = list_next(e))
	{
		fe = list_entry(e, struct frame_entry, elem);
		if(fe != NULL && fe->frame == frame){ // find!
			fe->frame = NULL;
			fe->page = NULL;
			fe->t = NULL;
			list_remove( &(fe->elem) );
			free(fe);
			
			lock_release( &(frame_tb->lock) );
			return true;
		}
	}
	
	lock_release( &(frame_tb->lock) );
	return false;
}

// set page in frame table corresponded to a kpage
void
set_page_in_frame(void *kpage, void *upage)
{
	lock_acquire( &(frame_tb->lock) );

	struct list_elem *e;
	struct frame_entry *fe;

	for(e = list_begin( &(frame_tb->frame_list) ); e != list_end( &(frame_tb->frame_list) ); e = list_next(e))
	{
		fe = list_entry(e, struct frame_entry, elem);
		if(fe != NULL && fe->frame == kpage){
			fe->page = upage;
			break;
		}
	}

	lock_release( &(frame_tb->lock) );
}
