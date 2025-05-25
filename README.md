# TP_LinuxEmbarqué
Dépôt TP Linux Embarqué 3DN

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

![image](https://github.com/user-attachments/assets/d4df5525-43b6-4a37-8947-cd27976f6200)

On définit le chemin du fichier avec un fpga_led%d, qui va permettre d'avoir le chemin de chaque LED. Dans la fonction set_led, on ouvre le fichier britghness du dossier de la ledX, que l'on choisit avec int led_number, puis on écrit 1 ou 0 pour allumer. Enfin dans le main on fait une boucle pour faire le chenillard.

# 2. Modules Kernel

## 2.1  Accès aux registres

mmap() permet de mapper une zone mémoire physique dans l'espace virtuel du processus utilisateur. Mais l'utilisation de mmap() peut causer des risques de sécurité, parce que l'on accède directement au matériel sans protection, de statibilité car une erreur peut bloquer le système, on a pas d'accès aux interruptions.

![image](https://github.com/user-attachments/assets/f3a63255-a588-49db-a36e-ea9aea82c7b3)

On crée un pointeur qui va acceuillir la base virtuelle mappée avec le chemin et l'addresse du GPIO, on obtient alors l'adresse des LEDS.

## 2.2 Compilation de module noyau sur la VM

Nous allons développer nos propres noyaux, et nous allons les tester premièrement sur la VM.
le_module simple:
 
![image](https://github.com/user-attachments/assets/742593a3-d78d-426d-967b-a9e10c1d1793)

module_param: 

![image](https://github.com/user-attachments/assets/11701256-e052-49bf-9278-3e7b641ce35e)

module_proc:

![image](https://github.com/user-attachments/assets/7bdee49d-b0c3-4815-9b90-f493723aa59f)
![image](https://github.com/user-attachments/assets/ab70a840-bb6b-43eb-a01c-c4dce8ac71ae)


module_timer:

![image](https://github.com/user-attachments/assets/ee43a44b-328a-4517-bb84-bddb9538c3b5)

## 2.3 CrossCompilation de modules noyau

module_timer sur FPGA :
![image](https://github.com/user-attachments/assets/61d65cda-1e5b-45ef-8a78-0e1a066ae794)


faire les modules
