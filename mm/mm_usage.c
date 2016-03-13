#include <linux/mm_usage.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/mutex.h>

extern mm_usage_t __start_mm_usage[];
extern mm_usage_t __stop_mm_usage[];

LIST_HEAD(module_mm_usage_list);
DEFINE_MUTEX(mm_usage_mutex);

#define ENTRIES (__stop_mm_usage - __start_mm_usage)

void mm_usage_list_add(mm_usage_t *mm_usage)
{
	mutex_lock(&mm_usage_mutex);
	list_add(&(mm_usage->list), &module_mm_usage_list);
	mutex_unlock(&mm_usage_mutex);
}

void mm_usage_list_del(mm_usage_t *mm_usage)
{
	mutex_lock(&mm_usage_mutex);
	list_del(&(mm_usage->list));
	mutex_unlock(&mm_usage_mutex);
}

static void *s_start(struct seq_file *f, loff_t *pos)
{
	return seq_list_start(&module_mm_usage_list, *pos);
}

static void *s_next(struct seq_file *f, void *data, loff_t *pos)
{
	return seq_list_next(data, &module_mm_usage_list, pos);
}

static void s_stop(struct seq_file *f, void *data)
{
}

static int s_show(struct seq_file *f, void *data)
{
	mm_usage_t *entry = container_of(data, mm_usage_t, list);
	seq_printf(f, "%s\t\t\t%u\n", entry->name, entry->size);
	return 0;
}

static const struct seq_operations mm_usage_seq_ops = {
	.start	= s_start,
	.next	= s_next,
	.stop	= s_stop,
	.show	= s_show,
};

static int mm_usage_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &mm_usage_seq_ops);
}

static const struct file_operations mm_usage_fops = {
	.open		= mm_usage_open,
	.release	= seq_release,
	.read		= seq_read,
	.llseek		= seq_lseek,
};

static int __init mm_usage_init(void)
{
	int i;

	if (!proc_create("mm_usage", 0644, NULL, &mm_usage_fops))
		return -ENOMEM;

	for (i = 0; i < ENTRIES; i++) {
		mm_usage_list_add(&__start_mm_usage[i]);
	}

	return 0;
}

module_init(mm_usage_init);
