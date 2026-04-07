# 🚗 Car Plate Identification

Real-time embedded system for **license plate detection** on ARM architecture (aarch64). The pipeline captures images from two cameras simultaneously, runs YOLO inference via **ExecuTorch + XNNPACK**, displays results on an LCD screen and saves detections to disk — all orchestrated by a real-time multi-thread sequencer.

---

## 📋 Table of Contents

- [System Architecture](#system-architecture)
- [Real-Time Analysis — Rate Monotonic Analysis](#real-time-analysis--rate-monotonic-analysis)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Environment Setup](#environment-setup)
  - [System Update](#1-system-update)
  - [Miniconda Installation](#2-miniconda-installation)
  - [Conda Environment Creation](#3-conda-environment-creation)
  - [OpenCV Build from Source](#4-opencv-build-from-source)
  - [PyTorch and Python Dependencies](#5-pytorch-and-python-dependencies)
- [ExecuTorch Installation](#executorch-installation)
  - [Clone and Initialize](#1-clone-and-initialize)
  - [Build ExecuTorch](#2-build-executorch)
  - [Known Build Error — FlatCC](#known-build-error--flatcc)
- [Project Build](#project-build)
- [Usage](#usage)
- [AI Model](#ai-model)
- [Technologies](#technologies)
- [Troubleshooting](#troubleshooting)
- [License](#license)

---

## System Architecture

The system relies on a **multi-threaded** pipeline driven by a real-time sequencer (POSIX SIGALRM timer, 5 ms tick). Each task runs on a dedicated CPU core with `SCHED_FIFO` real-time scheduling policy.

```
┌─────────────┐     ┌──────────────────────────────────────────────────────┐
│  Sequencer  │────▶│                  Threads (CPU affinity)              │
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

**Sequencer cadences:**
- Every **48 ticks (240 ms)** → camera 1 + camera 2 capture
- Every **35 ticks (175 ms)** → screen display + save
- Every **70 ticks (350 ms)** → AI inference

---

## Real-Time Analysis — Rate Monotonic Analysis

Task priorities are determined using **Rate Monotonic Analysis (RMA)**: the shorter the period, the higher the priority. Each task is pinned to a dedicated CPU core (`pthread_setaffinity_np`) with the `SCHED_FIFO` scheduling policy.

**Task-to-core assignment:**

| Task | CPU Core | SCHED_FIFO Priority |
|------|----------|---------------------|
| Cam1 (camera 1 capture) | Core 1 (CPU 0) | 80 |
| Display (LCD) | Core 1 (CPU 0) | 60 |
| Cam2 (camera 2 capture) | Core 2 (CPU 1) | 80 |
| Save (disk write) | Core 2 (CPU 1) | 60 |
| AI (YOLO inference) | Core 3 (CPU 2) | 90 |
| Main / Sequencer | Core 4 (CPU 3) | 89 |

---

### RMA Tables per Core

Column definitions: **P** = period (ms), **F** = frequency (Hz), **So** = occurrences per sequencer tick, **Priority** = RMA priority (1 = highest), **C** = execution time (ms), **U%** = CPU utilization.

> Utilization is computed as U = C / P × 100. The RMA schedulability bound for n tasks is U ≤ n × (2^(1/n) − 1).

**Core 1 — Cam1 + Display** (total U = 27.14%, RMA bound for 2 tasks ≈ 82.8% ✅)

| Task | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|------|--------|--------|----|----------|--------|----|
| T1 — Capture Cam1 | 240 | 4.17 | 1.36 | **1** | 62 | 26% |
| T2 — Display | 175 | 5.71 | 1 | **2** | 2 | 1.14% |

**Core 2 — Cam2 + Save** (total U = 29.43%, RMA bound for 2 tasks ≈ 82.8% ✅)

| Task | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|------|--------|--------|----|----------|--------|----|
| T1 — Capture Cam2 | 240 | 4.17 | 1.36 | **1** | 62 | 26% |
| Tk — Save | 175 | 5.71 | 1 | **2** | 6 | 3.43% |

**Core 3 — AI Inference** (U = 91.14%, RMA bound for 1 task = 100% ✅)

| Task | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|------|--------|--------|----|----------|--------|----|
| T2 — AI Inference | 350 | 2.86 | 1 | **1** | 319 | 91.14% |

> ⚠️ The inference task is the most compute-intensive in the system (319 ms out of a 350 ms period). Core 3 is exclusively dedicated to this task to guarantee deadline compliance.

**Core 4 — Sequencer / Main** (U = 40%, RMA bound for 1 task = 100% ✅)

| Task | P (ms) | F (Hz) | So | Priority | C (ms) | U% |
|------|--------|--------|----|----------|--------|----|
| T2 — Sequencer | 5 | 200 | 1 | **1** | 2 | 40% |

> The sequencer runs at 200 Hz (5 ms tick) and orchestrates all other tasks via POSIX binary semaphores.

---

## Project Structure

```
Car_plate_identification/
├── AI/                                         # AI models and data
│   ├── Data_exploration.ipynb                  # Dataset exploration notebook
│   ├── yolov8n.pt                              # Base YOLOv8n model
│   ├── yolov8n_finetune.pt                     # Fine-tuned model
│   ├── yolov8n_finetune_pruned.pt              # Pruned model (PyTorch)
│   ├── yolov8n_finetune_pruned.onnx            # Pruned model (ONNX)
│   ├── yolov8n_pruned_state_dict.pt            # Pruned model state dict
│   ├── model_quantified_executorch.pte         # Final model (ExecuTorch quantized)
│   └── calibration_image_sample_data_*.npy    # Quantization calibration data
│
├── app_code/                                   # Embedded C++ application
│   ├── cMakeLists.txt                          # CMake configuration
│   ├── Ai_file/
│   │   └── model_quantified_executorch.pte     # Model used by the app
│   ├── cpp_file/
│   │   ├── main.cpp                            # Entry point
│   │   ├── System.cpp                          # Thread orchestration + sequencer
│   │   ├── Yolo_infer.cpp                      # YOLOv8 inference via ExecuTorch
│   │   ├── Camera.cpp                          # Video capture (OpenCV)
│   │   ├── Image.cpp                           # Image processing + saving
│   │   ├── Display.cpp                         # LCD driver (GPIO / /dev/gpiomem)
│   │   └── Notification.cpp                    # Logging via syslog
│   └── hpp_file/
│       ├── Ai.hpp                              # Abstract AI base class (template)
│       ├── Yolo_infer.hpp                      # YOLO interface
│       ├── Camera.hpp                          # Camera interface
│       ├── Image.hpp                           # Image interface
│       ├── Display.hpp                         # LCD display interface
│       ├── System.hpp                          # System interface
│       ├── Notification.hpp                    # Notification interface
│       └── ThreadSafe_queue.hpp                # Thread-safe queue (template)
│
└── LICENSE                                     # MIT License
```

---

## Prerequisites

- **Architecture:** Linux aarch64 (ARM 64-bit — Raspberry Pi 4/5, Jetson, etc.)
- **OS:** Ubuntu 22.04+ recommended
- **Hardware:**
  - 2 USB cameras (IDs 0 and 2 by default, MJPG, 640×480 @ 30fps)
  - LCD screen connected via GPIO (16-bit parallel bus, `/dev/gpiomem`)
- **Disk space:** ~10 GB minimum
- **RAM:** 4 GB minimum
- **Permissions:** access to `/dev/gpiomem` for the LCD driver

---

## Environment Setup

### 1. System Update

```bash
sudo apt update
sudo apt upgrade
```

### 2. Miniconda Installation

```bash
# Download the ARM64 installer
curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-aarch64.sh

# Run the installer (install in /home/<username>/)
bash ~/Miniconda3-latest-Linux-aarch64.sh

# Reload shell
source ~/.bashrc
```

### 3. Conda Environment Creation

```bash
conda create --name cpd_env python=3.10.19
conda activate cpd_env
```

### 4. OpenCV Build from Source

> ⚠️ OpenCV must be compiled from source on aarch64 to ensure compatibility.

```bash
# Build tools
conda install gxx_linux-aarch64 cmake make

# Download and extract sources
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.x.zip
unzip opencv.zip
mv opencv-4.x opencv

# Build
mkdir -p build && cd build
cmake ../opencv
make -j4
```

### 5. PyTorch and Python Dependencies

```bash
conda install conda-forge::pytorch
conda install pyyaml
```

---

## ExecuTorch Installation

### 1. Clone and Initialize

```bash
git clone -b release/1.1 https://github.com/pytorch/executorch.git
cd executorch
conda activate cpd_env

# Initialize all submodules
git submodule update --init --recursive
```

### 2. Build ExecuTorch

```bash
cmake -B cmake-out --preset linux -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-out -j9
```

### Known Build Error — FlatCC

The following error may occur during the build:

```
executorch/third-party/flatcc/include/flatcc/portable/grisu3_print.h:186:33:
error: initializer-string for array of 'char' truncates NUL terminator
[-Werror=unterminated-string-initialization]
  186 |     static char hexdigits[16] = "0123456789ABCDEF";
```

**Cause:** the ARM GCC compiler appends a null terminator `\0` to the string literal, resulting in 17 characters for a 16-element array.

**Fix:** edit `executorch/third-party/flatcc/include/flatcc/portable/grisu3_print.h` and replace line 186:

```c
// Before (faulty line)
static char hexdigits[16] = "0123456789ABCDEF";

// After (fix)
static char hexdigits[16] = {
    '0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F'
};
```

Then re-run the build:

```bash
cmake -B cmake-out --preset linux -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-out -j9
```

---

## Project Build

Place the built ExecuTorch repository inside `app_code/executorch/`, then from `app_code/`:

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

**CMake flags description:**

| Flag | Description |
|------|-------------|
| `EXECUTORCH_BUILD_XNNPACK` | ARM-optimized CPU backend (NEON/SIMD) |
| `EXECUTORCH_BUILD_EXTENSION_MODULE` | Module loader for `.pte` model files |
| `EXECUTORCH_BUILD_EXTENSION_DATA_LOADER` | Model weight loading |
| `EXECUTORCH_BUILD_KERNELS_QUANTIZED` | Kernels for quantized models (int8) |
| `EXECUTORCH_BUILD_EXTENSION_FLAT_TENSOR` | FlatTensor format support |
| `EXECUTORCH_BUILD_EXTENSION_NAMED_DATA_MAP` | Named data access for model tensors |
| `EXECUTORCH_BUILD_EXTENSION_TENSOR` | Base tensor extension |

---

## Usage

```bash
# Run the executable from the build folder
./Car_Plate_identification
```

On startup, the system:
1. Initializes GPIO and the LCD screen (color test sequence)
2. Opens both cameras (IDs 2 and 0)
3. Loads the ExecuTorch model from `../Ai_file/model_quantified_executorch.pte`

Once initialization succeeds:

```
pour commencer cliquer sur s
```

Press `s` then `Enter` to start the pipeline. Press `Ctrl+C` to stop gracefully (the system flushes remaining queues and saves the last images before exiting).

**Generated outputs:**

Images and metadata are saved in a `./data/<date>/` folder:
- `<date>.txt` — CSV file with columns `id ; plate_count ; bbox_coordinates`
- `<image_id>.jpeg` — image with drawn bounding boxes

System logs are accessible via syslog (identifier `Plate_detection`):

```bash
journalctl -t Plate_detection -f
```

---

## AI Model

The AI pipeline follows these steps:

```
YOLOv8n (base)
    │
    ▼
Fine-tuning on plate dataset  →  yolov8n_finetune.pt
    │
    ▼
Model pruning  →  yolov8n_finetune_pruned.pt / .onnx
    │
    ▼
Quantization + ExecuTorch export  →  model_quantified_executorch.pte
    │
    ▼
Embedded inference via XNNPACK
```

**Inference parameters:**

| Parameter | Value |
|-----------|-------|
| Score threshold | 0.25 |
| NMS threshold | 0.60 |
| Camera input resolution | 640 × 480 px |
| Plate display resolution | 240 × 320 px |
| Processed anchors | 8,400 |
| Input tensor format | `[1, 3, H, W]` float32 normalized [0, 1] |

---

## Technologies

| Technology | Version | Role |
|------------|---------|------|
| **C++** | C++20 | Embedded real-time application |
| **Python** | 3.10.19 | Model training, pruning, export |
| **YOLOv8n** | Ultralytics | License plate detection |
| **ExecuTorch** | release/1.1 | Embedded inference runtime |
| **XNNPACK** | (bundled) | ARM CPU acceleration (NEON/SIMD) |
| **OpenCV** | 4.x | Video capture, image processing, NMS |
| **PyTorch** | conda-forge | Fine-tuning and export |
| **POSIX threads** | — | Real-time multi-threading (SCHED_FIFO) |
| **syslog** | — | System logging |
| **CMake** | ≥ 3.28 | Build system |
| **Miniconda** | latest aarch64 | Python environment management |

---

## Troubleshooting

**`unterminated-string-initialization` error in FlatCC**

See the [Known Build Error — FlatCC](#known-build-error--flatcc) section above.

**`conda activate` does not work in a bash script**

```bash
source ~/.bashrc
conda activate cpd_env
```

**Camera open error**

Check available device IDs:
```bash
ls /dev/video*
```
IDs 0 and 2 are hardcoded in `System.cpp`. Update them if needed.

**Permission denied on `/dev/gpiomem`**

```bash
sudo usermod -aG gpio $USER
# then log out and back in
```

**Out of memory during `make`**

```bash
make -j2  # reduce parallelism
```

**CMake cannot find ExecuTorch**

Make sure the built `executorch/` directory is present inside `app_code/` — `cMakeLists.txt` includes it via `add_subdirectory(executorch EXCLUDE_FROM_ALL)`.

---

## License

This project is distributed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

Copyright (c) 2025 MamoudouSD
