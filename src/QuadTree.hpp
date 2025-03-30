#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "QuadTreeNode.hpp"

extern unsigned char* imgData; 
extern unsigned char* imgReal;  
extern unsigned char* img2Data;
extern int imgWidth, imgHeight, imgChannels;

struct ImageBuffer {
    vector<unsigned char> buf;
    static void writeFunc(void* context, void* data, int size) {
        ImageBuffer* b = (ImageBuffer*)context;
        b -> buf.insert(b -> buf.end(), (unsigned char*)data, (unsigned char*)data + size);
    }
};

size_t getEncodedSize(unsigned char* image, int w, int h) {
    ImageBuffer out;
    stbi_write_jpg_to_func(ImageBuffer::writeFunc, &out, w, h, 3, image, 90);
    return out.buf.size();
}

class QuadTree {
    private:
        string imageInputPath;
        int mode;
        int threshold;
        int minimumBlock;
        double targetPercentage;
        string imageOutputPath;
        string gifOutputPath;

        bool last; 
        //last == true if we need to write the image to gif
        //last become false if we need to perform binary search for the best threshold
        //[BONUS : Compression Percentage]
        
        int width, height, channel;
        QuadTreeNode root;
        GifWriter g;
        uint8_t* data;
  
    public:
    
        /**
         * Constructs a QuadTree object with the provided parameters.
         *
         * @param imageInputPath The path to the input image file
         * @param mode The compression mode to use (1-5)
         * @param threshold The threshold value for compression
         * @param minimumBlock The minimum block size for subdivision
         * @param targetPercentage The target compression percentage (0.0-1.0)
         * @param imageOutputPath The path for the output image file
         * @param gifOutputPath The path for the output GIF file
         */
        QuadTree(string imageInputPath, int mode, int threshold, int minimumBlock, double targetPercentage, string imageOutputPath, string gifOutputPath) {
            
            this -> imageInputPath = imageInputPath;
            this -> mode = mode;
            this -> threshold = threshold;
            this -> minimumBlock = minimumBlock;
            this -> targetPercentage = targetPercentage;
            this -> imageOutputPath = imageOutputPath;
            this -> gifOutputPath = gifOutputPath;
            this -> width = imgWidth;
            this -> height = imgHeight;
            this -> channel = imgChannels;

            root = QuadTreeNode(0, 0, 0, width, height, mode);
            
            if (targetPercentage == 0) last = true;
            else last = false;

            char* gifOutputPath_c = new char[gifOutputPath.length() + 1];
            strcpy(gifOutputPath_c, gifOutputPath.c_str());

            GifBegin(&g, gifOutputPath_c, width, height, 50);
            data = (uint8_t*)malloc(width * height * 4);
        }

        void writeFakeImageToGif() {
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    int idx = (j * width + i) * 3;
                    int outIdx = (j * width + i) * 4;

                    data[outIdx + 0] = img2Data[idx + 0];
                    data[outIdx + 1] = img2Data[idx + 1];
                    data[outIdx + 2] = img2Data[idx + 2];
                    data[outIdx + 3] = 255;
                }
            }

            GifWriteFrame(&g, data, width, height, 100);
        }

        void writeRealImage(string path) {
            stbi_write_jpg(path.c_str(), width, height, 3, imgData, 90);
        }
        
        void writeFakeImage(string path) {
            stbi_write_jpg(path.c_str(), width, height, 3, img2Data, 90);
        }

        void performQuadTree() {
            queue<QuadTreeNode> q;
            q.push(root);
            int curMaxStep = 0;
            memcpy(img2Data, imgData, width * height * 3);

            while (!q.empty()) {
                QuadTreeNode node = q.front();
                q.pop();

                if (node.getWidth() == 0 || node.getHeight() == 0 || node.getWidth()*node.getHeight() <= minimumBlock) {
                    continue;
                }
                
                int step = node.getStep();
                int X = node.getX();
                int Y = node.getY();
                int width = node.getWidth();
                int height = node.getHeight();

                if (step > curMaxStep) {
                    curMaxStep = step;
                    if (curMaxStep < 10) {
                        if (last) {
                            writeFakeImageToGif();
                            //writeFakeImage("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna0" + to_string(curMaxStep) + ".jpg");
                            //GifWriteFrame(&g, img2.data, this -> width, this -> height, 100);
                        }
                    }
                    else {
                        if (last) {
                            writeFakeImageToGif();
                            //writeFakeImage("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna" + to_string(curMaxStep) + ".jpg");
                            //GifWriteFrame(&g, img2.data, this -> width, this -> height, 100);
                        }
                    }

                    memcpy(img2Data, imgData, width * height * 3);
                }

                double error = node.getError();

                if (error < threshold) {
                    node.fillRealRectangle();
                    if (last) {
                        node.fillFakeRectangle();
                    }
                    continue;
                } 
                else {
                    if (last) {
                        node.fillFakeRectangle();
                    }
                    q.push(QuadTreeNode(step + 1, X, Y, width / 2, height / 2, mode));
                    q.push(QuadTreeNode(step + 1, X + height / 2, Y, width / 2, height - height / 2, mode));
                    q.push(QuadTreeNode(step + 1, X, Y + width / 2, width - width / 2, height / 2, mode));
                    q.push(QuadTreeNode(step + 1, X + height / 2, Y + width / 2, width - width / 2, height - height / 2, mode));
                }
            }

            if (last) {
                for (int i = 0; i < this -> width; i++) {
                    for (int j = 0; j < this -> height; j++) {
                        int idx = (j * this -> width + i) * 3;
                        int outIdx = (j * this -> width + i) * 4;

                        data[outIdx + 0] = imgData[idx + 0];
                        data[outIdx + 1] = imgData[idx + 1];
                        data[outIdx + 2] = imgData[idx + 2];
                        data[outIdx + 3] = 255;
                    }
                }

                writeRealImage(imageOutputPath);
                GifWriteFrame(&g, data, this -> width, this -> height, 100);

                GifEnd(&g);
                free(data);
            }
        }

        void performBinserQuadTree(double ratio) {
            double l = 0.0, r = 5000.0;

            // IMPORTANT : NEED IF ELSE TO HANDLE RANGE OF L AND R
            // IMPORTANT : HANDLE PNG/JPG FORMAT (?)

            //imwrite("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\aveORI.jpg", img);

            size_t initialImageSize = getEncodedSize(imgData, width, height);
            size_t targetImageSize = initialImageSize - (initialImageSize * ratio);

            cout << "original size : " << initialImageSize << endl;
            cout << "target size : " << targetImageSize << endl;

            int bestThreshold = -1;
            last = false;

            for (int i = 1; i <= 13; i++) {
                double mid = (l + r) / 2;
                threshold = mid;
                performQuadTree();
    
                size_t currentImageSize = getEncodedSize(imgData, width, height);

                cout << "threshold : " << mid << " size : " << currentImageSize << endl;

                if (currentImageSize <= targetImageSize) {
                    bestThreshold = mid;
                    r = mid;
                } 
                else {
                    l = mid;
                }

                memcpy(imgData, imgReal, width * height * 3);
            }

            last = true;
            threshold = bestThreshold;
            performQuadTree();
        }
};

#endif