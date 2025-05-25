#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_NAME "monproc"
#define MESSAGE "Hello depuis /proc/monproc\n"

static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    static int finished = 0;
    if (finished) {
        finished = 0;
        return 0;
    }

    finished = 1;

    if (copy_to_user(buf, MESSAGE, sizeof(MESSAGE))) {
        return -EFAULT;
    }

    return sizeof(MESSAGE) - 1;
}

static struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .read = proc_read,
};

static int __init proc_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &proc_fops);
    printk(KERN_INFO "/proc/%s créé\n", PROC_NAME);
    return 0;
}

static void __exit proc_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s supprimé\n", PROC_NAME);
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_exit);
