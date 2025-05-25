#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define PROC_DIR "ensea"
#define PROC_FILE "chenillard"
#define MESSAGE "Hello depuis /proc/ensea/chenillard\n"
#define PATTERN_MAX_LEN 10 

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;

static char pattern[PATTERN_MAX_LEN] = "0000000000";
static int vitesse = 1000;
module_param(vitesse, int, 0);
MODULE_PARM_DESC(param, "Parametre de vitesse du chenillard");


ssize_t chenille_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char message[128];
    int len;

    len = snprintf(message, sizeof(message), "Hello depuis /proc/ensea/chenillard.\r\nPattern = %s .\r\nVitesse= %d\r\n", pattern, vitesse);

    if (*ppos > 0)
        return 0;

    if (copy_to_user(buf, message, len))
        return -EFAULT;

    *ppos = len;
    return len;
}

ssize_t chenille_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (count > PATTERN_MAX_LEN - 1)
        count = PATTERN_MAX_LEN - 1;

    if (copy_from_user(pattern, buf, count))
        return -EFAULT;

    pattern[count] = '\0'; // Null-terminer la chaîne

    printk(KERN_INFO "Pattern modifié : %s\n", pattern);

    return count;
}

static struct file_operations chenille_fops = {
    .owner = THIS_MODULE,
    .read = chenille_read,
    .write = chenille_write,
};

static int __init chenillard_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
        return -ENOMEM;

    proc_file = proc_create(PROC_FILE, 0666, proc_dir, &chenille_fops);
    if (!proc_file) {
        remove_proc_entry(PROC_DIR, NULL);
        return -ENOMEM;
    }

    printk(KERN_INFO "/proc/%s/%s cree. Vitesse=%d ms, Pattern=%s\n", PROC_DIR, PROC_FILE, vitesse, pattern);
    return 0;
}

static void __exit chenillard_exit(void)
{
    remove_proc_entry(PROC_FILE, proc_dir);
    remove_proc_entry(PROC_DIR, NULL);
    printk(KERN_INFO "/proc/%s/%s supprime\n", PROC_DIR, PROC_FILE);
}





MODULE_LICENSE("GPL");
module_init(chenillard_init);
module_exit(chenillard_exit);
