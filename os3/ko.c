#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

int init_module(void) {
	pr_info("Welcome to Tomsk\n");
	return 0;
}
void cleanup_module(void) {
	pr_info("Unloding the TSU Linux Module\n");
}

module_init(tsulab_init);
module_exit(tsulab_exit);
MODULE_LICENSE("GPL");
