/*******************************************************
 * Fichier : hello_module.c
 * Rôle    : Exemple de module noyau avec paramètre
 * Auteur  : Gad Katamba
 * Date    : 15 mai 2025
 ******************************************************/

 #include <linux/module.h>
 #include <linux/kernel.h>
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Gad Katamba");
 MODULE_DESCRIPTION("Un module simple avec paramètre.");
 MODULE_VERSION("1.0");
 
 // Paramètre avec valeur par défaut
 static int valeur = 42;
 module_param(valeur, int, S_IRUGO); // lecture autorisée par l'utilisateur
 MODULE_PARM_DESC(valeur, "Une valeur entière passée au module");
 
 static int __init hello_init(void) {
     printk(KERN_INFO "Hello_module chargé avec valeur = %d\n", valeur);
     return 0;
 }
 
 static void __exit hello_exit(void) {
     printk(KERN_INFO "Hello_module déchargé\n");
 }
 
 module_init(hello_init);
 module_exit(hello_exit);
 