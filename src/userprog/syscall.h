#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int sys_exit(int status);
typedef void* mapid_t;
mapid_t mmap (int, void *);

#endif /* userprog/syscall.h  */
