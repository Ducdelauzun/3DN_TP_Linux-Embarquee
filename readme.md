# 📘 TP Linux Embarqué – Réponses jusqu'à la partie 3

## ✅ 1.4.4 Accès au matériel

Les LEDs sont accessibles via des fichiers dans `/sys/class/leds/`. Par exemple :

```bash
echo 1 > /sys/class/leds/fpga_led1/brightness
```

> Cela active la LED numéro 1. Le fichier `brightness` sert à allumer (1) ou éteindre (0) la LED.

---

## ✅ 1.4.5 Chenillard

Au lieu de passer par `echo`, un programme en C peut ouvrir les fichiers de chaque LED et y écrire `"1"` ou `"0"` pour les allumer/éteindre.

Idée générale :
- Créer un tableau de chemins `/sys/class/leds/fpga_ledX/brightness`
- Allumer une LED à la fois et faire défiler vers la droite, puis vers la gauche
- Utiliser `usleep()` pour la temporisation

---

## ✅ 2.1 Accès aux registres

> **Adresse des LEDs :** `0xFF203000`

Pour accéder à un registre depuis l’espace utilisateur, on doit utiliser `mmap()` pour convertir l’adresse physique en adresse virtuelle, car la mémoire est virtualisée.

### ❗ Problèmes et limites de cette méthode :
- Accès non sécurisé aux registres (aucune isolation)
- Risques d’erreurs critiques si mauvaise adresse
- Incompatible avec les protections classiques du noyau
- Moins pérenne qu’un pilote bien intégré dans le noyau

---

## ✅ 2.3.2 – Rôle des commandes `export`

```bash
export CROSS_COMPILE=/usr/bin/arm-linux-gnueabihf-
export ARCH=arm
```

### 🔹 `CROSS_COMPILE`
Indique le **préfixe du compilateur croisé** à utiliser, par exemple :
`arm-linux-gnueabihf-gcc` = `/usr/bin/arm-linux-gnueabihf-` + `gcc`

### 🔹 `ARCH`
Spécifie l'architecture cible (ici `arm`) pour que le noyau sache générer des modules pour l’architecture correcte.

---

## ✅ Pourquoi le chemin finit par un `-` ?

Parce que ce préfixe sera concaténé automatiquement avec les outils nécessaires (`gcc`, `as`, `ld`, etc.). C’est une **convention de nommage** des toolchains de cross-compilation.

> Exemple : `/usr/bin/arm-linux-gnueabihf-gcc` ← on garde tout sauf le `gcc` final pour CROSS_COMPILE.

---

## ✅ 2.3.4 Chenillard (module noyau)

Le module demandé doit :

- Utiliser un `timer` du noyau
- Offrir un `fichier /proc/ensea/chenille` pour modifier le **pattern**
- Accepter un **paramètre au chargement** (`insmod chenillard.ko vitesse=100`)

💡 À ce stade, **les LEDs ne sont pas encore pilotées**, on vérifie les logs avec `dmesg`.

---

## ✅ 3 Device Tree

### 🎯 Objectif :
Accéder aux LED directement, ce qui nécessite **d’éviter les pilotes Altera** déjà présents.

### 🔧 Solution retenue :
Modifier le fichier `soc_system.dts` :
```dts
ledr: ensea {
    compatible = "dev,ensea";
    reg = <0x00000001 0x00003000 0x00000010>;
    clocks = <&clk_50>;
};
```

> Cela **supprime l’usage du driver par défaut** et permet à ton module d’accéder aux registres directement.

---

## ✅ 3.1 Module via `/dev`

### Fonctions importantes du module :

- **probe** : Appelée quand le périphérique est détecté
- **read** : Lecture du périphérique (par l’utilisateur)
- **write** : Écriture sur le périphérique
- **remove** : Nettoyage à la suppression du module