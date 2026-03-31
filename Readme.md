# Protocole de Transfert de Fichiers Multi-Serveurs (FTP) en C

**Université Grenoble Alpes – Licence 3 Informatique**  

Ce projet implémente un système FTP concurrent similaire, robuste et en C, avec une **architecture Master-Slave** pour l’équilibrage de charge. Il prend en charge les connexions persistantes, les transferts par blocs, les téléchargements reprenables et la gestion simultanée de plusieurs clients.

---

## Présentation du projet

Le système est composé d’un **serveur Master** qui agit comme un répartiteur Round-Robin et d’une pool de **serveurs Slaves** qui gèrent les transferts de fichiers. Cette architecture garantit évolutivité et fiabilité pour plusieurs clients simultanés.  

Objectifs principaux :  

- Transferts de fichiers efficaces pour de gros fichiers (jusqu’à plusieurs Go)  
- Support de la concurrence avec une pool de processus  
- Sessions persistantes avec reprise des transferts interrompus  
- Liste et récupération de fichiers à distance  

---

## Principales fonctionnalités

- **Architecture :** Le serveur Master distribue les requêtes clients aux serveurs Slaves en utilisant un algorithme Round-Robin.  
- **Concurrence :** Les serveurs Slaves utilisent une **pool de processus** (`NPROC`) pour gérer plusieurs clients simultanément.  
- **E/S robustes :** Les fichiers sont transférés par **blocs de 512 octets** pour optimiser l’utilisation mémoire et supporter de gros fichiers.  
- **Téléchargements reprenables :** Les clients peuvent reprendre un transfert interrompu ; le serveur envoie uniquement la partie manquante.  
- **Connexions persistantes :** Les clients peuvent envoyer plusieurs commandes (`GET`, `LS`) dans une même session jusqu’à ce qu’ils envoient la commande `BYE`.  

---

## Commandes supportées

| Commande | Description | Détails |
|---------|-------------|---------|
| `GET <nom_fichier>` | Télécharger un fichier depuis le serveur | Supporte les fichiers binaires/textes et la reprise via offsets |
| `LS <chemin>` | Lister les fichiers d’un répertoire serveur | Implémenté avec `popen` et des pipes internes |
| `BYE` | Fermer la connexion proprement | Termine la session avec le serveur Slave |

---

## État de l’implémentation

| Étape | Description |
|------|-------------|
| Étape I (Q1-7) | Structure FTP de base, terminaison propre du serveur, en-têtes requêtes/réponses, gestion des répertoires |
| Étape II (Q8-10) | Plusieurs demandes de fichiers par connexion, transferts par blocs, sessions persistantes, reprise après crash |
| Étape III (Q11-13) | Interconnexion Master-Slaves et redirection Round-Robin |
| Étape IV (Q15) | Liste des répertoires à distance (`LS`) |

> **Remarque :** La question 14 (redirection en cas de Slave défaillant) n’est pas incluse.  

---

## Structure des fichiers

- `masterserver.c` – Point d’entrée pour les clients ; attribue les serveurs Slaves.  
- `slavepool.c` – Serveur worker qui gère une pool de processus pour traiter les clients.  
- `file_transfer.c` – Logique principale pour les commandes `GET` et `LS`.  
- `client.c` – Interface client interactive.  
- `csapp.c` / `csapp.h` – Wrappers robustes pour appels système (Bryant & O’Hallaron).  
- `Makefile` – Script de compilation pour tous les composants.  

---

## Compilation et exécution

Le serveur Master écoute sur le port 2121 avec `NB_SLAVES` pour se connecter aux Slaves.  
Les serveurs Slaves commencent au port 5000 défini par `PORT_SLAVES`.  
Les serveurs Slaves créent `NB_PROC` processus pour gérer les clients.

```bash
# Compiler le projet
make clean
make

# Démarrer les serveurs Slaves (dans des terminaux séparés)
./slavepool 5000
...
./slavepool 5000 + NB_SLAVES - 1

# Démarrer le serveur Master sur le port 2121
./masterserver

# Lancer le client et se connecter au port 2121
./client <ip_serveur>

## Résultats des tests

- Fichiers standards: Transfert réussi de plusieurs fichiers `.txt` de tailles différentes.  
- Fichiers binaires: Intégrité vérifiée pour des images `.jpg` et des documents `.pdf`.  
- Gros fichiers: Transfert réussi d’un fichier texte de 1 Go avec blocs de 512 octets sans problème mémoire.  

## Bugs connus / Limitations

- Les commandes `PUT` et `RM` ne sont pas implémentées et ne donnent aucun retour lorsqu’elles sont utilisées.  
- Entrer une chaîne vide directement dans le client ne fonctionne pas et peut provoquer un comportement inattendu.  