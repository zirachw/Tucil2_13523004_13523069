#ifndef QUADTREENODE_HPP
#define QUADTREENODE_HPP

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gif.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <queue>
#include <cstring>
#include <stdio.h>

extern unsigned char* imgData;
extern unsigned char* img2Data;
extern int imgWidth, imgHeight, imgChannels;

using namespace std;

class QuadTreeNode {

    private:
        int step, x, y, width, height, mode;
        double error;
        double threshold;
        double avgR, avgG, avgB;

    public:

        QuadTreeNode() {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        step = 0;
        error = 0;
        threshold = 500;
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
            this->threshold = 500;
            this->avgR = 0;
            this->avgG = 0;
            this->avgB = 0;

            calculateError(1);
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

        double getThreshold() {return threshold;}
        void setThreshold(double threshold) {this->threshold = threshold;}

        double getError() {return error;}
        void setError(double error) {this->error = error;}

        void calculateError(int mode) {
        if (mode == 1) {
            //Variance

            double avgR = 0, avgG = 0, avgB = 0;
            double avgR2 = 0, avgG2 = 0, avgB2 = 0;
            double varianceR = 0, varianceG = 0, varianceB = 0;

            for (int i = x; i < x + height; i++) {
            for (int j = y; j < y + width; j++) {
                int idx = (i * imgWidth + j) * imgChannels;

                int8_t r = imgData[idx + 0];
                int8_t g = imgData[idx + 1];
                int8_t b = imgData[idx + 2];

                avgR += r;
                avgG += g;
                avgB += b;

                avgR2 += r * r;
                avgG2 += g * g;
                avgB2 += b * b;
            }
            }
        
            int n = width * height;
            avgR /= n;
            avgG /= n;
            avgB /= n;
            avgR2 /= n;
            avgG2 /= n;
            avgB2 /= n;
        
            varianceR = avgR2 - avgR * avgR;
            varianceG = avgG2 - avgG * avgG;
            varianceB = avgB2 - avgB * avgB;
        
            //cout << x << " " << y << " " << width << " " << height << " " << (varianceR + varianceG + varianceB) / 3 << endl;
        
            setError((varianceR + varianceG + varianceB) / 3);
            setAvg(avgR, avgG, avgB);
        }
        
        else if (mode == 2) {
            //Mean 
        }
        
        else if (mode == 3) {
            //Max pixel difference
        }
        
        else if (mode == 4) {
            //entrophy
        }
        
        else {
            //ssim
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

        void fillRealRectangle() {
            fillRectangle(imgData);
        }

        void fillFakeRectangle() {
            fillRectangle(img2Data);
        }

        QuadTreeNode& operator=(const QuadTreeNode& node) {
        x = node.x;
        y = node.y;
        width = node.width;
        height = node.height;
        step = node.step;
        error = node.error;
        threshold = node.threshold;
        avgR = node.avgR;
        avgG = node.avgG;
        avgB = node.avgB;

        return *this;
        }
};

#endif