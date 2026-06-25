#ifndef _UK_LINUX_PM_H
#define _UK_LINUX_PM_H
typedef struct pm_message { int event; } pm_message_t;

#define PM_EVENT_AUTO		0x0400
#define PM_EVENT_SUSPEND	0x0002
#define PM_EVENT_AUTO_SUSPEND	(PM_EVENT_AUTO | PM_EVENT_SUSPEND)
#define PM_EVENT_RESUME		0x0008
#define PM_EVENT_FREEZE		0x0001
#define PMSG_IS_AUTO(msg)	(((msg).event & PM_EVENT_AUTO) != 0)
#define PMSG_AUTO_SUSPEND	((struct pm_message){ .event = PM_EVENT_AUTO })
#define PMSG_SUSPEND		((struct pm_message){ .event = PM_EVENT_SUSPEND })
#define PMSG_RESUME		((struct pm_message){ .event = PM_EVENT_RESUME })
#define PMSG_FREEZE		((struct pm_message){ .event = PM_EVENT_FREEZE })

struct dev_pm_ops {
	int (*suspend)(struct device *dev);
	int (*resume)(struct device *dev);
	int (*runtime_suspend)(struct device *dev);
	int (*runtime_resume)(struct device *dev);
};

#define SIMPLE_DEV_PM_OPS(name, suspend_fn, resume_fn) \
	const struct dev_pm_ops name = { .suspend = suspend_fn, .resume = resume_fn }
#endif
