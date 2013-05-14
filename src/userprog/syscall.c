#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/synch.h"

/* This is a skeleton system call handler */

typedef int pid_t;

static void syscall_handler (struct intr_frame *);
static int sys_write (int fd, const void *buffer, unsigned length);
static int sys_exit (int s);
static int sys_halt (void);
static int sys_exec (const char *cmd);
static int sys_wait (pid_t pid);
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

  if (!is_user_vaddr(p))
    sys_exit(-1);

  if (*p < SYS_HALT || *p > SYS_INUMBER)
    sys_exit(-1);

  if (!(is_user_vaddr (p+5) && is_user_vaddr(p+6) && is_user_vaddr(p+7)))
    sys_exit(-1);

  switch (*p) {
  case SYS_WRITE :
    ret = sys_write(*(p+5), *(p+6), *(p+7));
    break;
  case SYS_HALT :
    ret = sys_halt();
  case SYS_EXIT :
    ret = sys_exit(-1);
  case SYS_WAIT :
    ret = sys_wait(*(p+1));
  case SYS_EXEC :
    ret = sys_exec(*(p+1));
  }

  f -> eax = ret;

  //thread_exit();
  return;
}
 static int
sys_write (int fd, const void *buffer, unsigned length)
{
  struct file * f;
  int ret;

  ret = -1;
  lock_acquire (&file_lock);
  if (fd == STDOUT_FILENO){ /* stdout */
    putbuf (buffer, length);
  }
  else if (fd == STDIN_FILENO) /* stdin */
    goto done;
  else if (!is_user_vaddr (buffer) || !is_user_vaddr (buffer + length))
  {
    lock_release (&file_lock);
    sys_exit (-1);
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
  return length;
}

static int
sys_exit (int s) {
  thread_exit();
  return;
}

static int
sys_halt (void) {
  power_off();
}

static int
sys_exec (const char *cmd)
{
  int ret;
  if (!cmd || !is_user_vaddr (cmd))
    return -1;
  lock_acquire ( &file_lock);
  ret = process_execute (cmd);
  lock_release ( &file_lock);
  return ret;
}

static int
sys_wait (pid_t pid)
{
  return process_wait (pid);
}
