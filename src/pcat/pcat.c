/*++
/* NAME
/*	pcat 1
/* SUMMARY
/*	copy process memory
/* SYNOPSIS
/*	\fBpcat\fR [\fB-hHv\fR] [\fB-m\fI mapfile\fR] \fIprocess_id\fR
/* DESCRIPTION
/*	\fBpcat\fR hooks onto the process specified with \fIprocess_id\fR
/*	and copies the contents of its memory to standard output. By default,
/*	\fBpcat\fR skips over holes in the process address space.
/*	Consequently, absolute location information is lost.
/*
/*	 Options:
/* .IP "\fB-h\fR (default)"
/*	Skip over holes in the process address space, so that absolute
/*	location information is lost.
/* .IP \fB-H\fR
/*	Preserve holes in the process address space, so that absolute
/*	location information is preserved.  This option writes holes to
/*	the output file, and requires that stdout is redirected to file.
/*	This option does not work on some Solaris versions.
/* .IP "\fB-m\fR \fImapfile\fR"
/*	Print the process memory map to \fImapfile\fR, one entry per line.
/*	Specify \fB-m-\fR to write to the standard error stream.
/*	Each map entry consists of a region start address and the first
/*	address beyond that region. Addresses are separated by space,
/*	and are printed as hexadecimal numbers (0xhhhh).
/* .IP \fB-v\fR
/*	Enable verbose mode, for debugging purposes.
/* BUGS
/*	On systems with a usable \fB/proc\fR file system, \fBpcat\fR
/*	does not stop the target process before accessing its memory.
/*	This can result in a loss of accuracy.
/*
/*	\fBpcat\fR uses ptrace(2) when the system lacks a usable
/*	\fB/proc\fR file system. This can be very, very, slow.
/*
/*	The use of ptrace(2) causes the target process to be stopped
/*	while its memory being is copied. This can be undesirable.
/*
/*	On some systems, the ptrace(2) detach operation resumes a process
/*	that was stopped prior to the ptrace(2) attach operation.
/*	\fBpcat\fR attempts to compensate by sending SIGSTOP signals when
/*	it detaches from the process.
/*
/*	On some systems, the ptrace(2) detach operation leaves a process
/*	stopped that was not stopped prior to the ptrace(2) attach operation.
/*	\fBpcat\fR attempts to compensate by sending a SIGCONT signal when
/*	it detaches from the process.
/*
/*	On most non-\fB/proc\fR systems, \fBpcat\fR accesses kernel data
/*	structures and therefore 1) needs super-user privilege and 2) must
/*	be compiled specifically for the machine that it runs on.
/*
/*	\fBpcat\fR will not copy its own memory.
/*
/*	\fBpcat\fR will not copy system processes (i.e. processes
/*	that live entirely inside the kernel).
/* HISTORY
/* .fi
/* .ad
/*	This \fBpcat\fR command was written for the coroner's toolkit.
/*	However, a command with the same name exists on some versions
/*	of System V UNIX. That command, a file decompression utility,
/*	should not be confused with the command described in this
/*	manual page.
/* LICENSE
/*	This software is distributed under the IBM Public License.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

 /*
  * Solaris 2.x has /proc, which immensely simplifies our task. However, the
  * implementation has changed over time, so it is as if we're dealing with
  * two completely different operating systems.
  */
#ifdef SUNOS5_0_5
#define SUPPORTED
#include <sys/procfs.h>
#define HAVE_PROC_MEM
#define USE_PREAD
#define USE_PWRITE
#endif

#ifdef SUNOS5
#define SUPPORTED
#include <sys/types.h>			/* Sheesh. */
#include <procfs.h>
#define USE_PREAD
#define USE_PWRITE
#define HAVE_PROC_MEM
#endif

 /*
  * FreeBSD 2.x and later have /proc, which immensely simplifies our task.
  * Unfortunately, FreeBSD 5.x no longer mounts /proc by default. We try to
  * use /proc first and use ptrace() only if we have to.
  * 
  * FreeBSD PTRACE_DETACH does not resume the target process so we must send
  * SIGCONT, but only if the process was stopped by us.
  * 
  * FreeBSD 5 no longer supports ptrace() access to the u area, so we have to
  * grope kernel memory instead.
  */
#if defined(FREEBSD2) || defined(FREEBSD3) || defined(FREEBSD4) \
	|| defined(FREEBSD5) || defined(FREEBSD6) || defined(FREEBSD7)
#define SUPPORTED
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <kvm.h>
#include <stddef.h>
#define HAVE_PROC_MEM
#define HAVE_PTRACE_MEM
#define PTRACE_ATTACH	PT_ATTACH
#define PTRACE_DETACH	PT_DETACH
#define PTRACE_PEEKDATA	PT_READ_D
#define PTRACE_ARG3_T	caddr_t
#endif

