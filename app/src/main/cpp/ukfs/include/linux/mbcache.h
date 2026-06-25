#ifndef _UK_LINUX_MBCACHE_H
#define _UK_LINUX_MBCACHE_H
#include <linux/types.h>
#include <linux/list.h>
#include <linux/atomic.h>
struct mb_cache;
struct mb_cache_entry { struct list_head e_list; u32 e_key; u64 e_value; atomic_t e_refcnt; bool e_reusable; bool e_referenced; unsigned long e_flags; };
struct mb_cache *mb_cache_create(int bucket_bits);
void mb_cache_destroy(struct mb_cache *cache);
int mb_cache_entry_create(struct mb_cache *cache, gfp_t mask, u32 key, u64 value, bool reusable);
void mb_cache_entry_delete(struct mb_cache *cache, u32 key, u64 value);
struct mb_cache_entry *mb_cache_entry_get(struct mb_cache *cache, u32 key, u64 value);
struct mb_cache_entry *mb_cache_entry_find_first(struct mb_cache *cache, u32 key);
struct mb_cache_entry *mb_cache_entry_find_next(struct mb_cache *cache, struct mb_cache_entry *entry);
void mb_cache_entry_put(struct mb_cache *cache, struct mb_cache_entry *entry);
void mb_cache_entry_touch(struct mb_cache *cache, struct mb_cache_entry *entry);
#endif
