#ifndef _UK_LINUX_CIRC_BUF_H
#define _UK_LINUX_CIRC_BUF_H
struct circ_buf { char *buf; int head; int tail; };
#define CIRC_CNT(head,tail,size) (((head)-(tail))&((size)-1))
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))
#endif