#if defined(FREEBSD2) || defined(FREEBSD3) || defined(FREEBSD4)
#define PROCP_STATUS(p) ((p)->kp_proc.p_stat)
#define PROCP_VMSPACE(p) ((p)->kp_proc.p_vmspace)
#endif

#if defined(FREEBSD5) || defined(FREEBSD6) || defined(FREEBSD7)
#define PROCP_STATUS(p) ((p)->ki_stat)
#define PROCP_VMSPACE(p) ((p)->ki_vmspace)
#endif

 /*
  * Linux has /proc. However, at this time, /proc/pid/mem is broken: it maps
  * to the memory of the probing process, not the memory of the target. So we
  * we still have to use slow old ptrace() to grab process memory.
  */
#ifdef LINUX2
#define SUPPORTED
#define HAVE_PTRACE_MEM
#endif

 /*
  * BSD/OS does not have /proc. It does have kvm_uread(), which is much
  * faster than grabbing process memory with ptrace(), but kvm_uread() can
  * give us only the resident memory pages. So we're stuck with slow old
  * ptrace(). Fortunately, ptrace() peeking the u area is enough to get all
  * the memory segment information that we need.
  * 
  * BSD/OS 2.x PTRACE_ATTACH is a privileged operation, so the program still
  * needs to be run by root.
  * 
  * BSD/OS 2.x and 3.x PTRACE_DETACH does not resume the target process so we
  * must send SIGCONT, but only if the process was stopped by us.
  * 
  * BSD/OS 2.x .. 4.x ptrace() resumes a stopped process when attaching and
  * detaching two times, so we must re-suspend upon detach.
  */
#if defined(BSDI2) || defined(BSDI3) || defined(BSDI4)
#define SUPPORTED
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <kvm.h>
#include <stddef.h>
#define HAVE_PTRACE_MEM
#define PTRACE_ATTACH	PT_ATTACH
#define PTRACE_DETACH	PT_DETACH
#define PTRACE_PEEKDATA	PT_READ_D
#define PTRACE_PEEKUSER	PT_READ_U
#define PTRACE_ARG3_T	void *
#endif

 /*
  * OpenBSD has no /proc, and ptrace() gives no access to the u area. We must
  * grope kernel memory in order to find out the process memory segment
  * sizes. This requires super-user privilege.
  * 
  * OpenBSD 2.4 ctob() is broken so we roll our own.
  * 
  * OpenBSD 2.4 ptrace() resumes a stopped process when detaching, so we must
  * re-suspend upon detach.
  */
#if defined(OPENBSD2) || defined(OPENBSD3) || defined(OPENBSD4)
#define SUPPORTED
#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <kvm.h>
#include <stddef.h>
#define HAVE_PTRACE_MEM
#define PTRACE_ATTACH	PT_ATTACH
#define PTRACE_DETACH	PT_DETACH
#define PTRACE_PEEKDATA	PT_READ_D
#define HAVE_BROKEN_CTOB
#define PTRACE_ARG3_T	caddr_t
#endif

 /*
  * SunOS 4.x has no /proc, and ptrace() peeking the u area won't give us the
  * process memory segment sizes. Instead we must grope process information
  * from kernel virtual memory. This requires super-user privilege.
  * 
  * SunOS 4.x ptrace() resumes a stopped process when detaching, so we must
  * re-suspend upon detach.
  */
#ifdef SUNOS4
#define SUPPORTED
#include <sys/param.h>
#include <machine/vmparam.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <kvm.h>
#define USE_ON_EXIT
#define HAVE_PTRACE_MEM
#define USE_PTRACE_READDATA
#define offsetof(type, member)	((size_t)(&((type *)0)->member))
extern char *optarg;
extern int optind;

#endif

 /*
  * Catch-all.
  */
#ifndef SUPPORTED
#error "This operating system is not supported"
#endif

#include "error.h"
#include "mymalloc.h"

 /*
  * Structure to hold one process memory map entry.
  */
typedef unsigned long MEM_OFFSET;

typedef struct {
    MEM_OFFSET start;			/* start address */
    MEM_OFFSET end;			/* actually one past */
} MAP_INFO;

 /*
  * Structure to carry around process-related info.
  */
typedef struct PROC_INFO {
#ifdef HAVE_PROC_MEM
    int     mem_fd;			/* process memory */
#endif
    pid_t   pid;			/* a process id */
    int     map_count;			/* nr of map entries */
    void    (*read_proc) (struct PROC_INFO *, char *, int, off_t);
    MAP_INFO map_info[1];		/* actually a bunch. */
} PROC_INFO;

 /*
  * Other stuff.
  */
static FILE *map_out;
static int keep_holes;

 /*
  * Begin stuff for systems that must use ptrace().
  */
