#ifndef ERROR_METHODS_HPP
#define ERROR_METHODS_HPP

#include "Image.hpp"
#include <unordered_map>

extern int imgWidth, imgHeight, imgChannels;
extern unsigned char* currImgData;

class ErrorMethod {
    protected:  
        double upperThreshold;
        double lowerThreshold;
        double avgR, avgG, avgB;
    public:
        virtual double calculateError(unsigned char* currImgData, int x, int y, int width, int height) = 0;
        double getAvgR() const { return avgR; }
        double getAvgG() const { return avgG; }
        double getAvgB() const { return avgB; }
        double getUpperThreshold() const { return upperThreshold; }
        double getLowerThreshold() const { return lowerThreshold; }
};

class Variance : public ErrorMethod {
    private:
        long long** prefsumR;
        long long** prefsumG;
        long long** prefsumB;
        long long** prefsumR2;
        long long** prefsumG2;
        long long** prefsumB2;

    public:
        Variance() {
            upperThreshold = 127.5 * 127.5; // Example threshold value
            lowerThreshold = 0;

            cout << "Calculating prefix sum..." << endl;
            cout << "Image width: " << imgWidth << ", Image height: " << imgHeight << endl;

            prefsumR = new long long*[imgHeight];
            prefsumG = new long long*[imgHeight];
            prefsumB = new long long*[imgHeight];
            prefsumR2 = new long long*[imgHeight];
            prefsumG2 = new long long*[imgHeight];
            prefsumB2 = new long long*[imgHeight];
            for (int i = 0; i < imgHeight; ++i) {
                prefsumR[i] = new long long[imgWidth];
                prefsumG[i] = new long long[imgWidth];
                prefsumB[i] = new long long[imgWidth];
                prefsumR2[i] = new long long[imgWidth];
                prefsumG2[i] = new long long[imgWidth];
                prefsumB2[i] = new long long[imgWidth];
            }

            for (int i = 0; i < imgHeight; i++) {
                for (int j = 0; j < imgWidth; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                   
                    prefsumR[i][j] = currImgData[idx + 0];
                    prefsumG[i][j] = currImgData[idx + 1];
                    prefsumB[i][j] = currImgData[idx + 2];
                    prefsumR2[i][j] = currImgData[idx + 0] * currImgData[idx + 0];
                    prefsumG2[i][j] = currImgData[idx + 1] * currImgData[idx + 1];
                    prefsumB2[i][j] = currImgData[idx + 2] * currImgData[idx + 2];

                    if (i == 0 && j == 0) {
                        continue;
                    }
                    else if (i == 0) {
                        prefsumR[i][j] += prefsumR[i][j - 1];
                        prefsumG[i][j] += prefsumG[i][j - 1];
                        prefsumB[i][j] += prefsumB[i][j - 1];
                        prefsumR2[i][j] += prefsumR2[i][j - 1];
                        prefsumG2[i][j] += prefsumG2[i][j - 1];
                        prefsumB2[i][j] += prefsumB2[i][j - 1];
                    }
                    else if (j == 0) {
                        prefsumR[i][j] += prefsumR[i - 1][j];
                        prefsumG[i][j] += prefsumG[i - 1][j];
                        prefsumB[i][j] += prefsumB[i - 1][j];
                        prefsumR2[i][j] += prefsumR2[i - 1][j];
                        prefsumG2[i][j] += prefsumG2[i - 1][j];
                        prefsumB2[i][j] += prefsumB2[i - 1][j];
                    }
                    else {
                        prefsumR[i][j] += prefsumR[i - 1][j] + prefsumR[i][j - 1] - prefsumR[i - 1][j - 1];
                        prefsumG[i][j] += prefsumG[i - 1][j] + prefsumG[i][j - 1] - prefsumG[i - 1][j - 1];
                        prefsumB[i][j] += prefsumB[i - 1][j] + prefsumB[i][j - 1] - prefsumB[i - 1][j - 1];
                        prefsumR2[i][j] += prefsumR2[i - 1][j] + prefsumR2[i][j - 1] - prefsumR2[i - 1][j - 1];
                        prefsumG2[i][j] += prefsumG2[i - 1][j] + prefsumG2[i][j - 1] - prefsumG2[i - 1][j - 1];
                        prefsumB2[i][j] += prefsumB2[i - 1][j] + prefsumB2[i][j - 1] - prefsumB2[i - 1][j - 1];
                    }

                    // if (i < 10 && j < 10) {
                    //     cout << "i: " << i << ", j: " << j << ", prefsumR: " << prefsumR[i][j] << ", prefsumG: " << prefsumG[i][j] << ", prefsumB: " << prefsumB[i][j] << endl;
                    //     cout << "i: " << i << ", j: " << j << ", prefsumR2: " << prefsumR2[i][j] << ", prefsumG2: " << prefsumG2[i][j] << ", prefsumB2: " << prefsumB2[i][j] << endl;
                    // }

                    if (i > 555 && j > 995) {
                        cout << "i: " << i << ", j: " << j << ", prefsumR: " << prefsumR[i][j] << ", prefsumG: " << prefsumG[i][j] << ", prefsumB: " << prefsumB[i][j] << endl;
                        cout << "i: " << i << ", j: " << j << ", prefsumR2: " << prefsumR2[i][j] << ", prefsumG2: " << prefsumG2[i][j] << ", prefsumB2: " << prefsumB2[i][j] << endl;
                    }
                }
            }
        }

