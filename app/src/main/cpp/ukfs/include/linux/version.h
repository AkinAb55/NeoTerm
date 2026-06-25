#ifndef _UK_LINUX_VERSION_H
#define _UK_LINUX_VERSION_H
#define KERNEL_VERSION(a,b,c) (((a)<<16)+((b)<<8)+((c)>255?255:(c)))
#define LINUX_VERSION_CODE KERNEL_VERSION(6,6,0)
#endif