#ifdef HAVE_PTRACE_MEM
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <setjmp.h>

 /*
  * On some systems, wait() after PTRACE_ATTACH can block forever when the
  * target process is already stopped. We set an alarm clock to prevent us
  * from getting stuck.
  */
static jmp_buf waitbuf;

 /*
  * Deal with ptrace() interface inconsistencies.
  */
#ifndef PTRACE_ARG3_T
#define PTRACE_ARG3_T	int
#endif

 /*
  * On some systems we must send SIGCONT after grabbing memory, but of course
  * we must send SIGCONT only if we actually stopped the process. On some
  * systems we must send SIGSTOP when done, but only if the process was
  * already stopped by someone else. Obviously all this is subject to race
  * conditions.
  */
static int pre_detach_signal;
static int post_detach_signal;

 /*
  * End stuff for systems that must use ptrace().
  */
#endif

 /*
  * How to clean up.
  */
#ifdef USE_ON_EXIT
#define EXIT_HANDLER_ARGS	int unused_status, void *unused_context
#define SET_EXIT_HANDLER(x)	on_exit((x), (char *) 0)
#else
#define EXIT_HANDLER_ARGS	void
#define SET_EXIT_HANDLER(x)	atexit(x)
#endif

 /*
  * Some UNIXen offer /proc memory access. Ideally we do not want things to
  * change while we're looking, so we should suspend process execution first.
  * However, doing this is fraught with peril, especially in combination with
  * job control-enabled shells and suspended processes. For now we punt and
  * leave the target process running.
  * 
  * Some UNIXen have no /proc access to process memory and must use the old
  * ptrace() debugging interface. There are several quirks with using this
  * interface.
  * 
  * 1 - Attaching to a process with ptrace() always causes the process to be
  * stopped.
  * 
  * 2 - On some systems, detaching from the process leaves the process stopped
  * when it should be resumed.
  * 
  * 3 - On some systems, detaching from the process causes the process to be
  * resumed when it should remain stopped.
  * 
  * 4 - The ptrace() interface is defined in terms of memory words. We prefer
  * character buffers where possible. The macros below ensure that the buffer
  * sizes match (but not necessarily their alignments).
  * 
  * 5 - If we don't properly detach from the target process it will hang. That
  * is undesirable if we want to grab memory without disturbing the target.
  * We must jump some hoops to ensure that we always clean up before exiting.
  * 
  * 6 - If we're ptrace()ing our own parent process it is possible that we exit
  * before the parent is resumed, so that the parent isn't properly notified
  * of our termination and doesn't wake up.  Finding this race condition cost
  * me a couple days. The workaround is to pause a while.
  */
#define READ_BUFSIZ_WORDS	1024
#define READ_BUFSIZ_CHARS	(READ_BUFSIZ_WORDS * sizeof(int))

static pid_t saved_pid;

/* cleanup - detach from process and allow it to continue */

static void cleanup(void)
{
    if (saved_pid) {
	if (verbose)
	    fprintf(stderr, "cleanup\n");
#ifdef HAVE_PTRACE_MEM
	if (verbose)
	    remark("pre_detach_signal = %d", pre_detach_signal);
	if (pre_detach_signal)
	    kill(saved_pid, pre_detach_signal);
	ptrace(PTRACE_DETACH, saved_pid, (PTRACE_ARG3_T) 1, 0);
	if (verbose)
	    remark("post_detach_signal = %d", post_detach_signal);
	if (post_detach_signal)
	    kill(saved_pid, post_detach_signal);
#endif
	if (saved_pid == getppid())		/* XXX $%^@ */
	    sleep(2);
	saved_pid = 0;
    }
}

/* sig_cleanup - detach from target process upon signal arrival */

static void sig_cleanup(int sig)
{
    cleanup();
    _exit(0);
}

/* exit_cleanup - detach from target process upon program exit */

static void exit_cleanup(EXIT_HANDLER_ARGS)
{
    cleanup();
}

/* init_cleanup - prepare for cleanup before all exits */

static void init_cleanup(pid_t pid)
{
    if (signal(SIGHUP, sig_cleanup) == SIG_ERR)
	error("signal SIGHUP: %m");
    if (signal(SIGINT, sig_cleanup) == SIG_ERR)
	error("signal SIGINT: %m");
    if (signal(SIGQUIT, sig_cleanup) == SIG_ERR)
	error("signal SIGQUIT: %m");
    if (signal(SIGTERM, sig_cleanup) == SIG_ERR)
	error("signal SIGTERM: %m");
    SET_EXIT_HANDLER(exit_cleanup);
    saved_pid = pid;
}

#ifdef HAVE_PTRACE_MEM

/* ptrace_attach_timeout - wake up after alarm */

static void ptrace_attach_timeout(int unused_sig)
{
    longjmp(waitbuf, 1);
}

/* ptrace_attach_wait - wait for child after attaching to it */

