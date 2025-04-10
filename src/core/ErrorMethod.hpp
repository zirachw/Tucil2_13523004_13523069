#ifndef ERROR_METHODS_HPP
#define ERROR_METHODS_HPP

// Libraries
#include "Image.hpp"
#include <unordered_map>

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
 * @brief Abstract base class for different error calculation methods
 * @param upperThreshold Upper limit for error value
 * @param lowerThreshold Lower limit for error value
 * @param avgR Average red channel value for the region
 * @param avgG Average green channel value for the region
 * @param avgB Average blue channel value for the region
 */
class ErrorMethod {

    protected:  
        double upperThreshold;
        double lowerThreshold;
        double avgR, avgG, avgB;

    public:
        /**
         * @brief Calculate error for a specific region of an image, overridden by derived classes
         * @param currImgData Pointer to image data
         * @param x Starting x-coordinate
         * @param y Starting y-coordinate
         * @param width Width of the region
         * @param height Height of the region
         * @return Error value calculated for the region using the specific method (derived class)
         */
        virtual double calculateError(unsigned char* currImgData, int x, int y, int width, int height) = 0;
        
        /**
         * @brief Get the average red value of the last processed region
         * @return Average red channel value
         */
        double getAvgR() const { return avgR; }

        /**
         * @brief Get the average green value of the last processed region
         * @return Average green channel value
         */
        double getAvgG() const { return avgG; }

        /**
         * @brief Get the average blue value of the last processed region
         * @return Average blue channel value
         */
        double getAvgB() const { return avgB; }

        /**
         * @brief Get the upper threshold for this error method
         * @return Upper threshold value
         */
        double getUpperThreshold() const { return upperThreshold; }

        /**
         * @brief Get the lower threshold for this error method
         * @return Lower threshold value
         */
        double getLowerThreshold() const { return lowerThreshold; }
};

/**
 * @brief Implements error calculation using variance of pixel values
 * @param prefsumR 2D array for red channel prefix sum
 * @param prefsumG 2D array for green channel prefix sum
 * @param prefsumB 2D array for blue channel prefix sum
 * @param prefsumR2 2D array for squared red channel prefix sum
 * @param prefsumG2 2D array for squared green channel prefix sum
 * @param prefsumB2 2D array for squared blue channel prefix sum
 */
class Variance : public ErrorMethod {
    
    private:
        long long** prefsumR;
        long long** prefsumG;
        long long** prefsumB;
        long long** prefsumR2;
        long long** prefsumG2;
        long long** prefsumB2;

    public:
        /**
         * @brief Default Constructor that initializes prefix sum arrays
         */
        Variance() {

            // Default values for upper and lower thresholds in Variance method
            upperThreshold = 127.5 * 127.5;
            lowerThreshold = 0;

            // Initialize prefix sum arrays
            prefsumR = new long long*[imgHeight];
            prefsumG = new long long*[imgHeight];
            prefsumB = new long long*[imgHeight];
            prefsumR2 = new long long*[imgHeight];
            prefsumG2 = new long long*[imgHeight];
            prefsumB2 = new long long*[imgHeight];
            
            // Allocate memory for each row of the prefix sum arrays
            for (int i = 0; i < imgHeight; ++i) {
                prefsumR[i] = new long long[imgWidth];
                prefsumG[i] = new long long[imgWidth];
                prefsumB[i] = new long long[imgWidth];
                prefsumR2[i] = new long long[imgWidth];
                prefsumG2[i] = new long long[imgWidth];
                prefsumB2[i] = new long long[imgWidth];
            }

            // Calculate prefix sums for the image data
            for (int i = 0; i < imgHeight; i++) {
                for (int j = 0; j < imgWidth; j++) {
        
                    int idx = (i * imgWidth + j) * imgChannels;
                   
                    // Initialize prefix sum arrays with pixel values
                    prefsumR[i][j] = currImgData[idx + 0];
                    prefsumG[i][j] = currImgData[idx + 1];
                    prefsumB[i][j] = currImgData[idx + 2];
                    prefsumR2[i][j] = currImgData[idx + 0] * currImgData[idx + 0];
                    prefsumG2[i][j] = currImgData[idx + 1] * currImgData[idx + 1];
                    prefsumB2[i][j] = currImgData[idx + 2] * currImgData[idx + 2];
                    
                    // For the top-left pixel, no need to add anything
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    // For the first row, add the previous pixel value
                    else if (i == 0) {
                        prefsumR[i][j] += prefsumR[i][j - 1];
                        prefsumG[i][j] += prefsumG[i][j - 1];
                        prefsumB[i][j] += prefsumB[i][j - 1];
                        prefsumR2[i][j] += prefsumR2[i][j - 1];
                        prefsumG2[i][j] += prefsumG2[i][j - 1];
                        prefsumB2[i][j] += prefsumB2[i][j - 1];
                    }

                    // For the first column, add the previous pixel value
                    else if (j == 0) {
                        prefsumR[i][j] += prefsumR[i - 1][j];
                        prefsumG[i][j] += prefsumG[i - 1][j];
                        prefsumB[i][j] += prefsumB[i - 1][j];
                        prefsumR2[i][j] += prefsumR2[i - 1][j];
                        prefsumG2[i][j] += prefsumG2[i - 1][j];
                        prefsumB2[i][j] += prefsumB2[i - 1][j];
                    }

                    // For all other pixels, add the values from the left and top pixels
                    else {
                        prefsumR[i][j] += prefsumR[i - 1][j] + prefsumR[i][j - 1] - prefsumR[i - 1][j - 1];
                        prefsumG[i][j] += prefsumG[i - 1][j] + prefsumG[i][j - 1] - prefsumG[i - 1][j - 1];
                        prefsumB[i][j] += prefsumB[i - 1][j] + prefsumB[i][j - 1] - prefsumB[i - 1][j - 1];
                        prefsumR2[i][j] += prefsumR2[i - 1][j] + prefsumR2[i][j - 1] - prefsumR2[i - 1][j - 1];
                        prefsumG2[i][j] += prefsumG2[i - 1][j] + prefsumG2[i][j - 1] - prefsumG2[i - 1][j - 1];
                        prefsumB2[i][j] += prefsumB2[i - 1][j] + prefsumB2[i][j - 1] - prefsumB2[i - 1][j - 1];
                    }
                }
            }
        }

