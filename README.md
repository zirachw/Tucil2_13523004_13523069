# 💡 Image Compression with Quadtree Method
A simple interactive **CLI** in `C++` implements **Divide and Conquer** algorithm for compressing images using a **Quadtree**-based approach, integrating all user-specified parameters.

---

<!-- CONTRIBUTOR -->
<div align="center" id="contributor">
  <strong>
    <h3>~ gelatik kpn yh? 😓 ~</h3>
    <table align="center">
      <tr align="center">
        <td>NIM</td>
        <td>Nama</td>
      </tr>
      <tr align="center">
        <td>13523004</td>
        <td>Razi Rachman Widyadhana</td>
      </tr>
      <tr align="center">
        <td>13523069</td>
        <td>Mochammad Fariz Rifqi Rizqulloh</td>
      </tr>
    </table>
  </strong>
</div>

<div align="center">
  <h3 align="center">~ Tech Stacks ~ </h3>

  <p align="center">
    
[![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)][CPP-url]
  
  </p>
</div>

---

 ## 📦 Installation & Setup
 
 ### ✅ Requirements
 - Git
 - C++ Compiler
 
 ### ⬇️ Installing Dependencies
 
 #### 🪟 Windows
 1. Git
    Download and install Git from:  
    [https://git-scm.com/download/win](https://git-scm.com/download/win)
    
 2. C++ Compiler
    You can follow this tutorial : [https://code.visualstudio.com/docs/cpp/config-mingw](https://code.visualstudio.com/docs/cpp/config-mingw)

 <br/>
 <br/>
 
 #### 🐧 Linux
 1. Git
       ```bash
    sudo apt-get update
    sudo apt-get install git
    ```
 2. C++ Compiler
       ```bash
    sudo apt-get update
    sudo apt-get install build-essential
    ```
 ---
 ## 🛠️ How To Run
 
 ### **🪟 Windows**
     ```bash
    git clone https://github.com/zirachw/Tucil2_13523004_13523069.git
    cd Tucil2_13523004_13523069/src
    g++ -o main main.cpp
    ./main
     ```
 
 ### **🐧 Linux (UNIX system)**
    ```bash
    git clone https://github.com/zirachw/Tucil2_13523004_13523069.git
    cd Tucil2_13523004_13523069/src
    g++ -o main main.cpp
    ./main
     ```
 ---
 ## 📱 Repository Structure
 ```
Tucil2_13523004_13523069
├── README.md
├── bin
├── docs
├── src
│   ├── core
│   │   ├── ErrorMethods.hpp
│   │   ├── Image.hpp
│   │   ├── Input.hpp
│   │   ├── QuadTree.hpp
│   │   └── QuadTreeNode.hpp
│   ├── libs
│   │   ├── gif.h
│   │   ├── stb_image.h
│   │   ├── stb_image_write.h
│   │   └── style.h
│   └── main.cpp
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
