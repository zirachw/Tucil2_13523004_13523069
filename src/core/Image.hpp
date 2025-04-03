#ifndef IMAGE_HPP
#define IMAGE_HPP

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/gif.h"
#include "../libs/style.h"
#include "../libs/stb_image.h"
#include "../libs/stb_image_write.h"
#include <vector>
#include <iomanip>  // For std::fixed and std::setprecision

extern unsigned char* currImgData;
extern unsigned char* initImgData;
extern unsigned char* tempImgData;
int imgWidth, imgHeight, imgChannels;

using namespace std;

class Image {
    
    public:
        // Callback function to count bytes without storing them
        static void count_bytes(void *context, void *data, int size) {
            (void)data; // Unused parameter
            size_t *total = reinterpret_cast<size_t *>(context);
            *total += static_cast<size_t>(size);
        }

        static size_t getEncodedSize(unsigned char* image, int w, int h) {
            // Use the count_bytes callback instead of storing data
            size_t totalBytes = 0;
            stbi_write_jpg_to_func(count_bytes, &totalBytes, w, h, 3, image, 90);
            return totalBytes;
        }

        static size_t getOriginalSize(string path) {

            FILE* file = fopen(path.c_str(), "rb");
            if (!file) {
                cout << "Could not open the image file\n";
                return false;
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

        static bool loadImage(string path, string extension) {

            // Load the image
            currImgData = stbi_load(path.c_str(), &imgWidth, &imgHeight, &imgChannels, 3);
            if (!currImgData) {
                cout << "Could not load the image data\n";
                return false;
            }
        
            imgChannels = 3;
            initImgData = (unsigned char*) malloc (imgWidth * imgHeight * 3);
            memcpy(initImgData, currImgData, imgWidth * imgHeight * 3);
        
            tempImgData = (unsigned char*) malloc (imgWidth * imgHeight * 3);
            memcpy(tempImgData, currImgData, imgWidth * imgHeight * 3);

            return true;
        }
};

#endif // IMAGE_HPP