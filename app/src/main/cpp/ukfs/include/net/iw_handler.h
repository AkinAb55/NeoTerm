#ifndef _UK_NET_IW_HANDLER_H
#define _UK_NET_IW_HANDLER_H
#include <linux/wireless.h>
#include <linux/netdevice.h>
struct iw_handler_def;
struct iw_request_info { __u16 cmd; __u16 flags; };
typedef int (*iw_handler)(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
struct iw_handler_def {
	const iw_handler *standard; __u16 num_standard;
	const iw_handler *private; __u16 num_private;
	const struct iw_priv_args *private_args; __u16 num_private_args;
	struct iw_statistics *(*get_wireless_stats)(struct net_device *dev);
};
static inline char *iwe_stream_add_event(struct iw_request_info *info, char *stream, char *ends, struct iw_event *iwe, int len){ (void)info;(void)ends;(void)iwe;(void)len; return stream+len; }
static inline char *iwe_stream_add_point(struct iw_request_info *info, char *stream, char *ends, struct iw_event *iwe, char *extra){ (void)info;(void)ends;(void)iwe;(void)extra; return stream; }
static inline char *iwe_stream_add_value(struct iw_request_info *info, char *event, char *stream, char *ends, struct iw_event *iwe, int len){ (void)info;(void)event;(void)ends;(void)iwe;(void)len; return stream+len; }
static inline void wireless_send_event(struct net_device *d, unsigned cmd, union iwreq_data *w, char *e){ (void)d;(void)cmd;(void)w;(void)e; }
#endif
