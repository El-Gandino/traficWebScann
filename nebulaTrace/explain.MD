 # nebulaTrace
 Appplication serv ant a enregistrer tous ce qui ce passe sur le LAN et a le monitorer.
##  
NebulaTrace (C++) 
   ├── Enregistre en DB (live pour l'API)
   └── Archive en PCAP (pour Wireshark ou replays)
g++ src/main.cpp -o build/NebulaTrace



### tableau désitionnelle
| **Méthode** | **Points positifs** | **Points négatifs** | **Remarques spécifiques pour ton projet** |
|:------------|:--------------------|:--------------------|:-------------------------------------------|
| **Base de données** (MySQL, SQLite, etc.) | - Accès ultra rapide pour l'API<br>- Requêtes filtrées (IP, protocole, temps)<br>- Prêt pour *temps réel* et historique | - Complexité à gérer (indexation, nettoyage, volumétrie énorme)<br>- Peut ralentir la capture si mal optimisé<br>- Besoin de faire attention aux écritures concurrentes | 🔥 Idéal pour interaction Web (API REST) mais à condition de bufferiser ou batcher les insertions pour éviter d’étrangler le Raspberry. |
| **Fichiers JSON** (un gros fichier ou par paquet) | - Facile à lire/consommer<br>- Prêt pour API qui stream du JSON<br>- Simple à produire en C++ | - Risque d’avoir de très gros fichiers très vite<br>- Parsing coûteux en RAM<br>- Pas "queryable" facilement sans tout lire | ⚡ Bonne solution temporaire pour développement rapide, mais limité en scalabilité si tu veux capturer plusieurs heures/jours de trafic. |
| **Fichier PCAP** (format brut réseau) | - Standard réseau universel (Wireshark, tcpdump)<br>- Ultra efficace en écriture (format brut)<br>- Compact | - Difficilement lisible directement par une API<br>- Nécessite une étape de parsing pour extraire les infos utiles<br>- Pas idéal pour temps réel sans transformation | 🔍 Très bien pour archivage ou analyse "offline", mais pas directement exploitable pour ton objectif Web/API temps réel. |

| Base de données | Avantages | Inconvénients | Verdict |
|:---------------|:----------|:--------------|:--------|
| **SQLite** | - Ultra légère, fichier unique<br>- Aucune installation serveur<br>- Parfait pour petit à moyen trafic | - Pas fait pour du très gros volume<br>- Pas de vraie concurrence multi-accès lourd | ✅ **Parfait** pour débuter, simple, rapide |
| **MariaDB/MySQL** | - Plus robuste pour gros trafic<br>- Requêtes avancées possibles<br>- Bon support multi-connexion | - Plus lourd<br>- Nécessite un serveur qui tourne tout le temps | 🔥 **Idéal si tu vises un vrai environnement "prod"**, notamment avec beaucoup de requêtes/API |
| **PostgreSQL** | - Très puissant<br>- Gère super bien les gros datasets<br>- Transactions parfaites | - Très lourd pour un Raspberry Pi (par défaut)<br>- Besoin de config fine pour performances correctes | 🛑 Trop pour ton besoin actuel sur Pi sauf si tu optimises à fond |


### Fonctionnement discover Web
[Envoyer ARP Broadcast] -- toutes les 30 seconde (variable changeable dans le Main :D)
      ↓
[Lire /proc/net/arp]
      ↓
[Enregistrer IP/MAC dans la BDD]
      ↓
[Toutes 10 min : scanner IP en ping] -- controle les IP silencieux et complette la table.
      ↓
[Compléter / mettre à jour la BDD]


### liste de tache
1. créer un sécurity calss qui auras comme fonction de controler que l'app a les droit pour tout faire (paramaitre les droit pour ne pas avoir besoin de le lancer en root)