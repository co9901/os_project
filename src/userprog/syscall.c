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

	if (!is_user_vaddr(p))
		sys_exit(-1);

	if (*p < SYS_HALT || *p > SYS_INUMBER)
		sys_exit(-1);

	if (!(is_user_vaddr (p+1) && is_user_vaddr(p+2) && is_user_vaddr(p+3)))
		sys_exit(-1);


	switch (*p) {
		case SYS_WRITE :
			ret = sys_write(*(p+1), *(p+2), *(p+3));
			break;
		case SYS_HALT :
			ret = sys_halt();
			break;
		case SYS_WAIT :
			ret = sys_wait(*(p+1));
			break;
		case SYS_EXEC :
			ret = sys_exec(*(p+1));
			break;
		case SYS_EXIT:
			ret = sys_exit(*(p+1));
			break;
		case SYS_READ:
			ret = sys_read(*(p+1),*(p+2),*(p+3));
			break;
		case SYS_CLOSE:
			ret = sys_close(*(p+1));
			break;
		case SYS_FILESIZE:
			ret = sys_filesize(*(p+1));
			break;
		case SYS_CREATE:
			ret = sys_create(*(p+1),*(p+2));
			break;
		case SYS_OPEN:
			ret = sys_open(*(p+1));
			break;
	}

	f -> eax = ret;

	return;
}

	static int
sys_write (int fd, const void *buffer, unsigned length)
{
	struct file * f;
	int ret;
	fd =1;
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
sys_create (const char *file, unsigned init_size)
{
	if(!file)
		return sys_exit(-1);
	return filesys_create(file,init_size);
}
static int sys_open (const char *file)
{
	struct file *f;
	struct fd_elem *fde;
	int ret;
	ret = -1;
	if(!file){return -1;}

	f = filesys_open(file);
	if(!f){goto done;}
	fde = (struct fd_elem *)malloc(sizeof (struct fd_elem));
	if(!fde)
	{
		file_close(f);
		goto done;
	}
	fde->file = f;
	fde->fd = alloc_fid();
	list_push_back(&file_list,&fde->elem);
	list_push_back(&thread_current ()->files,&fde->thread_elem);
	ret = fde->fd;
done:
	return ret;
}

static int sys_close (int fd)
{
	struct fd_elem *f;
	int ret;

	f = find_fd_elem_by_fd_in_process (fd);

	if (!f) 
		goto done;
	file_close (f->file);
	list_remove (&f->elem);
	list_remove (&f->thread_elem);
	free (f);

done:
	return 0;
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
sys_read (int fd, void *buffer, unsigned size)
{
	struct file * f;
	unsigned i;
	int ret;

	ret = -1; /* Initialize to zero */
	lock_acquire (&file_lock);
	if (fd == STDIN_FILENO) /* stdin */
	{
		for (i = 0; i != size; ++i)
			*(uint8_t *)(buffer + i) = input_getc ();
		ret = size;
		goto done;
	}
	else if (fd == STDOUT_FILENO) /* stdout */
		goto done;
	else if (!is_user_vaddr (buffer) || !is_user_vaddr (buffer + size)) /* bad ptr */
	{
		lock_release (&file_lock);
		sys_exit (-1);
	}
	else
	{
		f = find_file_by_fd (fd);
		if (!f)
			goto done;
		ret = file_read (f, buffer, size);
	}

done:
	lock_release (&file_lock);
	return ret;
	return ret;
}

	static struct file *
find_file_by_fd (int fd)
{
	struct fd_elem *ret;

	ret = find_fd_elem_by_fd (fd);
	if (!ret)
		return NULL;
	return ret->file;
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

	static int
sys_wait (pid_t pid)
{
	return process_wait (pid);
}
