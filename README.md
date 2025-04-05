# 💡 Image Compression with Quadtree Method

A simple interactive **CLI** written in `C++` that implements the **Divide and Conquer** algorithm to compress images using a **Quadtree-based** approach. Supports various error calculation methods and user-defined parameters.

---

## 👥 Contributors

<div align="center">

### ~ Gelatik kapan yah? 😓 ~

| NIM       | Name                               |
|-----------|------------------------------------|
| 13523004  | Razi Rachman Widyadhana            |
| 13523069  | Mochammad Fariz Rifqi Rizqulloh    |

</div>

---

## 🚀 Tech Stack

<p align="center">
  <a href="https://learn.microsoft.com/en-us/cpp/cpp/?view=msvc-170">
    <img src="https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++ Badge">
  </a>
</p>

---

## 📆 Installation & Setup

### ✅ Requirements
- Git
- C++ Compiler

---

### ⬇️ Installing Dependencies

#### 🪟 Windows

1. **Git**  
   Download and install from:  
   [https://git-scm.com/download/win](https://git-scm.com/download/win)

2. **C++ Compiler**  
   Follow this guide to install MinGW for VS Code:  
   [https://code.visualstudio.com/docs/cpp/config-mingw](https://code.visualstudio.com/docs/cpp/config-mingw)

---

#### 🐧 Linux

1. **Git**
   ```bash
   sudo apt-get update
   sudo apt-get install git
   ```

2. **C++ Compiler**
   ```bash
   sudo apt-get install build-essential
   ```

---

## 🛠️ How To Run

https://github.com/user-attachments/assets/c00084c6-e5fd-494e-a817-b10ed780e820

### 🪟 Windows
```bash
git clone https://github.com/zirachw/Tucil2_13523004_13523069.git
cd Tucil2_13523004_13523069/src
g++ -o main main.cpp
./main
```

### 🐧 Linux / UNIX
```bash
git clone https://github.com/zirachw/Tucil2_13523004_13523069.git
cd Tucil2_13523004_13523069/src
g++ -o main main.cpp
./main
```

---

## 📁 Repository Structure

```
Tucil2_13523004_13523069
├── README.md
├── bin
├── docs
├── src
│   ├── core
│   │   ├── ErrorMethods.hpp
│   │   ├── Image.hpp
│   │   ├── Input.hpp
│   │   ├── QuadTree.hpp
│   │   └── QuadTreeNode.hpp
│   ├── libs
│   │   ├── gif.h
│   │   ├── stb_image.h
│   │   ├── stb_image_write.h
│   │   └── style.h
│   └── main.cpp
└── test
```
 
 ---
 ## 📃 Miscellaneous
 | No | Points | Ya | Tidak |
 | --- | --- | --- | --- |
 | 1 | Program berhasil dikompilasi tanpa kesalahan. | ✔️ | |
 | 2 | Program berhasil dijalankan. | ✔️ | |
 | 3 | Program berhasil melakukan kompresi gambar sesuai parameter yang ditentukan. | ✔️ | |
 | 4 | Mengimplementasi seluruh metode perhitungan error wajib. | ✔️ | |
 | 5 | **[Bonus]** Implementasi persentase kompresi sebagai parameter tambahan. | ✔️ | |
 | 6 | **[Bonus]** Implementasi Structural Similarity Index (SSIM) sebagai metode pengukuran error. | ✔️ | |
 | 7 | **[Bonus]** Output berupa GIF Visualisasi Proses pembentukan Quadtree dalam Kompresi Gambar. | ✔️ | |
 | 8 | Program dan laporan dibuat (kelompok) sendiri. | ✔️ | |

<!-- MARKDOWN LINKS & IMAGES -->
[CPP-url]: https://learn.microsoft.com/en-us/cpp/cpp/?view=msvc-170
