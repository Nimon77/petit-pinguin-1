// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/debugfs.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/types.h>

// https://cdn.intra.42.fr/pdf/pdf/62177/en.subject.pdf

#define LOGIN		"nsimon"
#define LOGIN_SIZE	6

MODULE_LICENSE("GPL");

static struct dentry *dir;
static char foo_value[PAGE_SIZE];

static DEFINE_MUTEX(foo_mutex);

static ssize_t	id_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t	id_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static ssize_t	foo_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t	foo_write(struct file *filp, const char *buf, size_t len, loff_t *off);

static const struct file_operations fops_id = {
	.read		= id_read,
	.write		= id_write,
};

static const struct file_operations fops_foo = {
	.read		= foo_read,
	.write		= foo_write,
};

static ssize_t id_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	ssize_t	tmp;

	if (len < LOGIN_SIZE)
		return -EINVAL;
	if (*off >= LOGIN_SIZE)
		return 0;
	if (len + *off >= LOGIN_SIZE)
		tmp = LOGIN_SIZE - *off;
	else
		tmp = len;
	tmp = tmp - copy_to_user(buf, LOGIN + *off, tmp);
	*off += tmp;
	return tmp;
}

static ssize_t id_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	char	data[LOGIN_SIZE];

	if (len != LOGIN_SIZE)
		return -EINVAL;
	if (copy_from_user(data, buf, LOGIN_SIZE))
		return -EFAULT;
	if (memcmp(data, LOGIN, LOGIN_SIZE))
		return -EINVAL;
	return len;
}

static ssize_t foo_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	mutex_lock(&foo_mutex);

	len = simple_read_from_buffer(buf, len, off, foo_value, strlen(foo_value));

	mutex_unlock(&foo_mutex);
	return len;
}

static ssize_t foo_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	mutex_lock(&foo_mutex);

	if (len >= PAGE_SIZE)
		return -EINVAL;

	len = simple_write_to_buffer(foo_value, PAGE_SIZE - 1, off, buf, len);
	foo_value[len] = '\0';

	mutex_unlock(&foo_mutex);
	return len;
}

int init_module(void)
{
	pr_debug("debugfs_fortytwo: Initializing module.\n");
	dir = debugfs_create_dir("fortytwo", 0);
	if (!dir) {
		pr_err("debugfs_fortytwo: failed to create /sys/kernel/debug/fortytwo\n");
		return -1;
	}

	if (!debugfs_create_file("id", 0666, dir, NULL, &fops_id)) {
		pr_err("debugfs_fortytwo: failed to create /sys/kernel/debug/fortytwo/id\n");
		return -1;
	}

	debugfs_create_u64("jiffies", 0444, dir, (u64 *)&jiffies);

	if (!debugfs_create_file("foo", 0644, dir, NULL, &fops_foo)) {
		pr_err("debugfs_fortytwo: failed to create /sys/kernel/debug/fortytwo/foo\n");
		return -1;
	}
	return 0;
}

void cleanup_module(void)
{
	pr_debug("debugfs_fortytwo: Cleaning up module.\n");
	debugfs_remove_recursive(dir);
}
