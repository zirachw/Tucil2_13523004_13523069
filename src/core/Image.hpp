#ifndef IMAGE_HPP
#define IMAGE_HPP

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../libs/gif.h"
#include "../libs/style.h"
#include "../libs/stb_image.h"
#include "../libs/stb_image_write.h"
#include <vector>

extern unsigned char* currImgData;
extern unsigned char* initImgData;
extern unsigned char* tempImgData;
int imgWidth, imgHeight, imgChannels;

using namespace std;

class Image {

    private:
        static int width, height, channel;
        static vector<unsigned char> buffer;
    
    public:
        static void writeFunc(void* context, void* data, int size) {
            vector<unsigned char>& buf = *static_cast<vector<unsigned char>*>(context);
            buf.insert(buf.end(), static_cast<unsigned char*>(data), static_cast<unsigned char*>(data) + size);
        }

        static size_t getEncodedSize(unsigned char* image, int w, int h) {
            buffer.clear();
            stbi_write_jpg_to_func(writeFunc, &buffer, w, h, 3, image, 80);
            return buffer.size();
        }

        static bool loadImage(string path) {
            currImgData = stbi_load(path.c_str(), &width, &height, &channel, 3);
            if (!currImgData) {
                cout << "Could not open or find the image\n";
                return false;
            }
        
            channel = 3;
            initImgData = (unsigned char*) malloc (width * height * 3);
            memcpy(initImgData, currImgData, width * height * 3);
        
            tempImgData = (unsigned char*) malloc (width * height * 3);
            memcpy(tempImgData, currImgData, width * height * 3);

            imgWidth = width;
            imgHeight = height;
            imgChannels = channel;

            cout << "Image loaded successfully\n";
            cout << "Image size: " << width << "x" << height << ", Channels: " << channel << endl;

            return true;
        }
};

// Initialize static members
vector<unsigned char> Image::buffer;
int Image::width = 0;
int Image::height = 0;
int Image::channel = 0;

#endif // IMAGE_HPP