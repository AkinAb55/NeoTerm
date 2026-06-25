/* uKernel hamis <linux/tracepoint.h> — a TRACE_EVENT-eket no-op trace_*-okká alakítja. */
#ifndef _UK_LINUX_TRACEPOINT_H
#define _UK_LINUX_TRACEPOINT_H
#include <linux/types.h>
#define TRACE_DEFINE_ENUM(x)
#define TRACE_DEFINE_SIZEOF(x)
#define DECLARE_TRACE_CONDITION(name, proto, args, cond) static inline void trace_##name(proto) {}
#define TP_PROTO(args...) args
#define TP_ARGS(args...) args
#define TP_CONDITION(args...) args
#define TP_STRUCT__entry(args...)
#define TP_fast_assign(args...)
#define TP_printk(args...)
#define TP_perf_assign(args...)
#define TRACE_EVENT(name, proto, args, ...) \
	static inline void trace_##name(proto) {} \
	static inline bool trace_##name##_enabled(void) { return false; }
#define TRACE_EVENT_FN(name, proto, args, ...) TRACE_EVENT(name, TP_PROTO(proto), TP_ARGS(args))
#define TRACE_EVENT_CONDITION(name, proto, args, ...) TRACE_EVENT(name, TP_PROTO(proto), TP_ARGS(args))
#define DECLARE_EVENT_CLASS(name, proto, args, ...)
#define DEFINE_EVENT(tmpl, name, proto, args, ...) \
	static inline void trace_##name(proto) {} \
	static inline bool trace_##name##_enabled(void) { return false; }
#define DEFINE_EVENT_PRINT(tmpl, name, proto, args, ...) DEFINE_EVENT(tmpl, name, TP_PROTO(proto), TP_ARGS(args))
#define DEFINE_EVENT_CONDITION(tmpl, name, proto, args, ...) DEFINE_EVENT(tmpl, name, TP_PROTO(proto), TP_ARGS(args))
#define TRACE_EVENT_FLAGS(name, value)
#define DECLARE_TRACE(name, proto, args) static inline void trace_##name(proto) {}
#define EXPORT_TRACEPOINT_SYMBOL(name)
#define EXPORT_TRACEPOINT_SYMBOL_GPL(name)
#define DEFINE_TRACE(name)
/* az entry-mező makrók — a fast_assign-on belül no-op */
#define __field(type, item)
#define __field_ext(type, item, ft)
#define __array(type, item, len)
#define __dynamic_array(type, item, len)
#define __string(item, src)
#define __assign_str(dst, src)
#define __entry ((void *)0)
#define __get_str(field) ""
#define __get_dynamic_array(field) NULL
#endif