        /**
         * @brief Destructor that cleans up allocated prefix sum arrays
         */
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
        
        /**
         * @brief Calculate variance-based error for a region
         * @param currImgData Pointer to image data
         * @param row Starting row
         * @param col Starting column
         * @param width Width of the region
         * @param height Height of the region
         * @return Average variance across RGB channels
         */
        double calculateError(unsigned char* currImgData, int row, int col, int width, int height) override {
            long long sumR, sumG, sumB;
            long long sumR2, sumG2, sumB2;

            // If the width or height is zero, return 0 to avoid division by zero (or invalid area)
            if (width == 0 || height == 0) {
                return 0;
            }
            
            // Top-left corner, use the prefix sum directly
            if (row == 0 && col == 0) {
                sumR = prefsumR[row + height - 1][col + width - 1];
                sumG = prefsumG[row + height - 1][col + width - 1];
                sumB = prefsumB[row + height - 1][col + width - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1];
            }

            // First row
            else if (row == 0) {
                sumR = prefsumR[row + height - 1][col + width - 1] - prefsumR[row + height - 1][col - 1];
                sumG = prefsumG[row + height - 1][col + width - 1] - prefsumG[row + height - 1][col - 1];
                sumB = prefsumB[row + height - 1][col + width - 1] - prefsumB[row + height - 1][col - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1] - prefsumR2[row + height - 1][col - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1] - prefsumG2[row + height - 1][col - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1] - prefsumB2[row + height - 1][col - 1];
            }

            // First column
            else if (col == 0) {
                sumR = prefsumR[row + height - 1][col + width - 1] - prefsumR[row - 1][col + width - 1];
                sumG = prefsumG[row + height - 1][col + width - 1] - prefsumG[row - 1][col + width - 1];
                sumB = prefsumB[row + height - 1][col + width - 1] - prefsumB[row - 1][col + width - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1] - prefsumR2[row - 1][col + width - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1] - prefsumG2[row - 1][col + width - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1] - prefsumB2[row - 1][col + width - 1];
            }

            // For all other pixels, use the prefix sum formula
            else {
                sumR = prefsumR[row + height - 1][col + width - 1] - prefsumR[row - 1][col + width - 1] - prefsumR[row + height - 1][col - 1] + prefsumR[row - 1][col - 1];
                sumG = prefsumG[row + height - 1][col + width - 1] - prefsumG[row - 1][col + width - 1] - prefsumG[row + height - 1][col - 1] + prefsumG[row - 1][col - 1];
                sumB = prefsumB[row + height - 1][col + width - 1] - prefsumB[row - 1][col + width - 1] - prefsumB[row + height - 1][col - 1] + prefsumB[row - 1][col - 1];

                sumR2 = prefsumR2[row + height - 1][col + width - 1] - prefsumR2[row - 1][col + width - 1] - prefsumR2[row + height - 1][col - 1] + prefsumR2[row - 1][col - 1];
                sumG2 = prefsumG2[row + height - 1][col + width - 1] - prefsumG2[row - 1][col + width - 1] - prefsumG2[row + height - 1][col - 1] + prefsumG2[row - 1][col - 1];
                sumB2 = prefsumB2[row + height - 1][col + width - 1] - prefsumB2[row - 1][col + width - 1] - prefsumB2[row + height - 1][col - 1] + prefsumB2[row - 1][col - 1];
            }

            // Calculate average values for each channel
            double n = width * height;
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;

            // Calculate variance for each channel
            double varianceR = (sumR2 / n) - (avgR * avgR);
            double varianceG = (sumG2 / n) - (avgG * avgG);
            double varianceB = (sumB2 / n) - (avgB * avgB);

            // Calculate the final error value as the average of variances across all channels
            return (varianceR + varianceG + varianceB) / 3.0;
        }
};

