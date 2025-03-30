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
#include <unordered_map>

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
        // Constants for SSIM calculation
        const double C1 = 6.5025; // (0.01 * 255)^2
        const double C2 = 58.5225; // (0.03 * 255)^2

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

        double getThreshold() {return threshold;}
        void setThreshold(double threshold) {this->threshold = threshold;}

        double getError() {return error;}
        void setError(double error) {this->error = error;}

        void calculateError(int mode) {
        if (mode == 1) {
            //Variance - Corrected implementation
            double sumR = 0, sumG = 0, sumB = 0;
            
            // First pass: calculate average RGB values
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = imgData[idx + 0];
                    uint8_t g = imgData[idx + 1];
                    uint8_t b = imgData[idx + 2];
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                }
            }
            
            int n = width * height;
            double avgR = sumR / n;
            double avgG = sumG / n;
            double avgB = sumB / n;
            
            // Second pass: calculate squared differences for variance
            double sumSqDevR = 0, sumSqDevG = 0, sumSqDevB = 0;
            
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = imgData[idx + 0];
                    uint8_t g = imgData[idx + 1];
                    uint8_t b = imgData[idx + 2];
                    
                    double diffR = r - avgR;
                    double diffG = g - avgG;
                    double diffB = b - avgB;
                    
                    sumSqDevR += diffR * diffR;
                    sumSqDevG += diffG * diffG;
                    sumSqDevB += diffB * diffB;
                }
            }
            
            double varianceR = sumSqDevR / n;
            double varianceG = sumSqDevG / n;
            double varianceB = sumSqDevB / n;
            
            setError((varianceR + varianceG + varianceB) / 3);
            setAvg(avgR, avgG, avgB);
        }
        
        else if (mode == 2) {
            // Mean Absolute Deviation (MAD)
            double sumAbsDevR = 0, sumAbsDevG = 0, sumAbsDevB = 0;
            double sumR = 0, sumG = 0, sumB = 0;
            
            // First pass: calculate average RGB values
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = imgData[idx + 0];
                    uint8_t g = imgData[idx + 1];
                    uint8_t b = imgData[idx + 2];
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                }
            }
            
            int n = width * height;
            double avgR = sumR / n;
            double avgG = sumG / n;
            double avgB = sumB / n;
            
            // Second pass: calculate absolute deviations
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = imgData[idx + 0];
                    uint8_t g = imgData[idx + 1];
                    uint8_t b = imgData[idx + 2];
                    
                    sumAbsDevR += fabs(r - avgR);
                    sumAbsDevG += fabs(g - avgG);
                    sumAbsDevB += fabs(b - avgB);
                }
            }
            
            double madR = sumAbsDevR / n;
            double madG = sumAbsDevG / n;
            double madB = sumAbsDevB / n;
            
            double madRGB = (madR + madG + madB) / 3.0;
            
            setError(madRGB);
            setAvg(avgR, avgG, avgB);
        }
        
        else if (mode == 3) {
            // Max Pixel Difference
            uint8_t minR = 255, minG = 255, minB = 255;
            uint8_t maxR = 0, maxG = 0, maxB = 0;
            double sumR = 0, sumG = 0, sumB = 0;
            
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = imgData[idx + 0];
                    uint8_t g = imgData[idx + 1];
                    uint8_t b = imgData[idx + 2];
                    
                    // Find min and max values for each channel
                    minR = min(minR, r);
                    minG = min(minG, g);
                    minB = min(minB, b);
                    
                    maxR = max(maxR, r);
                    maxG = max(maxG, g);
                    maxB = max(maxB, b);
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                }
            }
            
            double diffR = maxR - minR;
            double diffG = maxG - minG;
            double diffB = maxB - minB;
            
            double diffRGB = (diffR + diffG + diffB) / 3.0;
            
            int n = width * height;
            double avgR = sumR / n;
            double avgG = sumG / n;
            double avgB = sumB / n;
            
            setError(diffRGB);
            setAvg(avgR, avgG, avgB);
        }
        
        else if (mode == 4) {
            // Entropy
            unordered_map<int, int> histR, histG, histB;
            double sumR = 0, sumG = 0, sumB = 0;
            int n = width * height;
            
            // Build histograms and calculate average values
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = imgData[idx + 0];
                    uint8_t g = imgData[idx + 1];
                    uint8_t b = imgData[idx + 2];
                    
                    histR[r]++;
                    histG[g]++;
                    histB[b]++;
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                }
            }
            
            // Calculate entropy for each channel
            double entropyR = 0, entropyG = 0, entropyB = 0;
            
            for (const auto& pair : histR) {
                double probability = static_cast<double>(pair.second) / n;
                entropyR -= probability * log2(probability);
            }
            
            for (const auto& pair : histG) {
                double probability = static_cast<double>(pair.second) / n;
                entropyG -= probability * log2(probability);
            }
            
            for (const auto& pair : histB) {
                double probability = static_cast<double>(pair.second) / n;
                entropyB -= probability * log2(probability);
            }
            
            double entropyRGB = (entropyR + entropyG + entropyB) / 3.0;
            
            double avgR = sumR / n;
            double avgG = sumG / n;
            double avgB = sumB / n;
            
            setError(entropyRGB);
            setAvg(avgR, avgG, avgB);
        }
        
        else if (mode == 5) {
            // SSIM (Structural Similarity Index)
            // We'll calculate a simplified version of SSIM since we're comparing within the same block
            // rather than between original and compressed
            
            double sumR = 0, sumG = 0, sumB = 0;
            double sumSquareR = 0, sumSquareG = 0, sumSquareB = 0;
            int n = width * height;
            
            // First pass: compute means and variances
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    double r = imgData[idx + 0];
                    double g = imgData[idx + 1];
                    double b = imgData[idx + 2];
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                    
                    sumSquareR += r * r;
                    sumSquareG += g * g;
                    sumSquareB += b * b;
                }
            }
            
            double meanR = sumR / n;
            double meanG = sumG / n;
            double meanB = sumB / n;
            
            double varR = (sumSquareR / n) - (meanR * meanR);
            double varG = (sumSquareG / n) - (meanG * meanG);
            double varB = (sumSquareB / n) - (meanB * meanB);
            
            // For SSIM, we're adapting the formula to measure within-block variability
            // Higher variance = more dissimilarity = higher error
            // For a single region, we can use variance as an inverse proxy for SSIM
            // In a regular block, SSIM would be close to 1 (low error)
            // In a varied block, SSIM would be lower (high error)
            
            // We invert the SSIM-inspired metric to make it an error value
            // where higher means more difference
            double ssimR = (2 * meanR * meanR + C1) / (meanR * meanR + meanR * meanR + C1);
            double ssimG = (2 * meanG * meanG + C1) / (meanG * meanG + meanG * meanG + C1);
            double ssimB = (2 * meanB * meanB + C1) / (meanB * meanB + meanB * meanB + C1);
            
            // Factor in variance
            ssimR *= (2 * C2) / (varR + C2);
            ssimG *= (2 * C2) / (varG + C2);
            ssimB *= (2 * C2) / (varB + C2);
            
            // Average and invert (1 - ssim) to get error
            double ssimRGB = (ssimR + ssimG + ssimB) / 3.0;
            double error = 1.0 - ssimRGB;
            
            // Scale the error to make it comparable with other methods
            error *= 100.0;
            
            setError(error);
            setAvg(meanR, meanG, meanB);
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