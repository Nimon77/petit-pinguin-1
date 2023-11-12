// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>

int do_work(int *my_int)
{
	int i;
	int y = *my_int;

	for (i = 0; i < y; ++i)
		usleep_range(10, 11);
	if (y < 10)
		pr_info("We slept a long time!");
	return i * y;
}

int my_init(void)
{
	int x = 10;

	return do_work(&x);
}

void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);
