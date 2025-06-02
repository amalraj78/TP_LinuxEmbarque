# TP_LinuxEmbarqué
Dépôt TP Linux Embarqué 3DN

## Binôme

- [@amalraj78](https://www.github.com/amalraj78)
- [@mariam-m95](https://www.github.com/mariam-m95)

### 1.3 Connexion au système
Taille occupée par l'image : 

![image](https://github.com/user-attachments/assets/f584fc46-c557-4bed-b30e-b67dec9b8056)


## 1.4 Découverte de la cible

### 1.4.1 Exploration des dossiers /sys/class et /proc
Sosus réportoire de la racine :
* /dev : fichiers de périphériques
* /proc : interface virtuelle exposant des infos sur le noyau et matériel
* /sys : infos sur les périphériques et configuration

Dans le dossier /proc, on retrouve :
* /cpuinfo : informations sur le CPU
* /ioports : plages I/O
* /iomem : plages mémoires 
* /device-tree/sopc@0 : structure matérielle déclarée, on peut comparer avec iomem pour observer les plages mémoire

Dans /sys/class/leds/gpga_ledX/, on retrouve des paramètres qu'on peut modifier, comme brightness.

### 1.4.3 Hello World :

On a compilé un ficher helloworld.c sur la VM, puis on envoie par réseau le fichier obtenu par compilation, et on l'exécute :

![image](https://github.com/user-attachments/assets/c88b0bd0-28c7-4806-a167-2eebaf4a623c)

### 1.4.4 Accès au matériael

En utilisant la ligne ci-dessous, on peut allumer la led 1 :

```bash
echo "1" > /sys/class/leds/fpga_led1/brightness
```

### 1.4.5 Chenillard

Nous allons faire un code en C qui va écrire dans le fichier /sys/class/leds/fpga_ledX/brightness pour faire un chenillard.

[chenillard.c](chenillard.c)

![image](https://github.com/user-attachments/assets/d4df5525-43b6-4a37-8947-cd27976f6200)

On définit le chemin du fichier avec un fpga_led%d, qui va permettre d'avoir le chemin de chaque LED. Dans la fonction set_led, on ouvre le fichier britghness du dossier de la ledX, que l'on choisit avec int led_number, puis on écrit 1 ou 0 pour allumer. Enfin dans le main on fait une boucle pour faire le chenillard.

# 2. Modules Kernel

## 2.1  Accès aux registres

mmap() permet de mapper une zone mémoire physique dans l'espace virtuel du processus utilisateur. Mais l'utilisation de mmap() peut causer des risques de sécurité, parce que l'on accède directement au matériel sans protection, de statibilité car une erreur peut bloquer le système, on a pas d'accès aux interruptions.

[chenillard_mmap.c](chenillard_mmap.c)

![image](https://github.com/user-attachments/assets/f3a63255-a588-49db-a36e-ea9aea82c7b3)

On crée un pointeur qui va acceuillir la base virtuelle mappée avec le chemin et l'addresse du GPIO, on obtient alors l'adresse des LEDS.

## 2.2 Compilation de module noyau sur la VM

Nous allons développer nos propres noyaux, et nous allons les tester premièrement sur la VM.

### le_module simple:
C'est un module simple qui afffiche un message quand il est inséré dans le noyau et un autre quand il sort :

[le_module.c](module/le_module.c)

![image](https://github.com/user-attachments/assets/742593a3-d78d-426d-967b-a9e10c1d1793)

### module_param: 
C'est un module qui prend peut prendre des paramètres grace à module_param. Lors de l'insertion on précise la valeur du paramètre, ou la valeur est définie par défaut dans le code C :

[module_param.c](module/module_param.c)

![image](https://github.com/user-attachments/assets/11701256-e052-49bf-9278-3e7b641ce35e)

### module_proc:
C'est un module qui crée un fichier dans le dossier /proc/monproc. Quand on lit ce fichier, il affiche un message.

[module_proc.c](module/module_proc.c)

![image](https://github.com/user-attachments/assets/7bdee49d-b0c3-4815-9b90-f493723aa59f)
![image](https://github.com/user-attachments/assets/ab70a840-bb6b-43eb-a01c-c4dce8ac71ae)


module_timer:
C'est un module qui affiche un message toutes les 2 secondes, avec la fonction timer_setup qui est une fonction qui crée une structure timer et qui va appeler la fonction timer_callback.

[module_timer.c](module/timer.c)

![image](https://github.com/user-attachments/assets/ee43a44b-328a-4517-bb84-bddb9538c3b5)

## 2.3 CrossCompilation de modules noyau

### 2.3.2 Récupérationde la configuration actuelle du noyau

On récupère la configuration du noyau dans /proc/config.gz
```
export CROSS_COMPILE=/chemin/vers/arm-linux-gnueabihf-
export ARCH=arm
```
Ces 2 lignes permettent d'indiquer au make quel compilateur nous allons utiliser. En laissant un tiret à la fin on récupère le arm-linux-gnueabihf-gcc, ld et as.

### 2.3.3 Hello World

On va compiler les modules avec le nouveau ficher makefile, et nous allons tester les modules sur la carte. Pour cela on crée un nouveau Makefile.
[Makefile](module_fpga/Makefile)

Hormis pour le module timer, les autres modules n'ont pas besoin de changement. 
Tous les modules fonctionnent très bien.

### 2.3.4 Chenillard

Nous allons faire un programme dont on peut modifier le pattern dans le fichier /proc/ensea/chenille, et dont on peut choisir la vitesse du chenillard au chargement du module. Nous allons juste commencer par observer si nous pouvons modifier le fichier et entrer un paramètre.
Voici le code :

[module_chenillard.c](module_fpga/module_chenillard.c)

Dans la fonction nous allons créer le chemin et le fichier, ensea/chenille et dans exit nous allons le supprimer.

![image](https://github.com/user-attachments/assets/70a7cb8a-b58d-41d3-89cd-f9f3d6252bca)

La fonction chenille_read va lire le message dans le fichier /proc/ensea/chenillard et l'afficher. La fonction chenille_write va écrire dans ce fichier, le pattern.

![image](https://github.com/user-attachments/assets/eda18c21-e54f-485f-933d-a9ce114a6903)


# 3.Device Tree

Nous allons définir notre propre périphérique et programmer notre propre module qui identifie la présence du périphérique et se configure automatiquement à sa présence. Cet automatisme s'appuie sur le Device Tree. Pour cela nous modifier dans le fichier soc_system.dts, l'entrée ledr en remplaçant l'adresse par ensea. On compile ce fichier et on remplace ce nouveau fichier dans le mntboot.

## 3.1 Module accèdant au LED via /dev
On étudie le code gpio-leds.c : 

* leds_probe : cette fonction est automatiquement appelée quand un périphérique compatible est trouvé. Elle va récupérer la ressource mémoire du périphérique. Elle va allouer une structure ensea_leds_dev pour stocker les infos et un pointeur. Elle va créer un fichier dans /dev/ensea_leds
* leds_read : cette fonction récupère la structure ensea_leds_dev et va la copier vers l'utilisateur
* leds_write : on écrit dans le fichier /dev/ensea_leds et on met à jour le registre matériel
* leds_remove : cette fonction est appelée lors du retrait du périphérique et supprime le fichier /dev/ensea_leds

## 3.2 Module final

Réaliser un chenillard qui rempli les conditions suivantes :
* Choix de la vitesse de balayage par une option au moment du chargement
du module
* Récupération de la vitesse courante par lecture du fichier /proc/ensea/speed
* Modification de la patern par écriture dans le fichier /dev/ensea-led
* Récupération du patern courant par lecture du fichier /dev/ensea-led
* Modification du sens de balayage par écriture du fichier /proc/ensea/dir
* Récupération du sens de balayage par lecture du fichier /proc/ensea/dir
* Nous allons utiliser un timer pour le chenillard.

Voici le code :

[chenillard_final.c](chenillard_final.c)

Ce module permet de modifier : 
* la vitesse via /proc/ensea/speed
* la direction via /proc/ensea/dir
* le pattern via /dev/ensea_leds

On réutilise des fonctions du code précédent, comme leds_probe pour détecter un périphérique. Le timer permet d'écrire le pattern de façon périodique dans son fichier respectif, et de modifier le timer en lisant la vitesse (speed). leds_read/write permettent d'écrire et de lire le pattern. On a aussi d'autres fonctions pour lire et écrire la vitesse et la direction

# Conclusion

Lors de ce TP, nous avons pu prendre en main la carte VEEK MT2S. Nous avons pu naviguer dans l'OS, dans ses fichiers sys, proc etc, qui contiennent de nombreuses informations sur le CPU, les périphériques, les mémoires et autres. Nous avons appliqué nos compétences en C et fait de la cross compilation. Nous avons pu aller encore plus loin en créant des modules noyaux, très performants. Nous n'avons pas pu tester le chenillard_final, mais nous avons réussi à effectuer toutes les autres manipulations.
