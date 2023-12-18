// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/nsproxy.h>
#include <linux/mnt_namespace.h>
#include <../fs/mount.h>
#include <linux/fs_struct.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");

char buffer[PAGE_SIZE];

static struct proc_dir_entry *ent;

static ssize_t	mymounts_read(struct file *filp, char __user *buf, size_t len, loff_t *off);

static const struct proc_ops fops = {
	.proc_read	= mymounts_read,
};

static char *get_dentry_path(struct dentry *dentry, char * const buffer, const int buflen)
{
	struct inode *inode = dentry->d_inode;
	char *pos = ZERO_SIZE_PTR;

	if (buflen >= 256) {
		pos = dentry_path_raw(dentry, buffer, buflen - 1);
		if (!IS_ERR(pos) && *pos == '/' && pos[1])
			if (inode && S_ISDIR(inode->i_mode))
				buffer[buflen - 1] = '\0';
	}
	return pos;
}

static ssize_t mymounts_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	if (*off != 0)
		return 0;
	memset(buffer, 0, PAGE_SIZE);

	struct mnt_namespace *ns = current->nsproxy->mnt_ns;
	struct rb_node *node = ZERO_SIZE_PTR;
	struct mount *mnt = ZERO_SIZE_PTR;
	char **path = ZERO_SIZE_PTR;
	char pathbuf[256];
	int pathlen;

	buffer[0] = '\0';
	for (node = rb_first(&ns->mounts); node; node = rb_next(node)) {
		mnt = rb_entry(node, struct mount, mnt_node);
		if (!strcat(buffer, mnt->mnt_devname))
			return -EFAULT;
		if (!strcat(buffer, "\t"))
			return -EFAULT;
		path = kmalloc(sizeof(char *), GFP_KERNEL);
		if (!path)
			return -EFAULT;
		*path = kstrdup(get_dentry_path(mnt->mnt_mountpoint, pathbuf, 256), GFP_KERNEL);
		if (!*path)
			return -EFAULT;
		pathlen = 1;
		while (mnt->mnt_parent != mnt) {
			mnt = mnt->mnt_parent;
			pathlen++;
			path = krealloc(path, sizeof(char *) * pathlen, GFP_KERNEL);
			if (!path)
				return -EFAULT;
			*(path + pathlen - 1) = kstrdup(get_dentry_path(mnt->mnt_mountpoint,
				pathbuf, 256), GFP_KERNEL);
			if (!*(path + pathlen - 1))
				return -EFAULT;
		}
		for (int i = pathlen - 1; i >= 0; i--) {
			if (strncmp(*(path + i), "/", 2) == 0)
				continue;
			if (!strcat(buffer, *(path + i)))
				return -EFAULT;
		}
		if (buffer[strlen(buffer) - 1] == '\t')
			if (!strcat(buffer, "/"))
				return -EFAULT;
		if (!strcat(buffer, "\n"))
			return -EFAULT;
	}

	return simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
}

static int __init mymounts_init(void)
{
	ent = proc_create("mymounts", 0444, NULL, &fops);
	if (!ent) {
		pr_err("mymounts: Unable to register mymounts\n");
		return -ENOMEM;
	}

	pr_info("mymounts: Kernel Module Inserted Successfully...\n");
	return 0;
}

static void __exit mymounts_exit(void)
{
	proc_remove(ent);
	pr_info("mymounts: Kernel Module Removed Successfully...\n");
}

module_init(mymounts_init);
module_exit(mymounts_exit);
