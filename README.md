# traficWebScann
Dévollopement de A-Z d'iun systeme sur raspi pour scanner le trafique reseaux
## Liste des application  a installer



## NebulaTrace 
Application de Mapping du réseaux
### Intro
Application développer en C++, qui a pour objectif de enregistrer tout ce qui se passe sur le LAN
### Choix du language
🧰 Tableau comparatif des langages pour un agent de surveillance réseau

| Critère         | Go (Golang)     | Python          | Rust            | C / C++         |
|:---------------:|:---------------:|:---------------:|:---------------:|:---------------:|
| Performance     | ⚡ Très bonne (compilé, goroutines légères) | 🟡 Moyenne (interprété, GC) | ⚡⚡ Excellente (contrôle bas niveau) | ⚡⚡ Excellente (contrôle bas niveau) |
| Sécurité mémoire | 🟡 Bonne (GC, mais pas de vérification stricte) | 🔴 Faible (GC, pas de vérification stricte) | 🟢 Très bonne (système de possession strict) | 🟡 Moyenne (pas de GC, gestion manuelle) |
| Facilité de dev  | 🟢 Très bonne (syntaxe simple, outils intégrés) | 🟢 Très bonne (syntaxe simple, riche écosystème) | 🟡 Moyenne (courbe d'apprentissage plus raide) | 🔴 Faible (syntaxe complexe, gestion manuelle) |
| Concurrence      | 🟢 Excellente (goroutines, channels) | 🟡 Bonne (bibliothèques tierces) | 🟡 Bonne (async/await, mais plus complexe) | 🟡 Bonne (threads, mais gestion manuelle) |
| Écosystème réseau | 🟢 Très riche (Prometheus, gopacket, etc.) | 🟢 Très riche (scapy, psutil, etc.) | 🟡 En croissance (tokio, mio, etc.) | 🟡 Bon (libpcap, pcapplusplus, etc.) |
| Déploiement | 🟢 Facile (binaire statique) | 🟡 Moyen (dépendances, interpréteur) | 🟡 Moyen (binaire statique, mais plus complexe) | 🟡 Moyen (dépendances, compilation) |
| Communauté | 🟢 Très active (support, documentation) | 🟢 Très active (support, documentation) | 🟡 Croissante (communauté passionnée) | 🟡 Active (mais plus orientée bas niveau) |

Avec ce tableau le choix ce porte entre Rust et C++
Vu que l'objecti et de ce formé le cchois se porte sur **C++**.
### Liste des information a enregistrer 
<!-- A mêtre sous forme de shéma (un jour) -->
