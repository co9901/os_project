#include "threads/vaddr.h"

void init_swap_disk(void);
void free_swap_disk(void);
int get_slot(void);
int swap_out(void *page);
bool swap_in(int index, void *page);
bool remove_swap_slot(int index);
