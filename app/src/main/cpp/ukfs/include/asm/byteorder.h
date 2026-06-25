#ifndef _UK_ASM_BYTEORDER_H
#define _UK_ASM_BYTEORDER_H
#include <linux/types.h>
/* A glibc <endian.h> definiálja MIND a __LITTLE_ENDIAN MIND a __BIG_ENDIAN makrót
 * (értékként, 1234/4321). A kernel-kód viszont `#ifdef __BIG_ENDIAN`-t használ "ez a
 * gép big-endian" értelemben (kernel-konvenció: csak az aktuális arch van definiálva).
 * A host little-endian → a __BIG_ENDIAN-t le KELL szedni, különben az ntfs3 a hibás
 * big-endian run_unpack_s64-et fordítja és minden data-run-érték elromlik. */
#include <endian.h>
#undef __BIG_ENDIAN
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif
/* a host little-endian (aarch64/x86); a leképezés azonosság + bswap a be-hez */
#define __bswap16(x) ((u16)((((x)&0xff)<<8)|(((x)>>8)&0xff)))
#define __bswap32(x) ((u32)((((x)&0xffUL)<<24)|(((x)&0xff00UL)<<8)|(((x)>>8)&0xff00UL)|(((x)>>24)&0xffUL)))
/* MAKRÓK (nem inline fv), hogy KONSTANS kifejezések legyenek — az ntfs3 enum-okban
 * cpu_to_le32(...)-t használ. Little-endian → identitás. */
#define le16_to_cpu(v)  ((u16)(v))
#define le32_to_cpu(v)  ((u32)(v))
#define le64_to_cpu(v)  ((u64)(v))
#define cpu_to_le16(v)  ((__le16)(u16)(v))
#define cpu_to_le32(v)  ((__le32)(u32)(v))
#define cpu_to_le64(v)  ((__le64)(u64)(v))
#define be16_to_cpu(v)  ((u16)__bswap16(v))
#define be32_to_cpu(v)  ((u32)__bswap32(v))
#define cpu_to_be16(v)  ((__be16)__bswap16(v))
#define cpu_to_be32(v)  ((__be32)__bswap32(v))
#define ___constant_swab32 __bswap32

/* swab — sima byte-swap helperek */
static inline u16 swab16(u16 v){ return __bswap16(v); }
static inline u32 swab32(u32 v){ return __bswap32(v); }

/* __ aláhúzásos variánsok (a kernel ezeket is használja, host little-endian → le=azonosság) */
#define __cpu_to_le16(v) ((__le16)(u16)(v))
#define __cpu_to_le32(v) ((__le32)(u32)(v))
#define __cpu_to_le64(v) ((__le64)(u64)(v))
#define __le16_to_cpu(v) ((u16)(v))
#define __le32_to_cpu(v) ((u32)(v))
#define __le64_to_cpu(v) ((u64)(v))
static inline __be16 __cpu_to_be16(u16 v){ return __bswap16(v); }
static inline __be32 __cpu_to_be32(u32 v){ return __bswap32(v); }
static inline u16 __be16_to_cpu(__be16 v){ return __bswap16(v); }
static inline u32 __be32_to_cpu(__be32 v){ return __bswap32(v); }

/* pointer-variánsok (*p): a mutatott helyről olvasnak/írnak */
static inline u16 le16_to_cpup(const __le16 *p){ return *p; }
static inline u32 le32_to_cpup(const __le32 *p){ return *p; }
static inline u16 __le16_to_cpup(const __le16 *p){ return *p; }
static inline u32 __le32_to_cpup(const __le32 *p){ return *p; }
static inline u16 be16_to_cpup(const __be16 *p){ return __bswap16(*p); }
static inline u32 be32_to_cpup(const __be32 *p){ return __bswap32(*p); }

/* in-place (*s) variánsok: a mutatott értéket a helyén konvertálják */
static inline void le16_to_cpus(__le16 *p){ (void)p; }
static inline void le32_to_cpus(__le32 *p){ (void)p; }
static inline void le64_to_cpus(__le64 *p){ (void)p; }
static inline u64 le64_to_cpup(const __le64 *p){ return *p; }
static inline u64 __le64_to_cpup(const __le64 *p){ return *p; }
static inline __le16 cpu_to_le16p(const u16 *p){ return *p; }
static inline __le32 cpu_to_le32p(const u32 *p){ return *p; }
static inline void cpu_to_le16s(u16 *p){ (void)p; }
static inline void cpu_to_le32s(u32 *p){ (void)p; }
static inline void be16_to_cpus(__be16 *p){ *p = __bswap16(*p); }
static inline void be32_to_cpus(__be32 *p){ *p = __bswap32(*p); }
static inline void cpu_to_be16s(u16 *p){ *p = __bswap16(*p); }
static inline void cpu_to_be32s(u32 *p){ *p = __bswap32(*p); }
#endif