static int ptrace_attach_wait(pid_t pid)
{
    int     status;

    if (setjmp(waitbuf) == 0) {
	signal(SIGALRM, ptrace_attach_timeout);
	alarm(5);
	if (wait(&status) < 0)
	    error("wait: %m");
	alarm(0);
	if (!WIFSTOPPED(status))
	    error("process %d is dead", pid);
	return (0);
    } else {
	return (-1);
    }
}

#ifndef USE_PTRACE_READDATA

/* call_ptrace - ptrace() with error handling */

static int call_ptrace(int request, pid_t pid, int addr, int data)
{
    int     result;

    errno = 0;
    result = ptrace(request, pid, (PTRACE_ARG3_T) addr, data);
    if (errno)
	error("ptrace: %m%s", errno == EIO ?
	      "; did you use GCC with another machine's header files?" : "");
    return (result);
}

#endif

#endif

#ifdef HAVE_PROC_MEM

/* read_proc_mem - read block of memory at specified position */

static void read_proc_mem(PROC_INFO *proc, char *data, int len, off_t offset)
{
    if (verbose)
	fprintf(stderr, "read seek to 0x%lx\n", (long) offset);
#ifdef USE_PREAD
    if (pread(proc->mem_fd, data, len, offset) != len)
	error("pread: %m");
#else
    if (lseek(proc->mem_fd, offset, SEEK_SET) < 0)
	error("input seek: %m");
    if (read(proc->mem_fd, data, len) != len)
	error("read: %m");
#endif
}

#endif

#ifdef HAVE_PTRACE_MEM

/* read_ptrace_mem - read block of memory at specified position */

static void read_ptrace_mem(PROC_INFO *proc, char *data, int len, off_t offset)
{
#ifdef USE_PTRACE_READDATA
    if (verbose)
	fprintf(stderr, "read seek to 0x%lx\n", (long) offset);
    if (ptrace(PTRACE_READDATA, proc->pid, (int) offset, len, data) < 0)
	error("PTRACE_READDATA: %m%s", errno == EIO ?
	      "; did you use GCC with another machine's header files?" : "");
#else
    int     words[READ_BUFSIZ_WORDS];
    int     addr;
    int     n;

    /*
     * XXX This breaks when memory segments aren't word-aligned or when
     * memory segments sizes aren't a multiple of the word size. Tough.
     */
    if (verbose)
	fprintf(stderr, "read seek to 0x%lx\n", (long) offset);
    if (offset % sizeof(int))
	panic("read_proc: offset 0x%lx is not word-aligned", (long) offset);
    if (len % sizeof(int))
	panic("read_proc: request size %d is not word-aligned", len);
    if (verbose)
	fprintf(stderr, "read seek to 0x%lx\n", (long) offset);
    for (n = 0, addr = (int) offset; n < len / sizeof(int); addr += sizeof(int), n++)
	words[n] = call_ptrace(PTRACE_PEEKDATA, proc->pid, addr, 0);
    memcpy(data, (char *) words, len);
#endif
}

#endif

/* write_here - write a block at specified position */

static void write_here(int fd, char *data, int len, off_t offset)
{
    if (verbose)
	fprintf(stderr, "write seek to 0x%lx\n", (long) offset);
#ifdef USE_PWRITE
    if (pwrite(fd, data, len, offset) != len)
	error("pwrite: %m");
#else
    if (lseek(fd, offset, SEEK_SET) < 0)
	error("output seek: %m");
    if (write(fd, data, len) != len)
	error("write: %m");
#endif
}

/* copy_process - copy process following the memory map */

static void copy_process(PROC_INFO *proc, int out_fd)
{
    char    buf[READ_BUFSIZ_CHARS];
    long    size;
    off_t   where;
    int     len;
    int     n;

    for (n = 0; n < proc->map_count; n++) {
	size = proc->map_info[n].end - proc->map_info[n].start;
	where = proc->map_info[n].start;
	while (size > 0) {
	    len = (size > sizeof(buf) ? sizeof(buf) : size);
	    proc->read_proc(proc, buf, len, where);
	    if (keep_holes) {
		write_here(out_fd, buf, len, where);
	    } else {
		if (write(out_fd, buf, len) != len)
		    error("write: %m");
	    }
	    size -= len;
	    where += len;
	}
    }
}

/* open_process - open a process */

