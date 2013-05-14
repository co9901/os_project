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

typedef int pid_t;

static int sys_write (int fd, const void *buffer, unsigned length);
static int sys_halt (void);
static int sys_create (const char *file, unsigned initial_size);
static int sys_open (const char *file);
static int sys_close (int fd);
static int sys_read (int fd, void *buffer, unsigned size);
static int sys_exec (const char *cmd);
static int sys_wait (pid_t pid);
static int sys_filesize (int fd);
static int sys_tell (int fd);
static int sys_seek (int fd, unsigned pos);
static int sys_remove (const char *file);

static struct file *find_file_by_fd (int fd);
static struct fd_elem *find_fd_elem_by_fd (int fd);
static int alloc_fid (void);
static struct fd_elem *find_fd_elem_by_fd_in_process (int fd);

typedef int (*handler) (uint32_t, uint32_t, uint32_t);
static handler syscall_vec[128];
static struct lock file_lock;

struct fd_elem
{
	int fd;
	struct file *file;
	struct list_elem elem;
	struct list_elem thread_elem;
};

static struct list file_list;

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

int
sys_exit(int status)
{
	struct thread *t;
	struct list_elem *l;

	t = thread_current ();
	while (!list_empty (&t->files))
	{
		l = list_begin (&t->files);
		sys_close (list_entry (l, struct fd_elem, thread_elem)->fd);
	}

	t->ret_status = status;
	thread_exit ();
	return -1;
}

	static int
sys_close(int fd)
{
	struct fd_elem *f;
	int ret;

	f = find_fd_elem_by_fd_in_process (fd);

	if (!f) /* Bad fd */
		goto done;
	file_close (f->file);
	list_remove (&f->elem);
	list_remove (&f->thread_elem);
	free (f);

done:
	return 0;
}

	static struct fd_elem *
find_fd_elem_by_fd (int fd)
{
	struct fd_elem *ret;
	struct list_elem *l;

	for (l = list_begin (&file_list); l != list_end (&file_list); l = list_next (l))
	{
		ret = list_entry (l, struct fd_elem, elem);
		if (ret->fd == fd)
			return ret;
	}

	return NULL;
}

	static int
alloc_fid (void)
{
	static int fid = 2;
	return fid++;
}

	static int
sys_filesize (int fd)
{
	struct file *f;

	f = find_file_by_fd (fd);
	if (!f)
		return -1;
	return file_length (f);
}

	static int
sys_tell (int fd)
{
	struct file *f;

	f = find_file_by_fd (fd);
	if (!f)
		return -1;
	return file_tell (f);
}

	static int
sys_seek (int fd, unsigned pos)
{
	struct file *f;

	f = find_file_by_fd (fd);
	if (!f)
		return -1;
	file_seek (f, pos);
	return 0; /* Not used */
}

	static int
sys_remove (const char *file)
{
	if (!file)
		return false;
	if (!is_user_vaddr (file))
		sys_exit (-1);

	return filesys_remove (file);
}

	static struct fd_elem *
find_fd_elem_by_fd_in_process (int fd)
{
	struct fd_elem *ret;
	struct list_elem *l;
	struct thread *t;

	t = thread_current ();

	for (l = list_begin (&t->files); l != list_end (&t->files); l = list_next (l))
	{
		ret = list_entry (l, struct fd_elem, thread_elem);
		if (ret->fd == fd)
			return ret;
	}

	return NULL;
}
