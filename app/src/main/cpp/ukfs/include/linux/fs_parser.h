/* uKernel hamis <linux/fs_parser.h> — mount-paraméter parser felülete. */
#ifndef _UK_LINUX_FS_PARSER_H
#define _UK_LINUX_FS_PARSER_H
#include <linux/types.h>
#include <linux/uidgid.h>   /* kuid_t/kgid_t a fs_parse_result-ban */

struct fs_context;
struct fs_parameter;

enum fs_value_type {
	fs_value_is_undefined, fs_value_is_flag, fs_value_is_string,
	fs_value_is_blob, fs_value_is_filename, fs_value_is_file,
};

struct fs_parameter {
	const char *key;
	enum fs_value_type type;
	char *string;
	void *blob;
	void *name;
	int dirfd;
	size_t size;
};

struct fs_parse_result {
	bool negated;
	union {
		bool boolean;
		int int_32;
		unsigned int uint_32;
		u64 uint_64;
		kuid_t uid;
		kgid_t gid;
	};
	unsigned int uint_32_high;
};

typedef int fs_param_type(struct fs_context *, struct fs_parameter *, struct fs_parse_result *);

struct fs_parameter_spec {
	const char	*name;
	fs_param_type	*type;
	u8		opt;
	unsigned short	flags;
#define fs_param_neg_with_no	0x0002
#define fs_param_can_be_empty	0x0004
#define fs_param_deprecated	0x0008
	const void	*data;
};

int fs_parse(struct fs_context *fc, const struct fs_parameter_spec *desc,
	     struct fs_parameter *param, struct fs_parse_result *result);
int fs_lookup_param(struct fs_context *fc, struct fs_parameter *param,
		    bool want_bdev, unsigned int flags, struct path *_path);

extern fs_param_type fs_param_is_bool, fs_param_is_u32, fs_param_is_s32, fs_param_is_u64,
	fs_param_is_string, fs_param_is_enum, fs_param_is_uid, fs_param_is_gid, fs_param_is_path;

#define __fsparam(TYPE, NAME, OPT, FLAGS, DATA) \
	{ .name = NAME, .type = TYPE, .opt = OPT, .flags = FLAGS, .data = DATA }
#define fsparam_flag(NAME, OPT)    __fsparam(NULL, NAME, OPT, 0, NULL)
#define fsparam_flag_no(NAME, OPT) __fsparam(NULL, NAME, OPT, fs_param_neg_with_no, NULL)
#define fsparam_bool(NAME, OPT)    __fsparam(fs_param_is_bool, NAME, OPT, 0, NULL)
#define fsparam_u32(NAME, OPT)     __fsparam(fs_param_is_u32, NAME, OPT, 0, NULL)
#define fsparam_u32oct(NAME, OPT)  __fsparam(fs_param_is_u32, NAME, OPT, 0, (void *)8)
#define fsparam_s32(NAME, OPT)     __fsparam(fs_param_is_s32, NAME, OPT, 0, NULL)
#define fsparam_string(NAME, OPT)  __fsparam(fs_param_is_string, NAME, OPT, 0, NULL)
#define fsparam_enum(NAME, OPT, A) __fsparam(fs_param_is_enum, NAME, OPT, 0, A)
#define fsparam_uid(NAME, OPT)     __fsparam(fs_param_is_uid, NAME, OPT, 0, NULL)
#define fsparam_gid(NAME, OPT)     __fsparam(fs_param_is_gid, NAME, OPT, 0, NULL)

extern fs_param_type fs_param_is_blockdev;
#define fsparam_bdev(NAME, OPT) __fsparam(fs_param_is_blockdev, NAME, OPT, 0, NULL)
#define fsparam_string_empty(NAME, OPT) __fsparam(fs_param_is_string, NAME, OPT, fs_param_can_be_empty, NULL)
struct constant_table { const char *name; int value; };
#endif