static PROC_INFO *open_process(pid_t pid)
{
#if defined(FREEBSD2) || defined(FREEBSD3) || defined(FREEBSD4) \
	|| defined(FREEBSD5) || defined(FREEBSD6) || defined(FREEBSD7)
    PROC_INFO *proc = (PROC_INFO *) mymalloc(sizeof(*proc));
    MAP_INFO *mp;
    struct kinfo_proc *procp;
    kvm_t  *kd;
    struct vmspace vmspace;
    struct vm_map_entry entry;
    u_long  addr;
    int     cnt;
    char    buf[READ_BUFSIZ_CHARS];
    FILE   *map_fp;

    /*
     * Attach to process memory. Try to use /proc first. XXX Suspend/resume
     * the process if it isn't stopped.
     */
    init_cleanup(pid);
    sprintf(buf, "/proc/%ld/mem", (long) pid);
    if ((proc->mem_fd = open(buf, O_RDONLY)) >= 0) {
	proc->read_proc = read_proc_mem;

	/*
	 * Look up the process memory map.
	 * 
	 * XXX The map must fit inside one read operation. If the read fails
	 * with EFBIG then we should increase the read buffer size and retry.
	 */
	sprintf(buf, "/proc/%ld/map", (long) pid);
	if ((map_fp = fopen(buf, "r")) == 0)
	    error("open %s: %m", buf);

	for (proc->map_count = 0; fgets(buf, sizeof(buf), map_fp) != 0; proc->map_count++) {
	    if (proc->map_count > 0)
		proc = (PROC_INFO *) myrealloc((char *) proc,
		sizeof(*proc) + proc->map_count * sizeof(proc->map_info[0]));
	    mp = proc->map_info + proc->map_count;
	    if (sscanf(buf, "%lx %lx", &mp->start, &mp->end) != 2)
		error("unexpected map format: %s", buf);
	    if (verbose)
		fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
	    if (map_out)
		fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
	}
	if (ferror(map_fp))
	    error("map read: %m");
	(void) fclose(map_fp);
    }

    /*
     * We can't use /proc so we fall back to ptrace() and to peeking at
     * kernel memory. Look up the process status before attaching to it: 1)
     * the ptrace() detach operation will resume a stopped process, so we
     * must re-suspend it; 2) the ptrace() detach operation will not resume a
     * process that wasn't stopped, so we must resume it.
     */
    else {
	proc->read_proc = read_ptrace_mem;

	/*
	 * Look up the process status before attaching to it: PTRACE_DETACH
	 * will resume a stopped process, so we must re-suspend it.
	 */
	if ((kd = kvm_open((char *) 0, (char *) 0, (char *) 0, O_RDONLY, "pcat")) == 0)
	    error("kvm_open: %m");
	if ((procp = kvm_getprocs(kd, KERN_PROC_PID, pid, &cnt)) == 0 || cnt != 1)
	    error("kvm_getprocs: %m");
	if (PROCP_STATUS(procp) & SSTOP)
	    pre_detach_signal = post_detach_signal = SIGSTOP;
	else
	    pre_detach_signal = SIGCONT;

	/*
	 * Attach to process memory and stop the process.
	 */
	init_cleanup(pid);
	if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0)
	    error("ptrace PTRACE_ATTACH: %m");
	ptrace_attach_wait(pid);

	/*
	 * Look up the process memory map. With FreeBSD 5 the u area is no
	 * longer accessible via ptrace() so we must grope kernel memory.
	 * This requires root privileges.
	 */
	if (kvm_read(kd, (u_long) PROCP_VMSPACE(procp),
		     (void *) &vmspace, sizeof(vmspace)) != sizeof(vmspace))
	    error("struct vmspace kvm_read: %m");

	/*
	 * Copied from the code that implements /proc/pid/map.
	 */
	for (proc->map_count = 0, addr = (u_long) vmspace.vm_map.header.next;
	     addr != (u_long) PROCP_VMSPACE(procp)
	     + offsetof(struct vmspace, vm_map)
	     + offsetof(struct vm_map, header);
	     proc->map_count++, addr = (u_long) entry.next) {

	    if (kvm_read(kd, addr, (void *) &entry,
			 sizeof(entry)) != sizeof(entry))
		error("struct vm_map_entry kvm_read: %m");
	    if (proc->map_count > 0)
		proc = (PROC_INFO *) myrealloc((char *) proc,
		sizeof(*proc) + proc->map_count * sizeof(proc->map_info[0]));
	    mp = proc->map_info + proc->map_count;
	    mp->start = entry.start;
	    mp->end = entry.end;
	    if (verbose)
		fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
	    if (map_out)
		fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
	}

	kvm_close(kd);
    }
    proc->pid = pid;

    return (proc);
#endif

