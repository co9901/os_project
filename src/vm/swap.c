#include "vm/swap.h"
#include "devices/disk.h"
#include <bitmap.h>

#define SLOT_SIZE 1024*4

struct bitmap *swap_table;

void
init_swap_disk(void)
{
	swap_table = bitmap_create(SLOT_SIZE);
	// bitmap first init as false
}

int
get_slot(void)
{
	int idx = bitmap_scan(swap_table, 0, 1, false);
	if(idx == BITMAP_ERROR)
		PANIC("not enough swap slot");
	else
		return idx;
}

int
swap_out(void *page)
{
	int i, idx = get_slot();
	disk_sector_t swap_sector_start = (PGSIZE * idx) / DISK_SECTOR_SIZE;

	for(i = 0; i < PGSIZE; i += DISK_SECTOR_SIZE){
		disk_write (disk_get(1, 1), swap_sector_start, page + i);
		swap_sector_start++;
	}
	
	bitmap_set(swap_table, idx, true);

	return idx;
}

bool
swap_in(int index, void *page)
{
	int i;
	disk_sector_t swap_sector_start = (PGSIZE * index) / DISK_SECTOR_SIZE;
	
	if (bitmap_test(swap_table, index) == false)
		return false;
	// disk_sector 단위로 읽는다
	for(i = 0; i < PGSIZE; i += DISK_SECTOR_SIZE){
		disk_read (disk_get(1, 1), swap_sector_start, page + i);
		swap_sector_start++;
	}

	bitmap_set(swap_table, index, false);

	return true;
}

void free_swap_disk() {
  bitmap_destroy(swap_table);
}

bool
remove_swap_slot(int idx)
{
	if (bitmap_test(swap_table, idx))
		return false;
	bitmap_set(swap_table, idx, false);
	return true;
}