/**
 * @brief Implements error calculation using mean absolute deviation
 */
class MeanAbsoluteDeviation : public ErrorMethod {

    public:
        /**
         * @brief Default Constructor that sets appropriate thresholds for MAD
         */
        MeanAbsoluteDeviation() {
            upperThreshold = 127.5;
            lowerThreshold = 0;
        }
        
        /**
         * @brief Calculate mean absolute deviation for a region
         * @param currImgData Pointer to image data
         * @param x Starting x-coordinate
         * @param y Starting y-coordinate
         * @param width Width of the region
         * @param height Height of the region
         * @return Average MAD across RGB channels
         */
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            double sumAbsDevR = 0, sumAbsDevG = 0, sumAbsDevB = 0;
            double sumR = 0, sumG = 0, sumB = 0;
            
            // Calculate average values for each channel
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
            
            // Calculate absolute deviations for each channel
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
            
            // Calculate the final error value as the average of MADs across all channels
            return (madR + madG + madB) / 3.0;
        }
};

/**
 * @brief Implements error calculation using maximum pixel difference
 */
class MaxPixelDifference : public ErrorMethod {

    public:
        /**
         * @brief Default Constructor that sets appropriate thresholds for max pixel difference
         */
        MaxPixelDifference() {
            upperThreshold = 255.0;
            lowerThreshold = 0;
        }
        
        /**
         * @brief Calculate maximum pixel difference for a region
         * @param currImgData Pointer to image data
         * @param x Starting x-coordinate
         * @param y Starting y-coordinate
         * @param width Width of the region
         * @param height Height of the region
         * @return Average MPD across RGB channels
         */
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            uint8_t minR = 255, minG = 255, minB = 255;
            uint8_t maxR = 0, maxG = 0, maxB = 0;
            double sumR = 0, sumG = 0, sumB = 0;
            
            // Calculate min and max values for each channel
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
            
            // Calculate the differences for each channel
            double diffR = maxR - minR;
            double diffG = maxG - minG;
            double diffB = maxB - minB;

            // Assign average values for each channel
            int n = width * height;
            avgR = sumR / n;
            avgG = sumG / n;
            avgB = sumB / n;
            
            // Calculate the final error value as the average of differences across all channels
            return (diffR + diffG + diffB) / 3.0;
        }
};

/**
 * @brief Implements error calculation using entropy of pixel values
 */
class Entropy : public ErrorMethod {

    public:
        /**
         * @brief Default Constructor that sets appropriate thresholds for entropy
         * @param upperThreshold 8.0 (maximum entropy for 8-bit channels, log2(256) = 8)
         * @param lowerThreshold 0 (minimum entropy, no variation in pixel values)
         */
        Entropy() {
            upperThreshold = 8.0;
            lowerThreshold = 0;
        }
    
        /**
         * @brief Calculate entropy-based error for a region
         * @param currImgData Pointer to image data
         * @param x Starting x-coordinate
         * @param y Starting y-coordinate
         * @param width Width of the region
         * @param height Height of the region
         * @return Average entropy across RGB channels
         */
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            std::unordered_map<int, int> histR, histG, histB;
            double sumR = 0, sumG = 0, sumB = 0;
            int n = width * height;
        
            // Calculate average values for each channel
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
        
            // Build histograms for each channel
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
        
            // Calculate entropy for each channel
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
        
            // Calculate the final error value as the average of entropies across all channels
            return (entropyR + entropyG + entropyB) / 3.0;
        }
};

/**
 * @brief Implements error calculation using Structural Similarity Index
 * @param C2 Constant for stability in SSIM calculation, (0.03 * 225)^2 = 58.5225
 * @param sumR 2D array for red channel sum
 * @param sumG 2D array for green channel sum
 * @param sumB 2D array for blue channel sum
 * @param sumR2 2D array for squared red channel sum
 * @param sumG2 2D array for squared green channel sum
 * @param sumB2 2D array for squared blue channel sum
 */
class SSIM : public ErrorMethod {

