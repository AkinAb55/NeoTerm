/* uKernel hamis <linux/jiffies.h> — monoton óra alapú jiffies. */
#ifndef _UK_LINUX_JIFFIES_H
#define _UK_LINUX_JIFFIES_H

#include <linux/types.h>

#define HZ 1000UL   /* 1 jiffy = 1 ms ebben az emulációban */

extern volatile unsigned long jiffies;   /* a sched.c frissíti / olvasáskor számolja */
unsigned long ukernel_jiffies(void);
#define jiffies (ukernel_jiffies())

#define time_after(a, b)      ((long)((b) - (a)) < 0)
#define time_before(a, b)     time_after(b, a)
#define time_after_eq(a, b)   ((long)((a) - (b)) >= 0)
#define time_before_eq(a, b)  time_after_eq(b, a)

#define msecs_to_jiffies(m)   ((unsigned long)(m))           /* HZ=1000 */
#define jiffies_to_msecs(j)   ((unsigned int)(j))
#define usecs_to_jiffies(u)   ((unsigned long)((u) / 1000))

#define MAX_JIFFY_OFFSET ((long)(~0UL >> 1))
#endif
