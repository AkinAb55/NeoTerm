#ifndef _UK_LINUX_ACPI_H
#define _UK_LINUX_ACPI_H
#include <linux/types.h>

typedef u32 acpi_status;
typedef void *acpi_handle;
typedef u32 acpi_object_type;
typedef u64 acpi_size;
#define AE_OK			0
#define ACPI_SUCCESS(s)		((s) == AE_OK)
#define ACPI_FAILURE(s)		((s) != AE_OK)
#define ACPI_ALLOCATE_BUFFER	((size_t)-1)
#define ACPI_TYPE_INTEGER	0x01
#define ACPI_TYPE_STRING	0x02
#define ACPI_TYPE_BUFFER	0x03
#define ACPI_TYPE_PACKAGE	0x04
#define ACPI_HANDLE(dev)	NULL

struct acpi_buffer { size_t length; void *pointer; };

union acpi_object {
	u32 type;
	struct { u32 type; u64 value; } integer;
	struct { u32 type; u32 length; char *pointer; } string;
	struct { u32 type; u32 length; u8 *pointer; } buffer;
	struct { u32 type; u32 count; union acpi_object *elements; } package;
};

struct acpi_object_list { u32 count; union acpi_object *pointer; };

static inline acpi_status acpi_evaluate_object(acpi_handle h, char *path,
		struct acpi_object_list *params, struct acpi_buffer *ret)
{ (void)h;(void)path;(void)params; if (ret) { ret->length = 0; ret->pointer = 0; } return ~0u; }

struct acpi_device;
static inline int acpi_dev_get_property(void *adev, const char *name, int type, void **obj) { (void)adev;(void)name;(void)type;(void)obj; return -1; }
#endif