        ~Variance() {
            for (int i = 0; i < imgHeight; ++i) {
                delete[] prefsumR[i];
                delete[] prefsumG[i];
                delete[] prefsumB[i];
                delete[] prefsumR2[i];
                delete[] prefsumG2[i];
                delete[] prefsumB2[i];
            }
            delete[] prefsumR;
            delete[] prefsumG;
            delete[] prefsumB;
            delete[] prefsumR2;
            delete[] prefsumG2;
            delete[] prefsumB2;
        }
        
        double calculateError(unsigned char* currImgData, int row, int col, int width, int height) override {
            long long sumR, sumG, sumB;
            long long sumR2, sumG2, sumB2;

            if (width == 0 || height == 0) {
                return 0;
            }

            if (row == 0 && col == 0) {
                sumR = prefsumR[row + height - 1][col + width - 1];
                sumG = prefsumG[row + height - 1][col + width - 1];
                sumB = prefsumB[row + height - 1][col + width - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1];
            }
            else if (row == 0) {
                sumR = prefsumR[row + height - 1][col + width - 1] - prefsumR[row + height - 1][col - 1];
                sumG = prefsumG[row + height - 1][col + width - 1] - prefsumG[row + height - 1][col - 1];
                sumB = prefsumB[row + height - 1][col + width - 1] - prefsumB[row + height - 1][col - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1] - prefsumR2[row + height - 1][col - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1] - prefsumG2[row + height - 1][col - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1] - prefsumB2[row + height - 1][col - 1];
            }
            else if (col == 0) {
                sumR = prefsumR[row + height - 1][col + width - 1] - prefsumR[row - 1][col + width - 1];
                sumG = prefsumG[row + height - 1][col + width - 1] - prefsumG[row - 1][col + width - 1];
                sumB = prefsumB[row + height - 1][col + width - 1] - prefsumB[row - 1][col + width - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1] - prefsumR2[row - 1][col + width - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1] - prefsumG2[row - 1][col + width - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1] - prefsumB2[row - 1][col + width - 1];
            }
            else {
                sumR = prefsumR[row + height - 1][col + width - 1] - prefsumR[row - 1][col + width - 1] - prefsumR[row + height - 1][col - 1] + prefsumR[row - 1][col - 1];
                sumG = prefsumG[row + height - 1][col + width - 1] - prefsumG[row - 1][col + width - 1] - prefsumG[row + height - 1][col - 1] + prefsumG[row - 1][col - 1];
                sumB = prefsumB[row + height - 1][col + width - 1] - prefsumB[row - 1][col + width - 1] - prefsumB[row + height - 1][col - 1] + prefsumB[row - 1][col - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1] - prefsumR2[row - 1][col + width - 1] - prefsumR2[row + height - 1][col - 1] + prefsumR2[row - 1][col - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1] - prefsumG2[row - 1][col + width - 1] - prefsumG2[row + height - 1][col - 1] + prefsumG2[row - 1][col - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1] - prefsumB2[row - 1][col + width - 1] - prefsumB2[row + height - 1][col - 1] + prefsumB2[row - 1][col - 1];
            }

            double n = width * height;
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;
            double varianceR = (sumR2 / n) - (avgR * avgR);
            double varianceG = (sumG2 / n) - (avgG * avgG);
            double varianceB = (sumB2 / n) - (avgB * avgB);

            return (varianceR + varianceG + varianceB) / 3.0;
        }
};

