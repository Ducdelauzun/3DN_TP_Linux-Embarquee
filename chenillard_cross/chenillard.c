/*******************************************************
 * Fichier : chenillard.c
 * Rôle    : Chenillard circulaire avec motif dynamique binaire
 * Auteur  : Gad Katamba
 * Date    : 15 mai 2025
 ******************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gad Katamba");
MODULE_DESCRIPTION("Chenillard circulaire dynamique sur LEDs sysfs");
MODULE_VERSION("1.0");

// Paramètre vitesse en ms
static int vitesse = 500;
module_param(vitesse, int, S_IRUGO);
MODULE_PARM_DESC(vitesse, "Vitesse du chenillard en ms");

// LEDs et pattern
#define NUM_LEDS 10
#define MAX_PATTERN_LEN 32
static char pattern[MAX_PATTERN_LEN + 1] = "11";
static int pattern_len = 2;
static int current_index = 0;

static struct timer_list chenille_timer;

// /proc
#define PROC_DIR  "ensea"
#define PROC_FILE "chenille"
static struct proc_dir_entry *proc_dir, *proc_file;

// Fonction : écrire dans /sys/class/leds/fpga_led%d/brightness
#define LED_PATH_TEMPLATE "/sys/class/leds/fpga_led%d/brightness"

static void allumer_led(int led_num, int etat) {
    char path[64];
    struct file *f;
    mm_segment_t old_fs;
    char val = etat ? '1' : '0';

    if (led_num < 1 || led_num > 10) return;

    snprintf(path, sizeof(path), LED_PATH_TEMPLATE, led_num);
    old_fs = get_fs();
    set_fs(KERNEL_DS);

    f = filp_open(path, O_WRONLY, 0);
    if (!IS_ERR(f)) {
        kernel_write(f, &val, 1, 0);
        filp_close(f, NULL);
    }

    set_fs(old_fs);
}

static void clear_all_leds(void) {
    int i;
    for (i = 1; i <= NUM_LEDS; i++) {
        allumer_led(i, 0);
    }
}

// Fonction appelée à chaque tick du timer
static void chenille_callback(unsigned long data) {
    int i;
    clear_all_leds();

    for (i = 0; i < pattern_len; i++) {
        if (pattern[i] == '1') {
            int led_pos = (current_index + i) % NUM_LEDS;
            allumer_led(led_pos + 1, 1);  // LEDs de 1 à 10
        }
    }

    current_index = (current_index + 1) % NUM_LEDS;

    chenille_timer.expires = jiffies + msecs_to_jiffies(vitesse);
    add_timer(&chenille_timer);
}

// Lecture du fichier /proc/ensea/chenille
static ssize_t chenille_read(struct file *file, char __user *buffer, size_t count, loff_t *f_pos) {
    return simple_read_from_buffer(buffer, count, f_pos, pattern, pattern_len);
}

// Écriture dans /proc/ensea/chenille (binaire)
static ssize_t chenille_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos) {
    if (count > MAX_PATTERN_LEN)
        return -EINVAL;

    if (copy_from_user(pattern, buffer, count))
        return -EFAULT;

    pattern[count] = '\0';
    pattern_len = strnlen(pattern, MAX_PATTERN_LEN);
    current_index = 0;

    return count;
}

static const struct file_operations proc_fops = {
    .owner  = THIS_MODULE,
    .read   = chenille_read,
    .write  = chenille_write,
};

// Initialisation du module
static int __init chenillard_init(void) {
    printk(KERN_INFO "[chenillard] Initialisation avec vitesse = %d ms\n", vitesse);

    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir) return -ENOMEM;

    proc_file = proc_create(PROC_FILE, 0666, proc_dir, &proc_fops);
    if (!proc_file) {
        remove_proc_entry(PROC_DIR, NULL);
        return -ENOMEM;
    }

    init_timer(&chenille_timer);
    chenille_timer.function = chenille_callback;
    chenille_timer.expires = jiffies + msecs_to_jiffies(vitesse);
    add_timer(&chenille_timer);

    return 0;
}

// Nettoyage du module
static void __exit chenillard_exit(void) {
    del_timer_sync(&chenille_timer);
    clear_all_leds();
    remove_proc_entry(PROC_FILE, proc_dir);
    remove_proc_entry(PROC_DIR, NULL);
    printk(KERN_INFO "[chenillard] Module déchargé proprement.\n");
}

module_init(chenillard_init);
module_exit(chenillard_exit);
