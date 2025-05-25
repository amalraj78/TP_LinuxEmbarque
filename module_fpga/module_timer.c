#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>

static struct timer_list mon_timer;
static int compteur = 0;

void timer_callback(struct timer_list *t)
{
    compteur++;
    printk(KERN_INFO "Timer tick %d\n", compteur);
    mod_timer(&mon_timer, jiffies + msecs_to_jiffies(2000));
}

static int __init timer_init(void)
{
    printk(KERN_INFO "Module timer charge\n");
    
    init_timer(&mon_timer);
    mon_timer.function = timer_callback;
    mon_timer.data = 0;
    mon_timer.expires = jiffies + msecs_to_jiffies(2000);
    add_timer(&mon_timer);
    return 0;
}

static void __exit timer_exit(void)
{
    del_timer(&mon_timer);
    printk(KERN_INFO "Module timer décharge\n");
}

MODULE_LICENSE("GPL");
module_init(timer_init);
module_exit(timer_exit);
