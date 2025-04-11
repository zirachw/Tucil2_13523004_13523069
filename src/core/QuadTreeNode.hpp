#ifndef QUADTREENODE_HPP
#define QUADTREENODE_HPP

// Libraries
#include "ErrorMethod.hpp"
#include <tuple>

/**
 * @brief Image data buffers used throughout the compression process
 * @param currImgData Current image data buffer used for processing
 * @param initImgData Initial image data buffer kept as reference
 * @param tempImgData Temporary image data buffer for intermediate processing
 */
extern unsigned char *currImgData, *initImgData, *tempImgData;

/**
 * @brief Global image dimensions and format information
 * @param imgWidth Width of the image in pixels
 * @param imgHeight Height of the image in pixels
 * @param imgChannels Number of color channels (typically 3 for RGB, 4 for RGBA)
 */
extern int imgWidth, imgHeight, imgChannels;

/**
 * @brief Global variable for error calculation method
 * @param errorMethod Pointer to the current error calculation method
 */
ErrorMethod *errorMethod = nullptr;

/**
 * @brief Represents a node in the quadtree for an image region
 * @param step Current depth/level in the quadtree
 * @param x X-coordinate of the region
 * @param y Y-coordinate of the region
 * @param width Width of the region in pixels
 * @param height Height of the region in pixels
 * @param mode Error calculation mode
 * @param error Calculated error value for this region
 * @param avgR Average red value for this region
 * @param avgG Average green value for this region
 * @param avgB Average blue value for this region
 */
class QuadTreeNode {

    private:
        int step, x, y, width, height, mode;
        double error;
        double avgR, avgG, avgB;

    public:
        /**
         * @brief Default constructor for QuadTreeNode
         */
        QuadTreeNode() {
            x = 0;
            y = 0;
            width = 0;
            height = 0;
            step = 0;
            error = 0;
            avgR = 0;
            avgG = 0;
            avgB = 0;
        }

        /**
         * @brief Parameterized constructor for QuadTreeNode
         * @param step Current depth/level in the quadtree
         * @param x X-coordinate of the region
         * @param y Y-coordinate of the region
         * @param width Width of the region in pixels
         * @param height Height of the region in pixels
         * @param mode Error calculation mode
         */
        QuadTreeNode(int step, int x, int y, int width, int height, int mode) {
            this->step = step;
            this->x = x;
            this->y = y;
            this->width = width;
            this->height = height;
            this->mode = mode;
            this->error = 0;
            this->avgR = 0;
            this->avgG = 0;
            this->avgB = 0;
            calculateError(mode);
        }

        /**
         * @brief Assignment operator
         * @param node Source node to copy from
         * @return Reference to this node
         */
        QuadTreeNode& operator=(const QuadTreeNode& node) {
            x = node.x;
            y = node.y;
            width = node.width;
            height = node.height;
            step = node.step;
            error = node.error;
            avgR = node.avgR;
            avgG = node.avgG;
            avgB = node.avgB;

            return *this;
        }

        /**
         * @brief Calculate the error for this region using the specified error method
         * @param mode Error calculation mode (1-5)
         */
        void calculateError(int mode) {
            if (errorMethod == nullptr) {
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
                }
            }
            
            if (errorMethod) {
                setError(errorMethod->calculateError(currImgData, x, y, width, height));
                setAvg(errorMethod->getAvgR(), errorMethod->getAvgG(), errorMethod->getAvgB());
            }
            else {
                cout << "Error: Error method not initialized." << endl;
            }
        }

        /**
         * @brief Fill the rectangle region with the average RGB values
         * @param image Pointer to the image data
         */
        void fillRectangle(unsigned char* image) {
            if (!image) return;

            for (int i = x; i < x + height; ++i) {
                for (int j = y; j < y + width; ++j) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    image[idx] = static_cast<unsigned char>(avgR);
                    image[idx + 1] = static_cast<unsigned char>(avgG);
                    image[idx + 2] = static_cast<unsigned char>(avgB);
                }
            }
        }

        /**
         * @brief Fill the current image data with the average color of this region
         */
        void fillCurrRectangle() {
            fillRectangle(currImgData);
        }

        /**
         * @brief Fill the temporary image data with the average color of this region
         */
        void fillTempRectangle() {
            fillRectangle(tempImgData);
        }

        /**
         * @brief Get the current step in the quadtree
         * @return Current step value
         */
        int getStep() {return step;}

        /**
         * @brief Get the current step in the quadtree
         * @return Current step value
         */
        void setStep(int step) {this->step = step;}

        /**
         * @brief Get the X-coordinate of the region
         * @return X-coordinate value
         */
        int getX() {return x;}

        /**
         * @brief Get the Y-coordinate of the region
         * @return Y-coordinate value
         */
        void setX(int x) {this->x = x;}

        /**
         * @brief Get the Y-coordinate of the region
         * @return Y-coordinate value
         */
        void setY(int y) {this->y = y;}

        /**
         * @brief Get the Y-coordinate of the region
         * @return Y-coordinate value
         */
        int getY() {return y;}

        /**
         * @brief Get the width of the region
         * @return Width value
         */
        int getWidth() {return width;}

        /**
         * @brief Set the width of the region
         * @param width Width value to set
         */
        void setWidth(int width) {this->width = width;}
        
        /**
         * @brief Get the height of the region
         * @return Height value
         */
        int getHeight() {return height;}

        /**
         * @brief Set the height of the region
         * @param height Height value to set
         */
        void setHeight(int height) {this->height = height;}

        /**
         * @brief Get the average RGB values for this region
         * @return Tuple of average R, G, B values
         */
        tuple<double, double, double> getAvg() {return {avgR, avgG, avgB};}

        /**
         * @brief Set the average RGB values for this region
         * @param avgR Average red value
         * @param avgG Average green value
         * @param avgB Average blue value
         */
        void setAvg(double avgR, double avgG, double avgB) {
            this->avgR = avgR;
            this->avgG = avgG;
            this->avgB = avgB;
        }

        /**
         * @brief Get the error value for this region
         * @return Error value
         */
        double getError() {return error;}

        /**
         * @brief Set the error value for this region
         * @param error Error value to set
         */
        void setError(double error) {this->error = error;}
};

#endif