class MeanAbsoluteDeviation : public ErrorMethod {
    public:
        MeanAbsoluteDeviation() {
            upperThreshold = 127.5;
            lowerThreshold = 0;
        }
        
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            double sumAbsDevR = 0, sumAbsDevG = 0, sumAbsDevB = 0;
            double sumR = 0, sumG = 0, sumB = 0;
            
            // First pass: calculate average RGB values
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = currImgData[idx + 0];
                    uint8_t g = currImgData[idx + 1];
                    uint8_t b = currImgData[idx + 2];
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                }
            }
            
            int n = width * height;
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;
            
            // Second pass: calculate absolute deviations
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = currImgData[idx + 0];
                    uint8_t g = currImgData[idx + 1];
                    uint8_t b = currImgData[idx + 2];
                    
                    sumAbsDevR += fabs(r - avgR);
                    sumAbsDevG += fabs(g - avgG);
                    sumAbsDevB += fabs(b - avgB);
                }
            }
            
            double madR = sumAbsDevR / n;
            double madG = sumAbsDevG / n;
            double madB = sumAbsDevB / n;
            
            return (madR + madG + madB) / 3.0;
        }
};

class MaxPixelDifference : public ErrorMethod {
    public:
        MaxPixelDifference() {
            upperThreshold = 255.0;
            lowerThreshold = 0;
        }
        
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            uint8_t minR = 255, minG = 255, minB = 255;
            uint8_t maxR = 0, maxG = 0, maxB = 0;
            double sumR = 0, sumG = 0, sumB = 0;
            
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = currImgData[idx + 0];
                    uint8_t g = currImgData[idx + 1];
                    uint8_t b = currImgData[idx + 2];
                    
                    // Find min and max values for each channel
                    minR = std::min(minR, r);
                    minG = std::min(minG, g);
                    minB = std::min(minB, b);
                    
                    maxR = std::max(maxR, r);
                    maxG = std::max(maxG, g);
                    maxB = std::max(maxB, b);
                    
                    sumR += r;
                    sumG += g;
                    sumB += b;
                }
            }
            
            double diffR = maxR - minR;
            double diffG = maxG - minG;
            double diffB = maxB - minB;
            
            int n = width * height;
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;
            
            return (diffR + diffG + diffB) / 3.0;
        }
};

class Entropy : public ErrorMethod {
    public:
        Entropy() {
            upperThreshold = 8.0; // Example threshold value (max entropy for 8-bit values)
            lowerThreshold = 0;
        }
        
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            std::unordered_map<int, int> histR, histG, histB;
            double sumR = 0, sumG = 0, sumB = 0;
            int n = width * height;
            
            // Build histograms and calculate average values
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    uint8_t r = currImgData[idx + 0];
                    uint8_t g = currImgData[idx + 1];
                    uint8_t b = currImgData[idx + 2];
                    
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
            
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;
            
            return (entropyR + entropyG + entropyB) / 3.0;
        }
};

class SSIM : public ErrorMethod {
    private:
        const double C2 = 58.5225; // (0.03 * 255)^2
        
    public:
        SSIM() {
            upperThreshold = 1.0;
            lowerThreshold = 0;
        }
        
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            double sumR = 0, sumG = 0, sumB = 0;
            double sumSquareR = 0, sumSquareG = 0, sumSquareB = 0;
            int n = width * height;
            
            // Calculate means and variances in one pass
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    
                    double r = currImgData[idx + 0];
                    double g = currImgData[idx + 1];
                    double b = currImgData[idx + 2];
                    
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

            // In the standard SSIM formula, when comparing an image x with an image y:
            // SSIM(x,y) = [2μxμy + C1] / [μx² + μy² + C1] · [2σxy + C2] / [σx² + σy² + C2]

            // When y is a uniform block with the same mean as x (μy = μx and σy = 0), this simplifies to:
            // SSIM(x,uniform) = [2μx² + C1] / [2μx² + C1] · [C2] / [σx² + C2]

            // The first term equals 1, so:
            // SSIM(x,uniform) = C2 / (σx² + C2)
            
            double ssimR = C2 / (varR + C2);
            double ssimG = C2 / (varG + C2);
            double ssimB = C2 / (varB + C2);
            
            // Invert and average to get the error (1 - SSIM)
            double errorR = 1.0 - ssimR;
            double errorG = 1.0 - ssimG;
            double errorB = 1.0 - ssimB;
            
            avgR = meanR;
            avgG = meanG;
            avgB = meanB;
            
            return (errorR + errorG + errorB) / 3.0;;
        }
};

#endif