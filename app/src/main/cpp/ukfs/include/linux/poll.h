/* uKernel hamis <linux/poll.h> — minimál poll-támogatás. */
#ifndef _UK_LINUX_POLL_H
#define _UK_LINUX_POLL_H

#include <linux/wait.h>

#define POLLIN     0x0001
#define POLLPRI    0x0002
#define POLLOUT    0x0004
#define POLLERR    0x0008
#define POLLHUP    0x0010
#define POLLRDNORM 0x0040
#define POLLWRNORM 0x0100

typedef unsigned int __poll_t;
typedef struct poll_table_struct poll_table;

/* a driver poll_wait(file, &wq, pt)-t hív; userspace-ben no-op (a proxy a
 * tényleges adatkészséget a visszaadott maszkból olvassa). */
static inline void poll_wait(struct file *f, wait_queue_head_t *wq, poll_table *p)
{ (void)f; (void)wq; (void)p; }

#endif
