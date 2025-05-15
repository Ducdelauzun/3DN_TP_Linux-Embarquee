/*******************************************************
 * Fichier : hello_advanced.c
 * Rôle    : Module noyau avec paramètre, /proc et timer
 * Auteur  : Gad Katamba
 * Date    : 15 mai 2025
 ******************************************************/

 #include <linux/init.h>
 #include <linux/module.h>
 #include <linux/kernel.h>
 #include <linux/proc_fs.h>
 #include <linux/seq_file.h>
 #include <linux/timer.h>
 #include <linux/jiffies.h>
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Gad Katamba");
 MODULE_DESCRIPTION("Module avec paramètre, entrée /proc et timer");
 MODULE_VERSION("1.0");
 
 // ---------- Paramètre utilisateur ----------
 static int valeur = 42;
 module_param(valeur, int, S_IRUGO);
 MODULE_PARM_DESC(valeur, "Un entier passé comme paramètre au module");
 
 // ---------- Timer ----------
 static struct timer_list my_timer;
 
 static void timer_callback(struct timer_list *t) {
     printk(KERN_INFO "[hello_advanced] Timer déclenché, valeur = %d\n", valeur);
     mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
 }
 
 // ---------- /proc ----------
 #define PROC_NAME "hello_info"
 
 static int proc_show(struct seq_file *m, void *v) {
     seq_printf(m, "Hello depuis /proc/hello_info ! Valeur = %d\n", valeur);
     return 0;
 }
 
 static int proc_open(struct inode *inode, struct file *file) {
     return single_open(file, proc_show, NULL);
 }
 
 // ✅ Version compatible noyaux < 5.6
 static const struct file_operations proc_fops = {
     .owner   = THIS_MODULE,
     .open    = proc_open,
     .read    = seq_read,
     .llseek  = seq_lseek,
     .release = single_release
 };
 
 // ---------- Fonctions init/exit ----------
 static int __init hello_init(void) {
     proc_create(PROC_NAME, 0, NULL, &proc_fops);
 
     timer_setup(&my_timer, timer_callback, 0);
     mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));
 
     printk(KERN_INFO "[hello_advanced] Module chargé avec valeur = %d\n", valeur);
     return 0;
 }
 
 static void __exit hello_exit(void) {
     remove_proc_entry(PROC_NAME, NULL);
     del_timer_sync(&my_timer);
     printk(KERN_INFO "[hello_advanced] Module déchargé.\n");
 }
 
 module_init(hello_init);
 module_exit(hello_exit); 