    private:
        const double C2 = 58.5225;
        double** sumR;
        double** sumG;
        double** sumB;
        double** sumR2;
        double** sumG2;
        double** sumB2;
        
    public:
        /**
         * @brief Constructor that initializes integral image arrays
         */
        SSIM() {
            // Default values for upper and lower thresholds in SSIM method
            upperThreshold = 1.0;
            lowerThreshold = 0;

            // Initialize integral image arrays
            sumR = new double*[imgHeight];
            sumG = new double*[imgHeight];
            sumB = new double*[imgHeight];
            sumR2 = new double*[imgHeight];
            sumG2 = new double*[imgHeight];
            sumB2 = new double*[imgHeight];

            // Allocate memory for each row of the integral image arrays
            for (int i = 0; i < imgHeight; ++i) {
                sumR[i] = new double[imgWidth];
                sumG[i] = new double[imgWidth];
                sumB[i] = new double[imgWidth];
                sumR2[i] = new double[imgWidth];
                sumG2[i] = new double[imgWidth];
                sumB2[i] = new double[imgWidth];
            }

            // Calculate integral images for the current image data
            for (int i = 0; i < imgHeight; i++) {
                for (int j = 0; j < imgWidth; j++) {
                    int idx = (i * imgWidth + j) * imgChannels;

                    // Initialize integral image arrays with pixel values
                    sumR[i][j] = currImgData[idx + 0];
                    sumG[i][j] = currImgData[idx + 1];
                    sumB[i][j] = currImgData[idx + 2];
                    sumR2[i][j] = currImgData[idx + 0] * currImgData[idx + 0];
                    sumG2[i][j] = currImgData[idx + 1] * currImgData[idx + 1];
                    sumB2[i][j] = currImgData[idx + 2] * currImgData[idx + 2];

                    // For the top-left pixel, no need to add anything
                    if (i == 0 && j == 0) {
                        continue;
                    }

                    // For the first row, only add the previous column pixel value
                    else if (i == 0) {
                        sumR[i][j] += sumR[i][j - 1];
                        sumG[i][j] += sumG[i][j - 1];
                        sumB[i][j] += sumB[i][j - 1];
                        sumR2[i][j] += sumR2[i][j - 1];
                        sumG2[i][j] += sumG2[i][j - 1];
                        sumB2[i][j] += sumB2[i][j - 1];
                    }

                    // For the first column, add the previous row pixel value
                    else if (j == 0) {
                        sumR[i][j] += sumR[i - 1][j];
                        sumG[i][j] += sumG[i - 1][j];
                        sumB[i][j] += sumB[i - 1][j];
                        sumR2[i][j] += sumR2[i - 1][j];
                        sumG2[i][j] += sumG2[i - 1][j];
                        sumB2[i][j] += sumB2[i - 1][j];
                    }
                    
                    // For all other pixels, add the values from the left and top pixels
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
        
        /**
         * @brief Calculate SSIM-based error for a region
         * @param currImgData Pointer to image data
         * @param x Starting x-coordinate
         * @param y Starting y-coordinate
         * @param width Width of the region
         * @param height Height of the region
         * @return Inverse of average SSIM across RGB channels
         */
        double calculateError(unsigned char* currImgData, int x, int y, int width, int height) override {
            int x1 = x;
            int y1 = y;
            int x2 = x + height - 1;
            int y2 = y + width - 1;
            int n = width * height;

            // Check for invalid region dimensions
            if (n == 0) {
                return 0;
            }
            if (x1 < 0 || y1 < 0 || x2 >= imgHeight || y2 >= imgWidth) {
                return 0;
            }
            if (x1 > x2 || y1 > y2) {
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
            
            // Calculate sums for each channel using the integral images
            double sumRVal = getSum(sumR);
            double sumGVal = getSum(sumG);
            double sumBVal = getSum(sumB);
            double sumR2Val = getSum(sumR2);
            double sumG2Val = getSum(sumG2);
            double sumB2Val = getSum(sumB2);

            // Calculate mean values for each channel
            avgR = sumRVal / n;
            avgG = sumGVal / n;
            avgB = sumBVal / n;

            // Calculate variance for each channel
            double varR = (sumR2Val / n) - (avgR * avgR);
            double varG = (sumG2Val / n) - (avgG * avgG);
            double varB = (sumB2Val / n) - (avgB * avgB);

            // Calculate the SSIM value across all channels
            double ssimR = C2 / (varR + C2);
            double ssimG = C2 / (varG + C2);
            double ssimB = C2 / (varB + C2);

            // Calculate the final error value as the inverse of average SSIM across all channels
            return (1.0 - ssimR + 1.0 - ssimG + 1.0 - ssimB) / 3.0;
        }
};

#endif