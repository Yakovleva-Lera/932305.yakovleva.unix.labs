#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/math64.h>

#define procfs_name "tsulab"
static struct proc_dir_entry *our_proc_file = NULL;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset) {
	static const s64 ref_timestamp = 1766620800LL;
	static const s64 ref_position_minutes = 16445LL;
	
	static const s64 synodic_period_seconds = (s64)(779.94 * 86400.0 + 0.5);
	static const s64 boundaries[13] = {0, 2012, 3468, 5592, 7092, 9118, 10889, 12572, 13831, 17850, 19560, 21582, 21600};
	
	static const char *const const_names[12] = {"Овен", "Телец", "Близнецы", "Рак", "Лев", "Дева", "Весы", "Скорпион", "Стрелец", "Козерог", "Водолей", "Рыбы"};
	time64_t current_time = ktime_get_real_seconds();
	s64 delta_t = (s64)current_time - ref_timestamp;
	s64 delta_minutes = div64_s64(delta_t * 21600LL, synodic_period_seconds);
	s64 current_minutes = ref_position_minutes + delta_minutes;
	
	current_minutes %= 21600;
	if (current_minutes < 0)
		current_minutes += 21600;
	
	int i;
	for (i = 0; i < 12; i++) {
		if (current_minutes >= boundaries[i] && current_minutes < boundaries[i+1])
			break;
       	}
	
	if (i == 12) {
		if (current_minutes < boundaries[1])
			i = 0;
		else
			i = 11;
	}
	
	int deg = (int)(current_minutes / 60);
	int min = (int)(current_minutes % 60);
	char output[128];
	int total_len = snprintf(output, sizeof(output), "Mars is at %d°%02d′ ecliptic longitude — in constellation %s\n", deg, min, const_names[i]);
	
	if (*offset >= total_len)
		return 0;
	
	int bytes_to_read = min((size_t)(total_len - (int)*offset), buffer_length);
	if (copy_to_user(buffer, output + *offset, bytes_to_read))
		return -EFAULT;
	
	*offset += bytes_to_read;
	return bytes_to_read;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
	.proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
	.read = procfile_read,
};
#endif

static int __init tsulab_init(void) {
	pr_info("Welcome to the Tomsk State University\n");
	our_proc_file = proc_create(procfs_name, 0444, NULL, &proc_file_fops);
	if (!our_proc_file) {
		pr_err("Failed to create /proc/%s\n", procfs_name);
		return -ENOMEM;
	}
	return 0;
}

static void __exit tsulab_exit(void) {
	proc_remove(our_proc_file);
	pr_info("Tomsk State University forever!\n");
}

MODULE_LICENSE("GPL");

module_init(tsulab_init);
module_exit(tsulab_exit);
