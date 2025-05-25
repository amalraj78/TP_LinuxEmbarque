#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define MODULE_NAME "chenillard"
#define PROC_DIR "ensea"
#define PROC_SPEED "speed"
#define PROC_DIR_FILE "dir"

static struct timer_list my_timer;
static struct ensea_leds_dev *led_dev;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_speed;
static struct proc_dir_entry *proc_direction;

static int speed = 500; // ms, modifiable à l'insertion
module_param(speed, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(speed, "Speed in milliseconds");

static int direction = 1; // 1 = gauche vers droite, -1 = droite vers gauche
static u8 pattern = 0x01;

// LED device
struct ensea_leds_dev {
    struct miscdevice miscdev;
    void __iomem *regs;
};

// Prototypes
static ssize_t leds_read(struct file *file, char __user *buffer, size_t len, loff_t *offset);
static ssize_t leds_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset);

static const struct file_operations ensea_leds_fops = {
    .owner = THIS_MODULE,
    .read = leds_read,
    .write = leds_write
};

// Platform driver
// Called whenever the kernel finds a new device that our driver can handle
// (In our case, this should only get called for the one instantiation of the Ensea LEDs module)
static int leds_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;
    struct ensea_leds_dev *dev;
    struct resource *r = 0;

    pr_info("leds_probe enter\n");

    // Get the memory resources for this LED device
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(r == NULL) {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }

    // Create structure to hold device-specific information (like the registers)
    dev = devm_kzalloc(&pdev->dev, sizeof(struct ensea_leds_dev), GFP_KERNEL);

    // Both request and ioremap a memory region
    // This makes sure nobody else can grab this memory region
    // as well as moving it into our address space so we can actually use it
    dev->regs = devm_ioremap_resource(&pdev->dev, r);
    if(IS_ERR(dev->regs))
        goto bad_ioremap;

    // Turn the LEDs on (access the 0th register in the ensea LEDs module)
    dev->leds_value = 0xFF;
    iowrite32(dev->leds_value, dev->regs);

    // Initialize the misc device (this is used to create a character file in userspace)
    dev->miscdev.minor = MISC_DYNAMIC_MINOR;    // Dynamically choose a minor number
    dev->miscdev.name = "ensea_leds";
    dev->miscdev.fops = &ensea_leds_fops;

    ret_val = misc_register(&dev->miscdev);
    if(ret_val != 0) {
        pr_info("Couldn't register misc device :(");
        goto bad_exit_return;
    }

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void*)dev);

    pr_info("leds_probe exit\n");

    return 0;

static int leds_remove(struct platform_device *pdev)
{
    struct ensea_leds_dev *dev = platform_get_drvdata(pdev);
    iowrite32(0, dev->regs);
    misc_deregister(&dev->miscdev);
    return 0;
}

static const struct of_device_id ensea_leds_ids[] = {
    { .compatible = "dev,ensea" },
    { }
};
MODULE_DEVICE_TABLE(of, ensea_leds_ids);

static struct platform_driver leds_platform = {
    .driver = {
        .name = MODULE_NAME,
        .of_match_table = ensea_leds_ids
    },
    .probe = leds_probe,
    .remove = leds_remove
};

// Timer callback
static void timer_callback(struct timer_list *timer)
{
    if (!led_dev) return;

    iowrite32(pattern, led_dev->regs);

    if (direction == 1)
        pattern = (pattern << 1) ? (pattern << 1) : 0x01;
    else
        pattern = (pattern >> 1) ? (pattern >> 1) : 0x80;

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(speed));
}

// /dev/ensea-led read
static ssize_t leds_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    return copy_to_user(buffer, &pattern, 1) ? -EFAULT : 1;
}

// /dev/ensea-led write
static ssize_t leds_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    if (copy_from_user(&pattern, buffer, 1))
        return -EFAULT;

    return 1;
}

// /proc/ensea/speed read
static ssize_t proc_speed_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char buffer[16];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", speed);
    return simple_read_from_buffer(buf, count, ppos, buffer, len);
}

// /proc/ensea/speed write
static ssize_t proc_speed_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char buffer[16];
    if (copy_from_user(buffer, buf, min(count, sizeof(buffer) - 1)))
        return -EFAULT;
    buffer[min(count, sizeof(buffer) - 1)] = '\0';
    kstrtoint(buffer, 10, &speed);
    return count;
}

// /proc/ensea/dir read
static ssize_t proc_dir_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char buffer[8];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", direction);
    return simple_read_from_buffer(buf, count, ppos, buffer, len);
}

// /proc/ensea/dir write
static ssize_t proc_dir_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char buffer[8];
    if (copy_from_user(buffer, buf, min(count, sizeof(buffer) - 1)))
        return -EFAULT;
    buffer[min(count, sizeof(buffer) - 1)] = '\0';
    kstrtoint(buffer, 10, &direction);
    if (direction != 1 && direction != -1)
        direction = 1; // Valeur par défaut
    return count;
}

static const struct proc_ops speed_ops = {
    .proc_read = proc_speed_read,
    .proc_write = proc_speed_write
};

static const struct proc_ops dir_ops = {
    .proc_read = proc_dir_read,
    .proc_write = proc_dir_write
};

static int __init chenillard_init(void)
{
    int ret;

    printk(KERN_INFO "Chenillard module init\n");

    ret = platform_driver_register(&leds_platform);
    if (ret)
        return ret;

    // /proc entries
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) goto error;

    proc_speed = proc_create(PROC_SPEED, 0666, proc_dir, &speed_ops);
    proc_direction = proc_create(PROC_DIR_FILE, 0666, proc_dir, &dir_ops);

    // Timer
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(speed));

    return 0;

error:
    platform_driver_unregister(&leds_platform);
    return -ENOMEM;
}

static void __exit chenillard_exit(void)
{
    del_timer_sync(&my_timer);
    if (proc_speed) remove_proc_entry(PROC_SPEED, proc_dir);
    if (proc_direction) remove_proc_entry(PROC_DIR_FILE, proc_dir);
    if (proc_dir) remove_proc_entry(PROC_DIR, NULL);

    platform_driver_unregister(&leds_platform);

    printk(KERN_INFO "Chenillard module exit\n");
}

module_init(chenillard_init);
module_exit(chenillard_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ENSEA");
MODULE_DESCRIPTION("Chenillard avec timer, /proc et /dev interface");
