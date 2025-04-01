#ifndef QUADTREENODE_HPP
#define QUADTREENODE_HPP

#include "ErrorMethods.hpp"
#include <tuple>

extern unsigned char* currImgData;
extern unsigned char* tempImgData;
extern int imgWidth, imgHeight, imgChannels;
ErrorMethod* errorMethod = nullptr;

class QuadTreeNode {

    private:
        int step, x, y, width, height, mode;
        double error;
        double avgR, avgG, avgB;

    public:
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

        int getStep() {return step;}
        void setStep(int step) {this->step = step;}

        int getX() {return x;}
        void setX(int x) {this->x = x;}

        void setY(int y) {this->y = y;}
        int getY() {return y;}

        int getWidth() {return width;}
        void setWidth(int width) {this->width = width;}
        
        int getHeight() {return height;}
        void setHeight(int height) {this->height = height;}

        tuple<double, double, double> getAvg() {return {avgR, avgG, avgB};}
        void setAvg(double avgR, double avgG, double avgB) {
            this->avgR = avgR;
            this->avgG = avgG;
            this->avgB = avgB;
        }

        double getError() {return error;}
        void setError(double error) {this->error = error;}

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

        void fillCurrRectangle() {
            fillRectangle(currImgData);
        }

        void fillTempRectangle() {
            fillRectangle(tempImgData);
        }

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
};

#endif