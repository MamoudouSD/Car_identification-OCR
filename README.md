# 🚗 Car Plate Identification

Système embarqué temps-réel de **détection de plaques d'immatriculation** sur architecture ARM (aarch64). Le pipeline capture des images depuis deux caméras simultanément, exécute une inférence YOLO via **ExecuTorch + XNNPACK**, affiche les résultats sur un écran LCD et sauvegarde les détections sur disque — le tout orchestré par un séquenceur temps-réel multi-thread.

---

## 📋 Table des matières

- [Architecture du système](#architecture-du-système)
- [Analyse temps-réel — Rate Monotonic Analysis](#analyse-temps-réel--rate-monotonic-analysis)
- [Structure du projet](#structure-du-projet)
- [Prérequis](#prérequis)
- [Installation de l'environnement](#installation-de-lenvironnement)
  - [Mise à jour du système](#1-mise-à-jour-du-système)
  - [Installation de Miniconda](#2-installation-de-miniconda)
  - [Création de l'environnement Conda](#3-création-de-lenvironnement-conda)
  - [Installation et compilation d'OpenCV](#4-installation-et-compilation-dopencv)
  - [Installation de PyTorch et dépendances Python](#5-installation-de-pytorch-et-dépendances-python)
- [Installation d'ExecuTorch](#installation-dexecutorch)
  - [Cloner et initialiser](#1-cloner-et-initialiser)
  - [Build ExecuTorch](#2-build-executorch)
  - [Correction d'erreur connue — FlatCC](#correction-derreur-connue--flatcc)
- [Build du projet](#build-du-projet)
- [Utilisation](#utilisation)
- [Modèle IA](#modèle-ia)
- [Technologies utilisées](#technologies-utilisées)
- [Dépannage](#dépannage)
- [Licence](#licence)

---

## Architecture du système

Le système repose sur un pipeline **multi-thread** piloté par un séquenceur temps-réel (timer POSIX SIGALRM, tick à 5 ms). Chaque tâche s'exécute sur un cœur CPU dédié avec une priorité temps-réel `SCHED_FIFO`.

```
┌─────────────┐     ┌──────────────────────────────────────────────────────┐
│  Séquenceur │────▶│                  Threads (CPU affinity)              │
│  (5ms tick) │     ├──────────┬──────────┬────────────┬──────┬──────────┤
└─────────────┘     │ capture1 │ capture2 │ inference  │ save │ screen   │
                    │ (CPU 0)  │ (CPU 1)  │  (CPU 2)   │(CPU1)│ (CPU 0)  │
                    │  prio 80 │  prio 80 │  prio 90   │prio60│  prio 60 │
                    └────┬─────┴────┬─────┴─────┬──────┴──┬───┴──────────┘
                         │          │            │         │
                    ┌────▼──────────▼──┐    ┌────▼────┐  ┌▼───────────┐
                    │  img_toAi        │    │img_toDisp│  │ img_toSave │
                    │  (queue cap. 30) │    │(queue 30)│  │ (queue 50) │
                    └──────────────────┘    └──────────┘  └────────────┘
                              │
                    ┌─────────▼──────────────────────────────────┐
                    │  YOLOv8n (ExecuTorch .pte + XNNPACK)       │
                    │  score threshold: 0.25 | NMS: 0.6          │
                    │  input: 1x3xHxW (float32 normalized)       │
                    │  output: bboxes + scores (8400 anchors)    │
                    └────────────────────────────────────────────┘
```

**Cadences du séquenceur :**
- Toutes les **48 ticks (240 ms)** → capture caméra 1 + caméra 2
- Toutes les **35 ticks (175 ms)** → affichage écran + sauvegarde
- Toutes les **70 ticks (350 ms)** → inférence IA

---

## Analyse temps-réel — Rate Monotonic Analysis

Les priorités des tâches sont déterminées par **Rate Monotonic Analysis (RMA)** : plus la période est courte, plus la priorité est haute. Chaque tâche est épinglée sur un cœur dédié (`pthread_setaffinity_np`) avec une politique d'ordonnancement `SCHED_FIFO`.

**Affectation des tâches aux cœurs :**

| Tâche | Cœur CPU | Priorité SCHED_RR |
|-------|----------|-------------------|
| Cam1 (capture caméra 1) | Cœur 1 (CPU 0) | 80 |
| Display (affichage LCD) | Cœur 1 (CPU 0) | 60 |
| Cam2 (capture caméra 2) | Cœur 2 (CPU 1) | 80 |
| Save (sauvegarde) | Cœur 2 (CPU 1) | 60 |
| AI (inférence YOLO) | Cœur 3 (CPU 2) | 90 |
| Main / Séquenceur | Cœur 4 (CPU 3) | 89 |

---

### Tableaux RMA par cœur

Les colonnes sont : **P** = période (ms), **F** = fréquence (Hz), **So** = nombre d'occurrences par tick, **Priority** = priorité RMA (1 = plus haute), **C** = temps d'exécution (ms), **U%** = taux d'utilisation CPU.

> Le taux d'utilisation est calculé comme U = C / P × 100. La limite de faisabilité RMA pour n tâches est U ≤ n × (2^(1/n) − 1).

**Cœur 1 — Cam1 + Display** (U total = 27.14%, limite RMA 2 tâches ≈ 82.8% ✅)

| Tâche | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|-------|--------|--------|----|----------|--------|----|
| T1 — Capture Cam1 | 240 | 4.17 | 1.36 | **1** | 62 | 26% |
| T2 — Display | 175 | 5.71 | 1 | **2** | 2 | 1.14% |

**Cœur 2 — Cam2 + Save** (U total = 29.43%, limite RMA 2 tâches ≈ 82.8% ✅)

| Tâche | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|-------|--------|--------|----|----------|--------|----|
| T1 — Capture Cam2 | 240 | 4.17 | 1.36 | **1** | 62 | 26% |
| Tk — Save | 175 | 5.71 | 1 | **2** | 6 | 3.43% |

**Cœur 3 — Inférence IA** (U = 91.14%, limite RMA 1 tâche = 100% ✅)

| Tâche | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|-------|--------|--------|----|----------|--------|----|
| T2 — AI Inference | 350 | 2.86 | 1 | **1** | 319 | 91.14% |

> ⚠️ La tâche d'inférence est la plus coûteuse du système (319 ms sur 350 ms de période). Le cœur 3 est dédié exclusivement à cette tâche pour garantir son respect d'échéance.

**Cœur 4 — Séquenceur / Main** (U = 40%, limite RMA 1 tâche = 100% ✅)

| Tâche | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|-------|--------|--------|----|----------|--------|----|
| T2 — Séquenceur | 5 | 200 | 1 | **1** | 2 | 40% |

> Le séquenceur tourne à 200 Hz (tick 5 ms) et orchestre toutes les autres tâches via des sémaphores binaires POSIX.

---

## Structure du projet

```
Car_plate_identification/
├── AI/                                         # Modèles et données IA
│   ├── Data_exploration.ipynb                  # Notebook d'exploration du dataset
│   ├── yolov8n.pt                              # Modèle YOLOv8n de base
│   ├── yolov8n_finetune.pt                     # Modèle fine-tuné
│   ├── yolov8n_finetune_pruned.pt              # Modèle pruné (PyTorch)
│   ├── yolov8n_finetune_pruned.onnx            # Modèle pruné (ONNX)
│   ├── yolov8n_pruned_state_dict.pt            # State dict du modèle pruné
│   ├── model_quantified_executorch.pte         # Modèle final (ExecuTorch quantifié)
│   └── calibration_image_sample_data_*.npy    # Données de calibration quantification
│
├── app_code/                                   # Application C++ embarquée
│   ├── cMakeLists.txt                          # Configuration CMake
│   ├── Ai_file/
│   │   └── model_quantified_executorch.pte     # Modèle utilisé par l'app
│   ├── cpp_file/
│   │   ├── main.cpp                            # Point d'entrée
│   │   ├── System.cpp                          # Orchestration threads + séquenceur
│   │   ├── Yolo_infer.cpp                      # Inférence YOLOv8 via ExecuTorch
│   │   ├── Camera.cpp                          # Capture vidéo (OpenCV)
│   │   ├── Image.cpp                           # Traitement + sauvegarde images
│   │   ├── Display.cpp                         # Pilote LCD (GPIO / /dev/gpiomem)
│   │   └── Notification.cpp                    # Logging via syslog
│   └── hpp_file/
│       ├── Ai.hpp                              # Classe abstraite IA (template)
│       ├── Yolo_infer.hpp                      # Interface YOLO
│       ├── Camera.hpp                          # Interface caméra
│       ├── Image.hpp                           # Interface image
│       ├── Display.hpp                         # Interface affichage LCD
│       ├── System.hpp                          # Interface système
│       ├── Notification.hpp                    # Interface notification
│       └── ThreadSafe_queue.hpp                # File d'attente thread-safe (template)
│
└── LICENSE                                     # MIT License
```

---

## Prérequis

- **Architecture :** Linux aarch64 (ARM 64-bit — Raspberry Pi 4/5, Jetson, etc.)
- **OS :** Ubuntu 22.04+ recommandé
- **Matériel :**
  - 2 caméras USB (IDs 0 et 2 par défaut, MJPG, 640×480 @ 30fps)
  - Écran LCD connecté via GPIO (bus parallèle 16 bits, `/dev/gpiomem`)
- **Espace disque :** ~10 Go minimum
- **RAM :** 4 Go minimum
- **Droits :** accès à `/dev/gpiomem` pour le pilote LCD

---

## Installation de l'environnement

### 1. Mise à jour du système

```bash
sudo apt update
sudo apt upgrade
```

### 2. Installation de Miniconda

```bash
# Téléchargement de l'installeur ARM64
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-aarch64.sh

# Exécution de l'installeur (installer dans /home/<username>/)
bash ~/Miniconda3-latest-Linux-aarch64.sh

# Rechargement du shell
source ~/.bashrc
```

### 3. Création de l'environnement Conda

```bash
conda create --name cpd_env python=3.10.19
conda activate cpd_env
```

### 4. Installation et compilation d'OpenCV

> ⚠️ OpenCV doit être compilé depuis les sources sur aarch64 pour garantir la compatibilité.

```bash
# Outils de compilation
conda install gxx_linux-aarch64 cmake make

# Téléchargement et extraction des sources
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.x.zip
unzip opencv.zip
mv opencv-4.x opencv

# Compilation
mkdir -p build && cd build
cmake ../opencv
make -j4
```

### 5. Installation de PyTorch et dépendances Python

```bash
conda install conda-forge::pytorch
conda install pyyaml
```

---

## Installation d'ExecuTorch

### 1. Cloner et initialiser

```bash
git clone -b release/1.1 https://github.com/pytorch/executorch.git
cd executorch
conda activate cpd_env

# Initialisation de tous les sous-modules
git submodule update --init --recursive
```

### 2. Build ExecuTorch

```bash
cmake -B cmake-out --preset linux -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-out -j9
```

### Correction d'erreur connue — FlatCC

Lors du build, l'erreur suivante peut survenir :

```
executorch/third-party/flatcc/include/flatcc/portable/grisu3_print.h:186:33:
error: initializer-string for array of 'char' truncates NUL terminator
[-Werror=unterminated-string-initialization]
  186 |     static char hexdigits[16] = "0123456789ABCDEF";
```

**Cause :** le compilateur GCC ARM ajoute un terminateur nul `\0` à la chaîne, ce qui donne 17 caractères pour un tableau de 16 cases.

**Correction :** éditer le fichier `executorch/third-party/flatcc/include/flatcc/portable/grisu3_print.h` et remplacer la ligne 186 :

```c
// Avant (ligne fautive)
static char hexdigits[16] = "0123456789ABCDEF";

// Après (correction)
static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F'
};
```

Puis relancer le build :

```bash
cmake -B cmake-out --preset linux -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-out -j9
```

---

## Build du projet

Placer le dépôt ExecuTorch buildé dans `app_code/executorch/`, puis depuis `app_code/` :

```bash
cd app_code
mkdir build && cd build

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DEXECUTORCH_BUILD_XNNPACK=ON \
  -DEXECUTORCH_BUILD_EXTENSION_MODULE=ON \
  -DEXECUTORCH_BUILD_EXTENSION_DATA_LOADER=ON \
  -DEXECUTORCH_BUILD_KERNELS_QUANTIZED=ON \
  -DEXECUTORCH_BUILD_EXTENSION_FLAT_TENSOR=ON \
  -DEXECUTORCH_BUILD_EXTENSION_NAMED_DATA_MAP=ON \
  -DEXECUTORCH_BUILD_EXTENSION_TENSOR=ON

make -j4
```

**Description des flags CMake :**

| Flag | Description |
|------|-------------|
| `EXECUTORCH_BUILD_XNNPACK` | Backend CPU optimisé ARM (NEON/SIMD) |
| `EXECUTORCH_BUILD_EXTENSION_MODULE` | Module de chargement des modèles `.pte` |
| `EXECUTORCH_BUILD_EXTENSION_DATA_LOADER` | Chargement des poids du modèle |
| `EXECUTORCH_BUILD_KERNELS_QUANTIZED` | Kernels pour modèles quantifiés (int8) |
| `EXECUTORCH_BUILD_EXTENSION_FLAT_TENSOR` | Support du format FlatTensor |
| `EXECUTORCH_BUILD_EXTENSION_NAMED_DATA_MAP` | Accès nommé aux données du modèle |
| `EXECUTORCH_BUILD_EXTENSION_TENSOR` | Extension tenseur de base |

---

## Utilisation

```bash
# Lancer l'exécutable depuis le dossier build
./Car_Plate_identification
```

Au démarrage, le système :
1. Initialise le GPIO et l'écran LCD (séquence de couleurs de test)
2. Ouvre les deux caméras (IDs 2 et 0)
3. Charge le modèle ExecuTorch depuis `../Ai_file/model_quantified_executorch.pte`

Une fois l'initialisation réussie :

```
pour commencer cliquer sur s
```

Appuyer sur `s` puis `Entrée` pour démarrer le pipeline. Appuyer sur `Ctrl+C` pour arrêter proprement (le système vide les queues et sauvegarde les dernières images avant de quitter).

**Sorties générées :**

Les images et métadonnées sont sauvegardées dans un dossier `./data/<date>/` :
- `<date>.txt` — CSV avec colonnes `id ; nombre_de_plaques ; coordonnées_bbox`
- `<image_id>.jpeg` — image avec bounding boxes dessinées

Les logs système sont accessibles via syslog (identifiant `Plate_detection`) :

```bash
journalctl -t Plate_detection -f
```

---

## Modèle IA

Le pipeline IA suit les étapes suivantes :

```
YOLOv8n (base)
    │
    ▼
Fine-tuning sur dataset plaques  →  yolov8n_finetune.pt
    │
    ▼
Pruning du modèle  →  yolov8n_finetune_pruned.pt / .onnx
    │
    ▼
Quantification + export ExecuTorch  →  model_quantified_executorch.pte
    │
    ▼
Inférence embarquée via XNNPACK
```

**Paramètres d'inférence :**

| Paramètre | Valeur |
|-----------|--------|
| Score threshold | 0.25 |
| NMS threshold | 0.60 |
| Résolution entrée caméra | 640 × 480 px |
| Résolution affichage plaque | 240 × 320 px |
| Anchors traités | 8 400 |
| Format tenseur entrée | `[1, 3, H, W]` float32 normalisé [0, 1] |

---

## Technologies utilisées

| Technologie | Version | Rôle |
|-------------|---------|------|
| **C++** | C++20 | Application embarquée temps-réel |
| **Python** | 3.10.19 | Entraînement, pruning, export modèle |
| **YOLOv8n** | Ultralytics | Détection de plaques |
| **ExecuTorch** | release/1.1 | Runtime d'inférence embarqué |
| **XNNPACK** | (inclus ET) | Accélération CPU ARM (NEON/SIMD) |
| **OpenCV** | 4.x | Capture vidéo, traitement image, NMS |
| **PyTorch** | conda-forge | Fine-tuning et export |
| **POSIX threads** | — | Multi-threading temps-réel (SCHED_RR) |
| **syslog** | — | Journalisation système |
| **CMake** | ≥ 3.28 | Système de build |
| **Miniconda** | latest aarch64 | Gestion environnement Python |

---

## Dépannage

**Erreur `unterminated-string-initialization` dans FlatCC**

Voir la section [Correction d'erreur connue — FlatCC](#correction-derreur-connue--flatcc) ci-dessus.

**`conda activate` ne fonctionne pas dans un script bash**

```bash
source ~/.bashrc
conda activate cpd_env
```

**Erreur d'ouverture de caméra**

Vérifier les IDs disponibles :
```bash
ls /dev/video*
```
Les IDs 0 et 2 sont codés dans `System.cpp`. Les adapter si nécessaire.

**Accès refusé à `/dev/gpiomem`**

```bash
sudo usermod -aG gpio $USER
# puis se déconnecter/reconnecter
```

**Erreur de mémoire lors du `make`**

```bash
make -j2  # réduire le parallélisme
```

**CMake ne trouve pas ExecuTorch**

S'assurer que le dossier `executorch/` buildé est bien présent dans `app_code/` — le `cMakeLists.txt` l'intègre via `add_subdirectory(executorch EXCLUDE_FROM_ALL)`.

---

## Licence

Ce projet est distribué sous licence **MIT**. Voir le fichier [LICENSE](LICENSE) pour les détails.

Copyright (c) 2025 MamoudouSD
