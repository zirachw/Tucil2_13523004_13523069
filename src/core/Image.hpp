#ifndef IMAGE_HPP
#define IMAGE_HPP

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/gif.h"
#include "../libs/style.h"
#include "../libs/stb_image.h"
#include "../libs/stb_image_write.h"
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>

// Global variables for image data
extern unsigned char* currImgData;
extern unsigned char* initImgData;
extern unsigned char* tempImgData;
int imgWidth, imgHeight, imgChannels;

using namespace std;

class Image {
    
    public:
        static void getBytes(void *context, void *data, int size) {
            (void)data;
            size_t *total = reinterpret_cast<size_t *>(context);
            *total += static_cast<size_t>(size);
        }

        static size_t getEncodedSize(unsigned char* image, int w, int h, const string& extension, int channels = 3) {

            size_t totalBytes = 0;
            if (extension == "png") {
                stbi_write_png_to_func(getBytes, &totalBytes, w, h, channels, image, w * channels);
            } 
            else {
                stbi_write_jpg_to_func(getBytes, &totalBytes, w, h, channels, image, 90);
            }
            
            return totalBytes;
        }

        static size_t getEncodedSize(unsigned char* image, int w, int h) {
            // Default to jpg for backward compatibility
            return getEncodedSize(image, w, h, "jpg");
        }

        static size_t getOriginalSize(string path) {
            FILE* file = fopen(path.c_str(), "rb");
            if (!file) {
                cout << "Could not open the image file\n";
                return 0;
            }
            
            // Get original file size
            fseek(file, 0, SEEK_END);
            size_t originalSize = ftell(file);
            fclose(file);

            return originalSize;
        }

        static double getSizeInKB(size_t sizeInBytes) {
            return static_cast<double>(sizeInBytes) / 1024.0;
        }

        static string loadImage(string path, string extension) {
            
            currImgData = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, 0);
            
            if (imgChannels < 3) {
                return "Image-nya harus ada minimal 3 channel (RGB), kok ini cuma " + to_string(imgChannels) + " channel doang.";
            }
            
            if (!currImgData) {
                return "Image-nya gagal di-load, coba ulang ya...";
            }
            
            // Allocate memory for backup copies
            initImgData = (unsigned char*) malloc(imgWidth * imgHeight * imgChannels);
            if (!initImgData) {
                return "Gagal alokasi memori, coba ulang ya.";
            }
            memcpy(initImgData, currImgData, imgWidth * imgHeight * imgChannels);

            tempImgData = (unsigned char*) malloc(imgWidth * imgHeight * imgChannels);
            if (!tempImgData) {
                free(initImgData);
                return "Gagal alokasi memori, coba ulang ya.";
            }
            memcpy(tempImgData, currImgData, imgWidth * imgHeight * imgChannels);

            return ""; // No error
        }
};

#endif // IMAGE_HPP