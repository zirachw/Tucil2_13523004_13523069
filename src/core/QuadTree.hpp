#ifndef QUADTREE_HPP
#define QUADTREE_HPP

// Libraries
#include <queue>
#include <time.h>
#include "QuadTreeNode.hpp"

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
 */
extern int imgWidth, imgHeight, imgChannels;

/**
 * @brief Main class for quadtree-based image compression
 * @param mode Error calculation mode (1-5)
 * @param minBlock Minimum block size in pixels
 * @param threshold Error threshold value
 * @param targetPercentage Target compression percentage (0-1)
 * @param inputPath Input image path
 * @param inputExtension Input image file extension
 * @param outputPath Output image path
 * @param gifPath Output GIF path
 * @param lastImg Flag for final image in compression process
 * @param root Root node of the quadtree
 * @param g GIF writer for visualization
 * @param data Buffer for GIF frames
 * @param startTime Compression start time
 * @param endTime Compression end time
 * @param initialSize Initial image size in bytes
 * @param finalSize Final compressed image size in bytes
 * @param compressionPercentage Achieved compression percentage
 * @param quadtreeDepth Maximum depth of the quadtree
 * @param quadtreeNode Number of nodes in the quadtree
 */
class QuadTree {

    private:
        int mode, minBlock;
        double threshold, targetPercentage;
        string inputPath, inputExtension, outputPath, gifPath;

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

        /**
         * @brief Write current image data to GIF animation
         */
        void writeCurrImageToGif() {
            for (int i = 0; i < imgWidth; i++) {
                for (int j = 0; j < imgHeight; j++) {
                    int idx = (j * imgWidth + i) * imgChannels;
                    int outIdx = (j * imgWidth + i) * 4;

                    data[outIdx + 0] = currImgData[idx + 0];
                    data[outIdx + 1] = currImgData[idx + 1];
                    data[outIdx + 2] = currImgData[idx + 2];
                    
                    if (imgChannels == 4) {
                        data[outIdx + 3] = currImgData[idx + 3];
                    } 
                    else {
                        data[outIdx + 3] = 255;
                    }
                }
            }

            GifWriteFrame(&g, data, imgWidth, imgHeight, 100);
        }

        /**
         * @brief Write temporary image data to GIF animation
         */
        void writeTempImageToGif() {
            for (int i = 0; i < imgWidth; i++) {
                for (int j = 0; j < imgHeight; j++) {
                    int idx = (j * imgWidth + i) * imgChannels;
                    int outIdx = (j * imgWidth + i) * 4;

                    data[outIdx + 0] = tempImgData[idx + 0];
                    data[outIdx + 1] = tempImgData[idx + 1];
                    data[outIdx + 2] = tempImgData[idx + 2];

                    if (imgChannels == 4) {
                        data[outIdx + 3] = currImgData[idx + 3];
                    } 
                    else {
                        data[outIdx + 3] = 255;
                    }
                }
            }

            GifWriteFrame(&g, data, imgWidth, imgHeight, 100);
        }

        /**
         * @brief Write current image data to file
         * @param path Output file path
         */
        void writeCurrImage(string path) const {

            if (!path.empty()) {
                if (inputExtension == "png") {
                    stbi_write_png(path.c_str(), imgWidth, imgHeight, imgChannels, currImgData, imgWidth * imgChannels);
                } 
                else {
                    stbi_write_jpg(path.c_str(), imgWidth, imgHeight, imgChannels, currImgData, 90);
                }
            }
        }
    
        /**
         * @brief Write temporary image data to file
         * @param path Output file path
         */
        void writeTempImage(string path) const {

            if (!path.empty()) {
                if (inputExtension == "png") {
                    stbi_write_png(path.c_str(), imgWidth, imgHeight, 4, tempImgData, imgWidth * imgChannels);
                } 
                else {
                    stbi_write_jpg(path.c_str(), imgWidth, imgHeight, 3, tempImgData, 90);
                }
            }
        }
  
    public:

        /**
         * @brief Constructor that initializes compression parameters
         * @param inputPath Path to input image
         * @param mode Error calculation mode
         * @param threshold Error threshold
         * @param minBlock Minimum block size
         * @param targetPercentage Target compression percentage
         * @param outputPath Path for output image
         * @param gifPath Path for output GIF
         * @param inputExtension Input image format
         */
        QuadTree(string inputPath, int mode, double threshold, int minBlock, double targetPercentage, string outputPath, string gifPath, string inputExtension) {
            
            this -> inputPath = inputPath;
            this -> mode = mode;
            this -> threshold = threshold;
            this -> minBlock = minBlock;
            this -> targetPercentage = targetPercentage;
            this -> outputPath = outputPath;
            this -> gifPath = gifPath;
            this -> inputExtension = inputExtension;
            this -> root = QuadTreeNode(0, 0, 0, imgWidth, imgHeight, mode);
            
            if (targetPercentage == 0) lastImg = true;
            else lastImg = false;

            char* gifPathCopy = new char[gifPath.length() + 1];
            strcpy(gifPathCopy, gifPath.c_str());
            GifBegin(&g, gifPathCopy, imgWidth, imgHeight, 50);

            this -> data = (uint8_t*) malloc (imgWidth * imgHeight * 4);
            this -> initialSize = Image::getOriginalSize(inputPath);
            this -> startTime = clock();
            this -> quadtreeNode = 0;
        }
    
        /**
         * @brief Destructor that cleans up resources
         */
        ~QuadTree() {
            if (data != nullptr) {
                free(data);
                data = nullptr;
            }
        }

        /**
         * @brief Perform quadtree compression with fixed threshold
         */
        void performQuadTree() {
            queue<QuadTreeNode> q;
            q.push(root);
            int curMaxStep = 0;
            memcpy(tempImgData, currImgData, imgWidth * imgHeight * imgChannels);

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
                    memcpy(tempImgData, currImgData, width * height * imgChannels);
                }

                if (width == 0 || height == 0 || ((long long)node.getWidth() * (long long)node.getHeight()) < minBlock || node.getError() <= threshold) {
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
                finalSize = Image::getEncodedSize(currImgData, imgWidth, imgHeight, inputExtension, imgChannels);
                compressionPercentage = ((double)(initialSize - finalSize) / initialSize) * 100.0;

                GifEnd(&g);
                free(data);
            }
        }

        /**
         * @brief Perform binary search to find optimal threshold for target ratio
         * @param ratio Target compression ratio
         */
        void performBinserQuadTree(double ratio) {
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
            size_t initImageSize = Image::getOriginalSize(inputPath);
            size_t targetImageSize = initImageSize - (initImageSize * ratio);

            double bestThreshold = -1;
            lastImg = false;

            for (int i = 1; i <= 24; i++) {
                double mid = (l + r) / 2;
                threshold = mid;
                performQuadTree();
    
                size_t currentImageSize = Image::getEncodedSize(currImgData, imgWidth, imgHeight, inputExtension, imgChannels);

                if (currentImageSize <= targetImageSize) {
                    bestThreshold = mid;
                    r = mid;
                } 
                else l = mid;

                memcpy(currImgData, initImgData, imgWidth * imgHeight * imgChannels);
            }

            if (bestThreshold == -1) {
                bestThreshold = threshold;
            }

            lastImg = true;
            threshold = bestThreshold;
            performQuadTree();
        }

        /**
         * @brief Get the maximum depth of the quadtree
         * @return Maximum depth
         */
        int getQuadtreeDepth() const {
            return quadtreeDepth;
        }

        /**
         * @brief Get the number of nodes in the quadtree
         * @return Node count
         */
        int getQuadtreeNode() const {
            return quadtreeNode;
        }

        /**
         * @brief Get the compression percentage achieved
         * @return Compression percentage
         */
        double getCompressionPercentage() const {
            return round(compressionPercentage * 100) / 100.0;
        }

        /**
         * @brief Get the execution time in milliseconds
         * @return Execution time
         */
        int getExecutionTime() const {
            return (endTime - startTime) / (CLOCKS_PER_SEC / 1000);
        }

        /**
         * @brief Get the initial image size in bytes
         * @return Initial size
         */
        int getInitialSize() const {
            return initialSize;
        }

        /**
         * @brief Get the final compressed image size in bytes
         * @return Final size
         */
        int getFinalSize() const {
            return finalSize;
        }
};

#endif