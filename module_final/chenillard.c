/*******************************************************
 * Fichier : chenillard.c
 * Rôle    : Module noyau chenillard complet (procfs + /dev)
 * Auteur  : Gad Katamba
 * Date    : 15 mai 2025
 ******************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>

#define DEVICE_NAME "ensea-led"
#define PROC_DIR "ensea"
#define PROC_SPEED "speed"
#define PROC_DIR_FILE "dir"

static void __iomem *led_base = NULL;
static struct timer_list t;

static char pattern[32] = "11";
static int pattern_len = 2;
static int vitesse = 500;
static int direction = 1;
static int index_pattern = 0;

static struct proc_dir_entry *proc_dir;

/********************* LED access *********************/
#define REG_OFFSET 0x0
static void afficher_pattern(void) {
    int i;
    u32 val = 0;
    for (i = 0; i < pattern_len; i++) {
        if (pattern[i] == '1') {
            int pos = (index_pattern + i * direction + 32) % 10;
            val |= (1 << pos);
        }
    }
    iowrite32(val, led_base + REG_OFFSET);
}

/********************* Timer *********************/
static void timer_callback(unsigned long data) {
    afficher_pattern();
    index_pattern = (index_pattern + direction + 10) % 10;
    mod_timer(&t, jiffies + msecs_to_jiffies(vitesse));
}

/********************* /proc *********************/
static ssize_t read_speed(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char temp[16];
    int len = snprintf(temp, sizeof(temp), "%d\n", vitesse);
    return simple_read_from_buffer(buf, count, ppos, temp, len);
}

static ssize_t write_speed(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char temp[16];
    if (count >= sizeof(temp)) return -EINVAL;
    if (copy_from_user(temp, buf, count)) return -EFAULT;
    temp[count] = '\0';
    kstrtoint(temp, 10, &vitesse);
    return count;
}

static ssize_t read_dir(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char temp[8];
    int len = snprintf(temp, sizeof(temp), "%d\n", direction);
    return simple_read_from_buffer(buf, count, ppos, temp, len);
}

static ssize_t write_dir(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char temp[8];
    if (count >= sizeof(temp)) return -EINVAL;
    if (copy_from_user(temp, buf, count)) return -EFAULT;
    temp[count] = '\0';
    kstrtoint(temp, 10, &direction);
    return count;
}

static const struct file_operations proc_speed_ops = {
    .owner = THIS_MODULE,
    .read = read_speed,
    .write = write_speed,
};

static const struct file_operations proc_dir_ops = {
    .owner = THIS_MODULE,
    .read = read_dir,
    .write = write_dir,
};

/********************* /dev *********************/
static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    return simple_read_from_buffer(buf, count, ppos, pattern, pattern_len);
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    if (count > sizeof(pattern) - 1) return -EINVAL;
    if (copy_from_user(pattern, buf, count)) return -EFAULT;
    pattern[count] = '\0';
    pattern_len = strnlen(pattern, sizeof(pattern));
    index_pattern = 0;
    return count;
}

static const struct file_operations fops_led = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
};

static struct miscdevice led_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &fops_led,
};

/********************* Platform driver *********************/
static const struct of_device_id ensea_match[] = {
    { .compatible = "dev,ensea" },
    { }
};
MODULE_DEVICE_TABLE(of, ensea_match);

static int ensea_probe(struct platform_device *pdev) {
    struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    led_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(led_base)) return PTR_ERR(led_base);

    proc_dir = proc_mkdir(PROC_DIR, NULL);
    proc_create(PROC_SPEED, 0666, proc_dir, &proc_speed_ops);
    proc_create(PROC_DIR_FILE, 0666, proc_dir, &proc_dir_ops);

    misc_register(&led_miscdev);

    init_timer(&t);
    t.function = timer_callback;
    t.expires = jiffies + msecs_to_jiffies(vitesse);
    add_timer(&t);

    return 0;
}

static int ensea_remove(struct platform_device *pdev) {
    del_timer_sync(&t);
    misc_deregister(&led_miscdev);
    remove_proc_entry(PROC_SPEED, proc_dir);
    remove_proc_entry(PROC_DIR_FILE, proc_dir);
    remove_proc_entry(PROC_DIR, NULL);
    return 0;
}

static struct platform_driver ensea_driver = {
    .driver = {
        .name = "ensea-led",
        .of_match_table = ensea_match,
    },
    .probe = ensea_probe,
    .remove = ensea_remove,
};

module_platform_driver(ensea_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gad Katamba");
MODULE_DESCRIPTION("Chenillard dynamique avec /dev et /proc (VEEK, Linux 4.9)");
