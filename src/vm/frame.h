//#define USERPROG_PROCESS_H
#include "threads/thread.h"
#include "threads/palloc.h"
#include <list.h>
#include "threads/synch.h"
#include "devices/disk.h"
enum frame_flags
{
    FRAME_MMAP = 001,
    FRAME_EXEC = 002,
    FRAME_SWAP = 004,
    FRAME_DIRTY = 010,
    FRAME_ACCESSED = 020,
    FRAME_IO = 040
};
struct frame_entry{
	//has physical address
	void *frame;
	//has page
	void *page;
	//thread belongs to
	struct thread *t;
	struct list_elem elem;
	int flags;
	struct list pte_list;
	size_t read_bytes;
	disk_sector_t sector_no;
	uintptr_t frame_addr;
};

struct frame_table{
	struct list frame_list;
};
void evict(struct frame_entry *);
void init_frame_table (void);
void *get_frame (int palloc_flags);
bool free_frame (void *frame);
void set_page_in_frame(void *kpage, void *upage);
