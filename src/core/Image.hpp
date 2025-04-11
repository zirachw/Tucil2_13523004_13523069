#ifndef IMAGE_HPP
#define IMAGE_HPP

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// Libraries
#include "../libs/gif.h"
#include "../libs/style.h"
#include "../libs/stb_image.h"
#include "../libs/stb_image_write.h"
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>

/**
 * @brief Image data buffers used throughout the compression process
 * @param currImgData Current image data buffer used for processing
 * @param initImgData Initial image data buffer kept as reference
 * @param tempImgData Temporary image data buffer for intermediate processing
 */
extern unsigned char* currImgData, *initImgData, *tempImgData;

/**
 * @brief Global image dimensions and format information
 * @param imgWidth Width of the image in pixels
 * @param imgHeight Height of the image in pixels
 * @param imgChannels Number of color channels (typically 3 for RGB, 4 for RGBA)
 * @param compressionQuality Adaptive compression quality for stb_image_write JPG/JPEG encoding
 */
int imgWidth, imgHeight, imgChannels, compressionQuality;

using namespace std;

/**
 * @brief Static utility class for image operations
 */
class Image {
    
    public:
        /**
         * @brief Callback function for stbi_write functions
         * @param context Pointer to context (size variable)
         * @param data Pointer to data
         * @param size Size of data
         */
        static void getBytes(void *context, void *data, int size) {
            (void)data;
            size_t *total = reinterpret_cast<size_t *>(context);
            *total += static_cast<size_t>(size);
        }

        /**
         * @brief Calculate the size after encoding with a specific format
         * @param image Pointer to image data
         * @param w Width of the image
         * @param h Height of the image
         * @param extension File extension/format
         * @param channels Number of color channels
         * @return Size in bytes after encoding
         */
        static size_t getEncodedSize(unsigned char* image, int w, int h, const string& extension, int channels = 3, int quality = 90) {

            size_t totalBytes = 0;
            if (extension == "png") {
                stbi_write_png_to_func(getBytes, &totalBytes, w, h, channels, image, w * channels);
            } 
            else {
                stbi_write_jpg_to_func(getBytes, &totalBytes, w, h, channels, image, quality);
            }
            
            return totalBytes;
        }

        /**
         * @brief Get the original size of the image file
         * @param path Path to the image file
         * @return Size in bytes of the original image file
         */
        static size_t getOriginalSize(string path) {
            FILE* file = fopen(path.c_str(), "rb");
            if (!file) {
                cout << "Could not open the image file\n";
                return 0;
            }
            
            fseek(file, 0, SEEK_END);
            size_t originalSize = ftell(file);
            fclose(file);

            return originalSize;
        }

        /**
         * @brief Convert size in bytes to kilobytes
         * @param sizeInBytes Size in bytes
         * @return Size in kilobytes
         */
        static double getSizeInKB(size_t sizeInBytes) {
            return static_cast<double>(sizeInBytes) / 1024.0;
        }

        /**
         * @brief Load an image from file into memory
         * @param path Path to the image file
         * @param extension File extension/format
         * @return Empty string if successful, error message if failed
         */
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

            if (extension != "png") {
                int originalSize = getOriginalSize(path);

                compressionQuality = 50;
                int l = 5, r = 100;
                while (l <= r) {
                    int mid = (l+r)/2;

                    int curSize = getEncodedSize(initImgData, imgWidth, imgHeight, extension, imgChannels, mid);

                    if (curSize <= originalSize) {
                        compressionQuality = mid;
                        l = mid+1;
                    }
                    else {
                        r = mid-1;
                    }
                }
            }

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