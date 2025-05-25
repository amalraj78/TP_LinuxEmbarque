#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>

static struct timer_list mon_timer;
static int compteur = 0;

void timer_callback(struct timer_list *t)
{
    compteur++;
    printk(KERN_INFO "Timer tick %d\n", compteur);
    // Relance le timer toutes les 2 secondes
    mod_timer(&mon_timer, jiffies + msecs_to_jiffies(2000));
}

static int __init timer_init(void)
{
    printk(KERN_INFO "Module timer chargé\n");
    timer_setup(&mon_timer, timer_callback, 0);
    mod_timer(&mon_timer, jiffies + msecs_to_jiffies(2000));
    return 0;
}

static void __exit timer_exit(void)
{
    del_timer(&mon_timer);
    printk(KERN_INFO "Module timer déchargé\n");
}

MODULE_LICENSE("GPL");
module_init(timer_init);
module_exit(timer_exit);
