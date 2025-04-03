#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include <queue>
#include "QuadTreeNode.hpp"
#include <time.h>

extern unsigned char* currImgData; 
extern unsigned char* initImgData;  
extern unsigned char* tempImgData;
extern int imgWidth, imgHeight, imgChannels;

class QuadTree {

    private:
        int mode, minBlock;
        double threshold, targetPercentage;
        string inputPath, outputPath, gifPath;

        bool lastImg;
        QuadTreeNode root;
        GifWriter g;
        uint8_t* data;

        time_t startTime, endTime;

        int initialSize;
        int finalSize;
        double compressionPercentage;

        int quadtreeDepth;
        int quadtreeNode;

        void writeCurrImageToGif() {
            for (int i = 0; i < imgWidth; i++) {
                for (int j = 0; j < imgHeight; j++) {
                    int idx = (j * imgWidth + i) * 3;
                    int outIdx = (j * imgWidth + i) * 4;

                    data[outIdx + 0] = currImgData[idx + 0];
                    data[outIdx + 1] = currImgData[idx + 1];
                    data[outIdx + 2] = currImgData[idx + 2];
                    data[outIdx + 3] = 255;
                }
            }

            GifWriteFrame(&g, data, imgWidth, imgHeight, 100);
        }

        void writeTempImageToGif() {
            for (int i = 0; i < imgWidth; i++) {
                for (int j = 0; j < imgHeight; j++) {
                    int idx = (j * imgWidth + i) * 3;
                    int outIdx = (j * imgWidth + i) * 4;

                    data[outIdx + 0] = tempImgData[idx + 0];
                    data[outIdx + 1] = tempImgData[idx + 1];
                    data[outIdx + 2] = tempImgData[idx + 2];
                    data[outIdx + 3] = 255;
                }
            }

            GifWriteFrame(&g, data, imgWidth, imgHeight, 100);
        }

        void writeCurrImage(string path) const {
            stbi_write_jpg(path.c_str(), imgWidth, imgHeight, 3, currImgData, 80);
        }
        
        void writeTempImage(string path) const {
            stbi_write_jpg(path.c_str(), imgWidth, imgHeight, 3, tempImgData, 80);
        }
  
        
    public:
        QuadTree(string inputPath, int mode, double threshold, int minBlock, double targetPercentage, string outputPath, string gifPath) {
            
            this -> inputPath = inputPath;
            this -> mode = mode;
            this -> threshold = threshold;
            this -> minBlock = minBlock;
            this -> targetPercentage = targetPercentage;
            this -> outputPath = outputPath;
            this -> gifPath = gifPath;

            root = QuadTreeNode(0, 0, 0, imgWidth, imgHeight, mode);
            
            if (targetPercentage == 0) lastImg = true;
            else lastImg = false;

            char* gifPathCopy = new char[gifPath.length() + 1];
            strcpy(gifPathCopy, gifPath.c_str());

            GifBegin(&g, gifPathCopy, imgWidth, imgHeight, 50);
            data = (uint8_t*)malloc(imgWidth * imgHeight * 4);

            initialSize = Image::getEncodedSize(currImgData, imgWidth, imgHeight);
            startTime = clock();

            quadtreeNode = 0;
        }
    
        ~QuadTree() {
            if (data != nullptr) {
                delete[] data;
                data = nullptr;
            }
        }

