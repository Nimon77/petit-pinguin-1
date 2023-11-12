// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define LOGIN		"nsimon"
#define LOGIN_SIZE	6

MODULE_LICENSE("GPL");

static int	__init fortytwo_init(void);
static void	__exit fortytwo_exit(void);
static ssize_t	fortytwo_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t	fortytwo_write(struct file *filp, const char *buf, size_t len, loff_t *off);

static const struct file_operations fops = {
	.read		= fortytwo_read,
	.write		= fortytwo_write,
};

struct miscdevice fortytwo_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fortytwo",
	.fops = &fops,
};

static ssize_t fortytwo_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
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

static ssize_t fortytwo_write(struct file *filp, const char *buf, size_t len, loff_t *off)
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

static int __init fortytwo_init(void)
{
	int error;

	error = misc_register(&fortytwo_device);
	if (error) {
		pr_err("Unable to register fortytwo\n");
		return error;
	}

	pr_info("Kernel Module Inserted Successfully...\n");
	return 0;
}

static void __exit fortytwo_exit(void)
{
	misc_deregister(&fortytwo_device);
	pr_info("Kernel Module Removed Successfully...\n");
}

module_init(fortytwo_init);
module_exit(fortytwo_exit);

