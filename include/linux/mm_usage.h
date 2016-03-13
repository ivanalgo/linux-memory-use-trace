#ifndef MM_USAGE_H
#define MM_USAGE_H

#include <linux/mm_types.h>
#include <linux/list.h>

typedef struct mm_usage {
	char *name;
	size_t size;
	/*
	 * For modules, this structure will deallocated when the Module
	 * was rmmod, but its allocated memory is using by others.
	 * alloc a new mm_usage for each Module's mm_usage structure,
	 * and referenced by module_ptr.
	 */
	struct mm_usage *module_ptr;	
	struct list_head list;
} mm_usage_t;

static mm_usage_t mm_usage __attribute__((section("__mm_usage"))) = {
	.name = KBUILD_MODNAME,
	.size = 0,
};

#ifdef MODULES
#define THIS_MM_USAGE (mm_usage.module_ptr)
#else
#define THIS_MM_USAGE (&mm_usage)
#endif

static inline void mm_usage_inc(struct page *page, int order)
{
	THIS_MM_USAGE->size += 1 << order;
	page->mm_usage = THIS_MM_USAGE;
}

static inline void mm_usage_dec(struct page *page, int order)
{
	mm_usage_t *this_usage = page->mm_usage;

	if (!this_usage)
		return;

	this_usage->size -= 1 << order;
	page->mm_usage = NULL;
}

extern void mm_usage_list_add(mm_usage_t *mm_usage);
extern void mm_usage_list_del(mm_usage_t *mm_usage);

#endif /* MM_USAGE_H */