        void performQuadTree() {
            queue<QuadTreeNode> q;
            q.push(root);
            int curMaxStep = 0;
            memcpy(tempImgData, currImgData, imgWidth * imgHeight * 3);

            while (!q.empty()) {
                QuadTreeNode node = q.front();
                q.pop();
                
                if (lastImg) quadtreeNode++;

                
                int step = node.getStep();
                int X = node.getX();
                int Y = node.getY();
                int width = node.getWidth();
                int height = node.getHeight();
                if (step < curMaxStep) continue;


                if (step > curMaxStep && lastImg) {
                    quadtreeDepth = step;
                    curMaxStep = step;
                    writeTempImageToGif();
                    memcpy(tempImgData, currImgData, width * height * 3);
                }

                if (width == 0 || height == 0 || min(node.getWidth(), node.getHeight()) < minBlock || node.getError() <= threshold) {
                    node.fillCurrRectangle();
                    if (lastImg) {
                        node.fillTempRectangle();
                    }
                    continue;
                } 
                else {
                    if (lastImg) {
                        node.fillTempRectangle();
                    }
                    q.push(QuadTreeNode(step + 1, X, Y, width / 2, height / 2, mode));
                    q.push(QuadTreeNode(step + 1, X + height / 2, Y, width / 2, height - height / 2, mode));
                    q.push(QuadTreeNode(step + 1, X, Y + width / 2, width - width / 2, height / 2, mode));
                    q.push(QuadTreeNode(step + 1, X + height / 2, Y + width / 2, width - width / 2, height - height / 2 , mode));
                }
            }

            if (lastImg) {
                writeCurrImageToGif();
                writeCurrImage(outputPath);
                
                endTime = clock();  
                finalSize = Image::getEncodedSize(currImgData, imgWidth, imgHeight);
                compressionPercentage = ((double)(initialSize - finalSize) / initialSize) * 100.0;

                GifEnd(&g);
                free(data);
            }
        }

        void performBinserQuadTree(double ratio) {
            // Get appropriate upper and lower thresholds based on selected mode
            ErrorMethod* errorMethod = nullptr;
            double lowerThreshold = 0.0, upperThreshold = 0.0;
            
            switch(mode) {
                case 1:
                    errorMethod = new Variance();
                    break;
                case 2:
                    errorMethod = new MeanAbsoluteDeviation();
                    break;
                case 3:
                    errorMethod = new MaxPixelDifference();
                    break;
                case 4:
                    errorMethod = new Entropy();
                    break;
                case 5:
                    errorMethod = new SSIM();
                    break;
                default:
                    errorMethod = new Variance();
                    break;
            }
            
            if (errorMethod) 
            {
                lowerThreshold = errorMethod->getLowerThreshold();
                upperThreshold = errorMethod->getUpperThreshold();
                delete errorMethod;
            }
            
            double l = lowerThreshold, r = upperThreshold;
            
            cout << "Using error method " << mode << " with threshold range: " << lowerThreshold << " to " << upperThreshold << endl;

            size_t initImageSize = Image::getOriginalSize(inputPath);
            size_t targetImageSize = initImageSize - (initImageSize * ratio);

            cout << "Init Size: " << initImageSize << "bytes (" << Image::getSizeInKB(initImageSize) << " KB)" << endl;
            cout << "Target Size: " << targetImageSize << "bytes (" << Image::getSizeInKB(targetImageSize) << " KB)" << endl;

            double bestThreshold = -1;
            lastImg = false;

            for (int i = 1; i <= 13; i++) {
                double mid = (l + r) / 2;
                threshold = mid;
                performQuadTree();
    
                size_t currentImageSize = Image::getEncodedSize(currImgData, imgWidth, imgHeight);

                // cout << "threshold : " << mid << " size : " << currentImageSize << endl;

                if (currentImageSize <= targetImageSize) {
                    bestThreshold = mid;
                    r = mid;
                } 
                else l = mid;

                memcpy(currImgData, initImgData, imgWidth * imgHeight * 3);
            }

            lastImg = true;
            threshold = bestThreshold;
            performQuadTree();
            // cout << "threshold : " << bestThreshold << " size : " << currentImageSize << endl;
        }

        int getQuadtreeDepth() const {
            return quadtreeDepth;
        }
        int getQuadtreeNode() const {
            return quadtreeNode;
        }

        double getCompressionPercentage() const {
            return round(compressionPercentage * 100) / 100.0;
        }

        int getExecutionTime() const {
            return (endTime - startTime) / (CLOCKS_PER_SEC / 1000);
        }

        int getInitialSize() const {
            return initialSize;
        }

        int getFinalSize() const {
            return finalSize;
        }


};

#endif