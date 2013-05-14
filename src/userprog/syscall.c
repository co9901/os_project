#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/synch.h"

/* This is a skeleton system call handler */

static void syscall_handler (struct intr_frame *);
static int sys_write (int fd, const void *buffer, unsigned length);
static struct lock file_lock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

static void
syscall_handler (struct intr_frame *f) 
{
  int *p;
  int ret;
  p = f -> esp;
  if (*p == SYS_WRITE) {
    ret = sys_write(*(p+1), *(p+2), *(p+3));
    f -> eax = ret;
  }

  thread_exit();
  return;
}
 static int
sys_write (int fd, const void *buffer, unsigned length)
{
  struct file * f;
  int ret;

  ret = -1;
  lock_acquire (&file_lock);
  if (fd == STDOUT_FILENO) /* stdout */
    putbuf (buffer, length);
  else if (fd == STDIN_FILENO) /* stdin */
    goto done;
  else if (!is_user_vaddr (buffer) || !is_user_vaddr (buffer + length))
  {
    lock_release (&file_lock);
    //sys_exit (-1);
  }
  else
  {
    /* f = find_file_by_fd (fd);*/
    /* if (!f)*/
    /*   goto done;*/

    /* ret = file_write (f, buffer, length);*/
  }

done:
  lock_release (&file_lock);
  return ret;
}
