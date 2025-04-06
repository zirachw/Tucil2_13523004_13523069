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
        
            // Step 1: Compute average values
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
        
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;
        
            // Step 2: Build difference histograms
            for (int i = x; i < x + height; i++) {
                for (int j = y; j < y + width; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
        
                    int dr = static_cast<int>(currImgData[idx + 0]) - static_cast<int>(avgR);
                    int dg = static_cast<int>(currImgData[idx + 1]) - static_cast<int>(avgG);
                    int db = static_cast<int>(currImgData[idx + 2]) - static_cast<int>(avgB);
        
                    histR[dr]++;
                    histG[dg]++;
                    histB[db]++;
                }
            }
        
            // Step 3: Compute entropy
            auto computeEntropy = [&](const std::unordered_map<int, int>& hist) -> double {
                double entropy = 0;
                for (const auto& x : hist) {
                    int count = x.second;
                    double p = static_cast<double>(count) / n;
                    entropy -= p * std::log2(p);
                }
                return entropy;
            };
        
            double entropyR = computeEntropy(histR);
            double entropyG = computeEntropy(histG);
            double entropyB = computeEntropy(histB);
        
            return (entropyR + entropyG + entropyB) / 3.0;
        }
};

class SSIM : public ErrorMethod {
    private:
        const double C2 = 58.5225; // (0.03 * 255)^2
        double** sumR;
        double** sumG;
        double** sumB;
        double** sumR2;
        double** sumG2;
        double** sumB2;
        
    public:
        SSIM() {
            upperThreshold = 1.0;
            lowerThreshold = 0;

            sumR = new double*[imgHeight];
            sumG = new double*[imgHeight];
            sumB = new double*[imgHeight];
            sumR2 = new double*[imgHeight];
            sumG2 = new double*[imgHeight];
            sumB2 = new double*[imgHeight];

            for (int i = 0; i < imgHeight; ++i) {
                sumR[i] = new double[imgWidth];
                sumG[i] = new double[imgWidth];
                sumB[i] = new double[imgWidth];
                sumR2[i] = new double[imgWidth];
                sumG2[i] = new double[imgWidth];
                sumB2[i] = new double[imgWidth];
            }

            for (int i = 0; i < imgHeight; i++) {
                for (int j = 0; j < imgWidth; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;
                    sumR[i][j] = currImgData[idx + 0];
                    sumG[i][j] = currImgData[idx + 1];
                    sumB[i][j] = currImgData[idx + 2];
                    sumR2[i][j] = currImgData[idx + 0] * currImgData[idx + 0];
                    sumG2[i][j] = currImgData[idx + 1] * currImgData[idx + 1];
                    sumB2[i][j] = currImgData[idx + 2] * currImgData[idx + 2];

                    if (i == 0 && j == 0) {
                        continue;
                    }
                    else if (i == 0) {
                        sumR[i][j] += sumR[i][j - 1];
                        sumG[i][j] += sumG[i][j - 1];
                        sumB[i][j] += sumB[i][j - 1];
                        sumR2[i][j] += sumR2[i][j - 1];
                        sumG2[i][j] += sumG2[i][j - 1];
                        sumB2[i][j] += sumB2[i][j - 1];
                    }
                    else if (j == 0) {
                        sumR[i][j] += sumR[i - 1][j];
                        sumG[i][j] += sumG[i - 1][j];
                        sumB[i][j] += sumB[i - 1][j];
                        sumR2[i][j] += sumR2[i - 1][j];
                        sumG2[i][j] += sumG2[i - 1][j];
                        sumB2[i][j] += sumB2[i - 1][j];
                    }
                    else {
                        sumR[i][j] += sumR[i - 1][j] + sumR[i][j - 1] - sumR[i - 1][j - 1];
                        sumG[i][j] += sumG[i - 1][j] + sumG[i][j - 1] - sumG[i - 1][j - 1];
                        sumB[i][j] += sumB[i - 1][j] + sumB[i][j - 1] - sumB[i - 1][j - 1];
                        sumR2[i][j] += sumR2[i - 1][j] + sumR2[i][j - 1] - sumR2[i - 1][j - 1];
                        sumG2[i][j] += sumG2[i - 1][j] + sumG2[i][j - 1] - sumG2[i - 1][j - 1];
                        sumB2[i][j] += sumB2[i - 1][j] + sumB2[i][j - 1] - sumB2[i - 1][j - 1];
                    }
                }
            }
        }
        
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            int x1 = x;
            int y1 = y;
            int x2 = x + height - 1;
            int y2 = y + width - 1;
            int n = width * height;

            if (n == 0) {
                //cout << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
                //cout << "Error: Zero area in SSIM calculation." << endl;
                return 0;
            }
            if (x1 < 0 || y1 < 0 || x2 >= imgHeight || y2 >= imgWidth) {
               // cout << "Error: Out of bounds in SSIM calculation." << endl;
                return 0;
            }
            if (x1 > x2 || y1 > y2) {
               // cout << "Error: Invalid rectangle in SSIM calculation." << endl;
                return 0;
            }

            // Helper lambda to extract sum from integral image
            auto getSum = [&](double** integral) -> double {
                double total = integral[x2][y2];
                if (x1 > 0) total -= integral[x1 - 1][y2];
                if (y1 > 0) total -= integral[x2][y1 - 1];
                if (x1 > 0 && y1 > 0) total += integral[x1 - 1][y1 - 1];
                return total;
            };

            double sumRVal = getSum(sumR);
            double sumGVal = getSum(sumG);
            double sumBVal = getSum(sumB);
            double sumR2Val = getSum(sumR2);
            double sumG2Val = getSum(sumG2);
            double sumB2Val = getSum(sumB2);

            double meanR = sumRVal / n;
            double meanG = sumGVal / n;
            double meanB = sumBVal / n;

            double varR = (sumR2Val / n) - (meanR * meanR);
            double varG = (sumG2Val / n) - (meanG * meanG);
            double varB = (sumB2Val / n) - (meanB * meanB);

            double ssimR = C2 / (varR + C2);
            double ssimG = C2 / (varG + C2);
            double ssimB = C2 / (varB + C2);

            avgR = meanR;
            avgG = meanG;
            avgB = meanB;

            return (1.0 - ssimR + 1.0 - ssimG + 1.0 - ssimB) / 3.0;
        }
};

#endif