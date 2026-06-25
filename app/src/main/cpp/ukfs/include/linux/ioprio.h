#ifndef _UK_LINUX_IOPRIO_H
#define _UK_LINUX_IOPRIO_H
#include <linux/sched.h>
enum { IOPRIO_CLASS_NONE = 0, IOPRIO_CLASS_RT = 1, IOPRIO_CLASS_BE = 2, IOPRIO_CLASS_IDLE = 3, IOPRIO_CLASS_INVALID = -1 };
#define IOPRIO_NR_LEVELS 8
#define IOPRIO_PRIO_VALUE(class, level) ((((class) & 0x7) << 13) | ((level) & 0x1fff))
#define IOPRIO_PRIO_CLASS(ioprio) (((ioprio) >> 13) & 0x7)
#define IOPRIO_PRIO_LEVEL(ioprio) ((ioprio) & 0x1fff)
struct io_context { unsigned short ioprio; };
static inline int get_task_ioprio(struct task_struct *task) { (void)task; return 0; }
static inline int set_task_ioprio(struct task_struct *task, int ioprio) { (void)task;(void)ioprio; return 0; }
#endif
