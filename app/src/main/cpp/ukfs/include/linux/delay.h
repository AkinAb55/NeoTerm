/* uKernel hamis <linux/delay.h> — alvás/várakozás. */
#ifndef _UK_LINUX_DELAY_H
#define _UK_LINUX_DELAY_H

void msleep(unsigned int msecs);
void mdelay(unsigned int msecs);
void udelay(unsigned int usecs);
void ndelay(unsigned int nsecs);
unsigned long msleep_interruptible(unsigned int msecs);
void usleep_range(unsigned long min_us, unsigned long max_us);

#endif
