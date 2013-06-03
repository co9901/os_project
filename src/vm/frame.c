#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"

// frame table
struct frame_table *frame_tb;
struct lock framelock;

void
init_frame_table(void)
{
	frame_tb = (struct frame_table*)malloc(sizeof(struct frame_table));
	list_init( &(frame_tb->frame_list) );
	lock_init(&framelock);
}

void*
get_frame(int flags)
{
	//lock_acquire( &(frame_tb->lock) );
	lock_acquire(&framelock);
	
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

	lock_release(&framelock);
	//lock_release( &(frame_tb->lock) );

	return page;
}

bool
free_frame(void *frame)
{
	//lock_acquire( &(frame_tb->lock) );
	lock_acquire(&framelock);
	
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
			
			lock_release(&framelock);
			return true;
		}
	}
	
	lock_release(&framelock);
	//lock_release( &(frame_tb->lock) );
	return false;
}

// set page in frame table corresponded to a kpage
void
set_page_in_frame(void *kpage, void *upage)
{
	//lock_acquire( &(frame_tb->lock) );
	lock_acquire(&framelock);

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

	lock_release(&framelock);
	//lock_release( &(frame_tb->lock) );
}


void evict(struct frame_entry *victim)
{
    if(victim->flags & FRAME_DIRTY)
    {
	if(victim->read_bytes == 0)
	{
	    int i;
	    uint8_t *page = ptov(victim->frame);
	    for(i= PGSIZE-1;i>=0;i--)
	    {
		if(page[i]!=0)
		break;
	    }
	    victim->read_bytes = i+1;
	}
	victim->flags &= ~FRAME_EXEC;
    }
   // swap_out(victim);
}