#ifdef SUNOS5_0_5
    PROC_INFO *proc = (PROC_INFO *) mymalloc(sizeof(*proc));
    char    buf[READ_BUFSIZ_CHARS];
    struct prmap *prmap;
    struct prmap *pr;
    MAP_INFO *mp;

    proc->read_proc = read_proc_mem;

    /*
     * Attach to process memory. XXX Suspend/resume the process if it isn't
     * stopped.
     */
    sprintf(buf, "/proc/%d", (long) pid);
    if ((proc->mem_fd = open(buf, O_RDONLY, 0)) < 0)
	error("open %s: %m", buf);
    init_cleanup(pid);

    /*
     * Look up the process memory map.
     */
    if (ioctl(proc->mem_fd, PIOCNMAP, (char *) &proc->map_count) < 0)
	error("ioctl PIOCNMAP: %m");
    proc = (PROC_INFO *) myrealloc((char *) proc,
	       sizeof(*proc) + proc->map_count * sizeof(proc->map_info[0]));
    prmap = (struct prmap *) mymalloc((proc->map_count + 1) * sizeof(*prmap));
    if (ioctl(proc->mem_fd, PIOCMAP, (char *) prmap) < 0)
	error("ioctl PIOCMAP: %m");
    for (mp = proc->map_info, pr = prmap; pr < prmap + proc->map_count; mp++, pr++) {
	mp->start = (MEM_OFFSET) pr->pr_vaddr;
	mp->end = (MEM_OFFSET) pr->pr_vaddr + pr->pr_size;
	if (verbose)
	    fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
	if (map_out)
	    fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
    }
    free((char *) prmap);

    proc->pid = pid;

    return (proc);
#endif

#ifdef SUNOS5
    PROC_INFO *proc = (PROC_INFO *) mymalloc(sizeof(*proc));
    char    buf[READ_BUFSIZ_CHARS];
    prmap_t prmap;
    MAP_INFO *mp;
    FILE   *map_fp;

    proc->read_proc = read_proc_mem;

    /*
     * Attach to process memory. XXX Suspend/resume the process if it isn't
     * stopped.
     */
    sprintf(buf, "/proc/%ld/as", (long) pid);
    if ((proc->mem_fd = open(buf, O_RDONLY)) < 0)
	error("open %s: %m", buf);
    init_cleanup(pid);

    /*
     * Look up the process memory map.
     */
    sprintf(buf, "/proc/%ld/map", (long) pid);
    if ((map_fp = fopen(buf, "r")) == 0)
	error("open %s: %m", buf);

    for (proc->map_count = 0; fread((char *) &prmap, sizeof(prmap), 1, map_fp) == 1; proc->map_count++) {
	if (proc->map_count > 0)
	    proc = (PROC_INFO *) myrealloc((char *) proc,
	       sizeof(*proc) + proc->map_count * sizeof(proc->map_info[0]));
	mp = proc->map_info + proc->map_count;
	mp->start = prmap.pr_vaddr;
	mp->end = prmap.pr_vaddr + prmap.pr_size;
	if (verbose)
	    fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
	if (map_out)
	    fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
    }
    if (ferror(map_fp))
	error("map read: %m");
    (void) fclose(map_fp);

    proc->pid = pid;

    return (proc);
#endif

#ifdef LINUX2
    PROC_INFO *proc = (PROC_INFO *) mymalloc(sizeof(*proc));
    char    buf[READ_BUFSIZ_CHARS];
    MAP_INFO *mp;
    FILE   *map_fp;

    /*
     * Attach to process memory and stop the process.
     */
    init_cleanup(pid);
#ifdef HAVE_PROC_MEM
    proc->read_proc = read_proc_mem;
    sprintf(buf, "/proc/%ld/mem", (long) pid);
    if ((proc->mem_fd = open(buf, O_RDONLY)) < 0)
	error("open %s: %m", buf);
#endif
#ifdef HAVE_PTRACE_MEM
    proc->read_proc = read_ptrace_mem;
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0)
	error("ptrace PTRACE_ATTACH: %m");
    ptrace_attach_wait(pid);
#endif

    /*
     * Look up the process memory map.
     */
    sprintf(buf, "/proc/%ld/maps", (long) pid);
    if ((map_fp = fopen(buf, "r")) == 0)
	error("open %s: %m", buf);

    for (proc->map_count = 0; fgets(buf, sizeof(buf), map_fp) != 0; proc->map_count++) {
	if (proc->map_count > 0)
	    proc = (PROC_INFO *) myrealloc((char *) proc,
	       sizeof(*proc) + proc->map_count * sizeof(proc->map_info[0]));
	mp = proc->map_info + proc->map_count;
	if (sscanf(buf, "%lx-%lx", &mp->start, &mp->end) != 2)
	    error("unexpected map format: %s", buf);
	if (verbose)
	    fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
	if (map_out)
	    fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
    }
    if (ferror(map_fp))
	error("map read: %m");
    (void) fclose(map_fp);

    proc->pid = pid;

    return (proc);
#endif

