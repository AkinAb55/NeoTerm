#ifndef _UK_LINUX_QUOTA_H
#define _UK_LINUX_QUOTA_H
#include <linux/types.h>
#include <linux/uidgid.h>
typedef long long qsize_t;
struct kqid { union { kuid_t uid; kgid_t gid; }; int type; };
#define USRQUOTA 0
#define GRPQUOTA 1
#define PRJQUOTA 2
#define MAXQUOTAS 3
struct dquot { int dummy; };
struct quota_info { int dummy; };
#endif
