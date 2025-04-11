# 💡 Image Compression with Quadtree Method

A simple interactive **CLI** written in `C++` that implements the **Divide and Conquer** algorithm to compress images using a **Quadtree-based** approach. Supports various error calculation methods and user-defined parameters.

---
<div align="center">
  
### 👥 Contributors

<img src="https://github.com/user-attachments/assets/1ecb5810-ef38-44dd-bd57-3b69b0509d38" width="25%" alt="cover">

</div>

<br>

<div align="center">
  
| NIM       | Name                               |
|-----------|------------------------------------|
| 13523004  | Razi Rachman Widyadhana            |
| 13523069  | Mochammad Fariz Rifqi Rizqulloh    |

</div>

<div align="center">

### 🚀 Tech Stacks

  <p align="center">
    
[![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)][CPP-url]
  
  </p>
</div>

---

## 🔎 Preview

https://github.com/user-attachments/assets/c00084c6-e5fd-494e-a817-b10ed780e820

---

## ✨ Features

### This project contains:

1. **Main Program as Image Compressor with Divide and Conquer approach**
2. **Support for `.jpg`, `.jpeg`, and `.png` image formats**
3. **Support for Windows and UNIX system**
4. **`(Bonus)` Target Compression for more flexibility**
5. **`(Bonus)` Structural Similarity Index (SSIM) Error Measurement Method**
6. **`(Bonus)` GIF Output for better visualization how the QuadTree works**


### **Space for Improvement:** 

1. **Implements GUI for better UI & UX**
2. **Implements support for other image formats besides `.jpg`, `.jpeg`, and `.png`**

### **Note for Users:**

**The CLI colors might differ for each color configuration used on your system**

---

## 🔧 Installation

### 📦 Requirements
- [**C++**](Java-url) 17 or later
- [**G++ (C++ Compiler)**](Java-url) 8 or later
- [**Git**](Gradle-url) 8.12 or later *(if self-compile again)*

### ⬇️ Installing Dependencies

- 🖼 **Windows**
  
  1. **Git**  
   Download and install from:  
   [https://git-scm.com/download/win](https://git-scm.com/download/win)

  3. **MinGW (For C/C++ Programming)**  
     Follow this guide to install the latest MinGW for VS Code:  
     [https://code.visualstudio.com/docs/cpp/config-mingw](https://code.visualstudio.com/docs/cpp/config-mingw)


- 🐧 **Linux / UNIX**

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

### Run Only

1. Make sure C++ Compiler already installed

2. Download the `main.exe` in the `bin` folder

3. Open terminal on the same directory as `main.exe`

4. Run
   
   1. 🖼 **Windows**

      ```bash
      main.exe
      ```

   2. 🐧 **Linux / UNIX**
      ```bash
      chmod +x main.exe
      ./main.exe
      ```

### Compile and Run

1. 🖼 **Windows**

   1. Clone the Repository
      
      ```bash
      git clone https://github.com/zirachw/Tucil2_13523004_13523069.git
      ```

   2. In Repository Workspace, compile and Run the Program
      
      ```bash
      g++ src\main.cpp -o bin\main
      bin\main.exe
      ```

2. 🐧 **Linux / UNIX**

   1. Clone the Repository
      
      ```bash
      git clone https://github.com/zirachw/Tucil2_13523004_13523069.git
      ```

   2. In Repository Workspace, compile and Run the Program
      
      ```bash
      g++ src/main.cpp -o bin/main
      chmod +x bin/main.exe
      bin/main.exe
      ```

---

## 📁 Repository Structure

```
Tucil2_13523004_13523069
├── README.md
├── bin                        // program executable
├── docs                       // program documentation
├── src                        // program main logic
│   ├── core
│   │   ├── ErrorMethod.hpp
│   │   ├── Image.hpp
│   │   ├── Input.hpp
│   │   ├── QuadTree.hpp
│   │   └── QuadTreeNode.hpp
│   │
│   ├── libs
│   │   ├── gif.h
│   │   ├── stb_image.h
│   │   ├── stb_image_write.h
│   │   └── style.h
│   └── main.cpp
│
└── test                        // program test cases
```
 
 ---
 ## 📃 Miscellaneous

 <div align="center">
   
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

 </div>

<!-- MARKDOWN LINKS & IMAGES -->
[CPP-url]: https://learn.microsoft.com/en-us/cpp/cpp/?view=msvc-170