#if defined(BSDI2) || defined(BSDI3) || defined(BSDI4)
    PROC_INFO *proc;
    MAP_INFO *mp;
    kvm_t  *kd;
    struct kinfo_proc *procp;
    int     cnt;

    /*
     * Look up the process status before attaching to it: 1) doing attach and
     * detach twice will resume a stopped process, so we must re-suspend it;
     * 2) detach will not resume a process that wasn't stopped, so we must
     * resume it.
     */
    if ((kd = kvm_open((char *) 0, (char *) 0, (char *) 0, O_RDONLY, "pcat")) == 0)
	error("kvm_open: %m");
    if ((procp = kvm_getprocs(kd, KERN_PROC_PID, pid, &cnt)) == 0 || cnt != 1)
	error("kvm_getprocs: %m");
    if (procp->kp_proc.p_stat & SSTOP)
	pre_detach_signal = post_detach_signal = SIGSTOP;
#if defined(BSDI2) || defined(BSDI3)
    else
	pre_detach_signal = SIGCONT;
#endif
    kvm_close(kd);

    /*
     * The process has three memory sections.
     */
    proc = (PROC_INFO *) mymalloc(sizeof(*proc) + 2 * sizeof(MAP_INFO));
    proc->map_count = 3;
    proc->read_proc = read_ptrace_mem;

    /*
     * Attach to process memory and stop the process.
     */
    init_cleanup(pid);
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0)
	error("ptrace PTRACE_ATTACH: %m");
    ptrace_attach_wait(pid);

    /*
     * Text segment.
     */
    mp = proc->map_info;
    mp->start = call_ptrace(PTRACE_PEEKUSER, pid,
		  offsetof(struct user, u_kproc.kp_eproc.e_vm.vm_taddr), 0);
    mp->end = mp->start + ctob(call_ptrace(PTRACE_PEEKUSER, pid,
		 offsetof(struct user, u_kproc.kp_eproc.e_vm.vm_tsize), 0));
    if (verbose)
	fprintf(stderr, "text segment:  0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    /*
     * Data segment.
     */
    mp += 1;
    mp->start = call_ptrace(PTRACE_PEEKUSER, pid,
		  offsetof(struct user, u_kproc.kp_eproc.e_vm.vm_daddr), 0);
    mp->end = mp->start + ctob(call_ptrace(PTRACE_PEEKUSER, pid,
		 offsetof(struct user, u_kproc.kp_eproc.e_vm.vm_dsize), 0));
    if (verbose)
	fprintf(stderr, "data segment:  0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    /*
     * Stack segment.
     */
    mp += 1;
#if defined(BSDI2) || defined(BSDI3)
    mp->end = USRSTACK;
#else
    mp->end = call_ptrace(PTRACE_PEEKUSER, pid,
		  offsetof(struct user, u_kproc.kp_eproc.e_vm.vm_saddr), 0);
#endif
    mp->start = mp->end - ctob(call_ptrace(PTRACE_PEEKUSER, pid,
		 offsetof(struct user, u_kproc.kp_eproc.e_vm.vm_ssize), 0));
    if (verbose)
	fprintf(stderr, "stack segment: 0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    proc->pid = pid;

    return (proc);
#endif

#if defined(OPENBSD2) || defined(OPENBSD3) || defined(OPENBSD4)
    PROC_INFO *proc;
    MAP_INFO *mp;
    struct kinfo_proc *procp;
    kvm_t  *kd;
    struct vmspace vmspace;
    int     cnt;

#ifdef HAVE_BROKEN_CTOB
    int     pagesize = getpagesize();

#define CTOB(x) ((x) * pagesize)
#endif

    /*
     * Look up the process status before attaching to it: PTRACE_DETACH will
     * resume a stopped process, so we must re-suspend it.
     */
    if ((kd = kvm_open((char *) 0, (char *) 0, (char *) 0, O_RDONLY, "pcat")) == 0)
	error("kvm_open: %m");
    if ((procp = kvm_getprocs(kd, KERN_PROC_PID, pid, &cnt)) == 0 || cnt != 1)
	error("kvm_getprocs: %m");
    if (procp->kp_proc.p_stat & SSTOP)
	pre_detach_signal = SIGSTOP;

    /*
     * The process has three memory sections.
     */
    proc = (PROC_INFO *) mymalloc(sizeof(*proc) + 2 * sizeof(MAP_INFO));
    proc->map_count = 3;
    proc->read_proc = read_ptrace_mem;

    /*
     * Attach to process memory and stop the process.
     */
    init_cleanup(pid);
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0)
	error("ptrace PTRACE_ATTACH: %m");
    ptrace_attach_wait(pid);

    /*
     * Look up the process memory map. The u area is not accessible via
     * ptrace() so we must grope kernel memory.
     */
    if (kvm_read(kd, (u_long) procp->kp_proc.p_vmspace,
		 (void *) &vmspace, sizeof(vmspace)) != sizeof(vmspace))
	error("kvm_read: %m");

    /*
     * Text segment.
     */
    mp = proc->map_info;
    mp->start = (MEM_OFFSET) vmspace.vm_taddr;
    mp->end = mp->start + CTOB(vmspace.vm_tsize);
    if (verbose)
	fprintf(stderr, "text segment:  0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    /*
     * Data segment.
     */
    mp += 1;
    mp->start = (MEM_OFFSET) vmspace.vm_daddr;
    mp->end = mp->start + CTOB(vmspace.vm_dsize);
    if (verbose)
	fprintf(stderr, "data segment:  0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    /*
     * Stack segment.
     */
    mp += 1;
    mp->end = USRSTACK;
    mp->start = mp->end - CTOB(vmspace.vm_ssize);
    if (verbose)
	fprintf(stderr, "stack segment: 0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
    kvm_close(kd);

    proc->pid = pid;

    return (proc);
#endif

#ifdef SUNOS4
    PROC_INFO *proc;
    MAP_INFO *mp;
    struct proc *procp;
    kvm_t  *kd;

    /*
     * Look up the process status before attaching to it: PTRACE_DETACH will
     * resume a stopped process, so we must re-suspend it.
     */
    if ((kd = kvm_open((char *) 0, (char *) 0, (char *) 0, O_RDONLY, "pcat")) == 0)
	error("kvm_open: %m");
    if ((procp = kvm_getproc(kd, pid)) == 0)
	error("kvm_getproc: %m");
    if (procp->p_stat & SSTOP)
	pre_detach_signal = SIGSTOP;

    /*
     * The process has three memory sections.
     */
    proc = (PROC_INFO *) mymalloc(sizeof(*proc) + 2 * sizeof(MAP_INFO));
    proc->map_count = 3;
    proc->read_proc = read_ptrace_mem;

    /*
     * Attach to process memory and stop the process.
     */
    init_cleanup(pid);
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) < 0)
	error("ptrace PTRACE_ATTACH: %m");
    ptrace_attach_wait(pid);

    /*
     * Look up the process memory map. The executable file header info in the
     * u area is not sufficient so we must grope kernel memory.
     */
    if ((procp = kvm_getproc(kd, pid)) == 0)
	error("kvm_getproc: %m");

    /*
     * Text segment.
     */
    mp = proc->map_info;
    mp->start = USRTEXT;
    mp->end = mp->start + ctob(procp->p_tsize);
    if (verbose)
	fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    /*
     * Data segment. Assume it is contiguous with the text segment.
     */
    mp += 1;
    mp->start = mp[-1].end;
    mp->end = mp->start + ctob(procp->p_dsize);
    if (verbose)
	fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);

    /*
     * Stack segment.
     */
    mp += 1;
    mp->end = USRSTACK;
    mp->start = mp->end - ctob(procp->p_ssize);
    if (verbose)
	fprintf(stderr, "map entry: 0x%lx 0x%lx\n", mp->start, mp->end);
    if (map_out)
	fprintf(map_out, "0x%lx 0x%lx\n", mp->start, mp->end);
    kvm_close(kd);

    proc->pid = pid;

    return (proc);
#endif
}

/* close_process - close a process */

static void close_process(PROC_INFO *proc)
{
#ifdef HAVE_PROC_MEM
    if (proc->mem_fd >= 0 && close(proc->mem_fd) < 0)
	error("close memory: %m");
#endif
    free((char *) proc);
}

/* usage - explain and terminate */

static void usage()
{
    error("usage: %s [-H (keep holes)] [-m mapfile] [-v] process_id",
	  progname);
}

/* main - open process and dump memory */

int     main(int argc, char **argv)
{
    PROC_INFO *proc;
    pid_t   pid;
    int     ch;

    progname = argv[0];

    while ((ch = getopt(argc, argv, "hHm:v")) > 0) {
	switch (ch) {
	default:
	    usage();
	case 'h':
	    keep_holes = 0;
	    break;
	case 'H':
	    keep_holes = 1;
	    break;
	case 'm':
	    if (strcmp(optarg, "-") == 0) {
		map_out = stderr;
	    } else {
		if ((map_out = fopen(optarg, "w")) == 0)
		    error("create map file %s: %m", optarg);
	    }
	    break;
	case 'v':
	    verbose++;
	    break;
	}
    }

    if (optind + 1 != argc)
	usage();
    if ((pid = atoi(argv[optind])) <= 0)
	usage();
    if (keep_holes && lseek(fileno(stdout), 0L, SEEK_SET) < 0)
	error("-H requires seekable output file");

    /*
     * Sorry, I ran out of time. Should jump the appropriate system-dependent
     * hoops to get the process flags.
     */
    if (pid < 10 && pid != 1)
	error("process %d is a system process", pid);
    if (pid == getpid())
	error("will not attach to myself!");

    /*
     * Open a process, copy it and close it.
     */
    proc = open_process(pid);
    copy_process(proc, fileno(stdout));
    close_process(proc);

    exit(0);
}
