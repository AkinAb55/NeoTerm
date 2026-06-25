/* uKernel hamis <linux/completion.h> — pthread cond fölött. */
#ifndef _UK_LINUX_COMPLETION_H
#define _UK_LINUX_COMPLETION_H

#include <pthread.h>
#include <linux/wait.h>	/* a valodi kernelhez hasonloan: wait_queue_head_t */

struct completion {
	pthread_mutex_t m;
	pthread_cond_t  c;
	unsigned        done;
};

void init_completion(struct completion *x);
void reinit_completion(struct completion *x);
void complete(struct completion *x);
void complete_all(struct completion *x);
void wait_for_completion(struct completion *x);
/* ms-os timeout; visszaad: hátralévő jiffies (0 = lejárt). */
unsigned long wait_for_completion_timeout(struct completion *x, unsigned long timeout_jiffies);
int  wait_for_completion_interruptible(struct completion *x);

#endif
