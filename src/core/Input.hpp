#ifndef INPUT_HPP
#define INPUT_HPP

#include <iostream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include "QuadTree.hpp"
#include "Image.hpp"

extern unsigned char* currImgData;
extern unsigned char* initImgData;
extern unsigned char* tempImgData;
extern int imgWidth, imgHeight, imgChannels;

using namespace std;

class Input {
    private:

        int mode, minBlock;
        double threshold, targetPercentage;
        string inputPath, inputExtension, outputPath, gifPath;

        // Helper function to extract extension from a path
        string getExtension(const string& path) {
            string extension = "";
            size_t dotPos = path.find_last_of('.');
            if (dotPos != string::npos) {
                extension = path.substr(dotPos + 1);
                return extension;
            }
            return "";
        }
        
        // Helper function to check if directory exists
        bool directoryExists(const string& path) {
            // Extract directory from path
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash == string::npos) {
                // No directory part, just a filename
                return true;
            }
            
            string directory = path.substr(0, lastSlash);
            if (directory.empty()) {
                return true;  // Root directory
            }
            
            struct stat info;
            if (stat(directory.c_str(), &info) != 0) {
                return false;  // Cannot access
            }
            
            return (info.st_mode & S_IFDIR) != 0;  // Is it a directory?
        }

        void validateInputPath() {
            string path;
            string extension;
            bool isValidFile = false;
            vector<string> allowedExtensions = {"jpg", "jpeg", "png"};
            
            while (!isValidFile) {
                cout << "Enter image input path" << endl;
                cout << "Example: D:\\something_in.jpg" << endl;
                cout << "Supported formats: jpg, jpeg, png" << endl;
                
                // Use getline instead of cin to handle paths with spaces
                getline(cin, path);
                
                // Check for empty input
                if (path.empty()) {
                    cout << "Error: Empty input. Please enter a path." << endl;
                    continue;
                }
                
                // Check if the file exists
                ifstream file(path);
                if (!file) {
                    cout << "File not found. Please enter a valid path." << endl;
                    continue;
                }
                
                // Extract and check extension
                extension = getExtension(path);
                if (extension.empty()) {
                    cout << "Error: No file extension found. Please include the file extension." << endl;
                    continue;
                }
                
                // Check if extension is allowed
                if (find(allowedExtensions.begin(), allowedExtensions.end(), extension) == allowedExtensions.end()) {
                    cout << "Invalid file format. Only jpg, jpeg, and png formats are supported." << endl;
                    continue;
                }
                
                // Use the static loadImage method directly
                if (!Image::loadImage(path)) {
                    cout << "Error loading image. Please check the file." << endl;
                    continue;
                }

                isValidFile = true;
            }
            
            this->inputPath = path;
            this->inputExtension = extension;
        }

        void validateMode() {
            string input;
            int modeValue;
            bool isValid = false;
            
            while (!isValid) {
                cout << "Enter mode:" << endl;
                cout << "1 - Variance" << endl;
                cout << "2 - Mean" << endl;
                cout << "3 - Max pixel difference" << endl;
                cout << "4 - Entropy" << endl;
                cout << "5 - SSIM" << endl;
                
                getline(cin, input);
                
                // Check for empty input
                if (input.empty()) {
                    cout << "Error: Empty input. Please enter a mode." << endl;
                    continue;
                }
                
                // Check for multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    cout << "Error: Multiple values entered. Please enter a single number." << endl;
                    continue;
                }
                
                // Check for non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789-") != string::npos) {
                    cout << "Error: Non-numeric character found. Please enter numbers only." << endl;
                    continue;
                }
                
                // Check the valid range
                modeValue = stoi(input);
                if (modeValue < 1 || modeValue > 5) {
                    cout << "Error: Invalid mode. Please enter a number between 1 and 5." << endl;
                    continue;
                }
                
                isValid = true;
            }
            
            this->mode = modeValue;
        }

        void validateThreshold() {
            string input;
            double thresholdValue;
            bool isValid = false;
            double upperThreshold = 0;
            double lowerThreshold = 0;
            
            switch(mode) {
                case 1: {
                    Variance variance;
                    upperThreshold = variance.getUpperThreshold();
                    lowerThreshold = variance.getLowerThreshold();
                    break;
                }
                case 2: {
                    MeanAbsoluteDeviation mad;
                    upperThreshold = mad.getUpperThreshold();
                    lowerThreshold = mad.getLowerThreshold();
                    break;
                }
                case 3: {
                    MaxPixelDifference mpd;
                    upperThreshold = mpd.getUpperThreshold();
                    lowerThreshold = mpd.getLowerThreshold();
                    break;
                }
                case 4: {
                    Entropy entropy;
                    upperThreshold = entropy.getUpperThreshold();
                    lowerThreshold = entropy.getLowerThreshold();
                    break;
                }
                case 5: {
                    SSIM ssim;
                    upperThreshold = ssim.getUpperThreshold();
                    lowerThreshold = ssim.getLowerThreshold();
                    break;
                }
            }
            
            while (!isValid) {
                cout << "Enter threshold (" << lowerThreshold << "-" << upperThreshold << ")" << endl;
                cout << "Example: " << (lowerThreshold + upperThreshold) / 2 << endl;
                
                getline(cin, input);
                
                // Check for empty input
                if (input.empty()) {
                    cout << "Error: Empty input. Please enter a threshold value." << endl;
                    continue;
                }
                
                // Check for multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    cout << "Error: Multiple values entered. Please enter a single number." << endl;
                    continue;
                }
                
                // Check for non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789-") != string::npos) {
                    cout << "Error: Non-numeric character found. Please enter numbers only." << endl;
                    continue;
                }
                
                thresholdValue = stod(input);

                if (thresholdValue < lowerThreshold || thresholdValue > upperThreshold) {
                    cout << "Error: Invalid threshold. Please enter a number between " << lowerThreshold << " and " << upperThreshold << "." << endl;
                    continue;
                }
            
                isValid = true;
            }
            
            this->threshold = thresholdValue;
        }

        void validateMinBlock() {
            
            string input;
            int blockSize;
            bool isValid = false;
            
            // Calculate the smallest dimension of the image
            int minDimension = min(imgWidth, imgHeight);
            
            while (!isValid) {
                cout << "Enter minimum block size, must not exceed the smallest image dimension (" << minDimension << ")" << endl;
                cout << "Example: 8" << endl;
                
                getline(cin, input);
                
                // Check for empty input
                if (input.empty()) {
                    cout << "Error: Empty input. Please enter a block size." << endl;
                    continue;
                }
                
                // Check for multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    cout << "Error: Multiple values entered. Please enter a single number." << endl;
                    continue;
                }
                
                // Check for non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789-") != string::npos) {
                    cout << "Error: Non-numeric character found. Please enter numbers only." << endl;
                    continue;
                }
            
                blockSize = stoi(input);
                
                // Check if the number is greater than 0
                if (blockSize <= 0) {
                    cout << "Error: Invalid block size. Please enter a positive number." << endl;
                    continue;
                }
                
                // Check if it exceeds the image dimension
                if (blockSize > minDimension) {
                    cout << "Error: Block size cannot be greater than the smallest image dimension (" << minDimension << ")." << endl;
                    continue;
                }
                
                isValid = true;
            }
            
            this->minBlock = blockSize;
        }
        
        void validateTargetPercentage() {
            string input;
            double percentValue;
            bool isValid = false;
            
            while (!isValid) {
                cout << "Enter target percentage as a decimal (0.0-1.0 where 1.0 = 100%)" << endl;
                cout << "Example: 0.85 for 85%" << endl;
                
                getline(cin, input);
                
                // Check for empty input
                if (input.empty()) {
                    cout << "Error: Empty input. Please enter a value." << endl;
                    continue;
                }
                
                // Check for multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    cout << "Error: Multiple values entered. Please enter a single number." << endl;
                    continue;
                }
                
                // Check for non-numeric values (allow minus sign and decimal point)
                if (input.find_first_not_of("0123456789.-") != string::npos) {
                    cout << "Error: Non-numeric character found. Please enter numbers and decimal point only." << endl;
                    continue;
                }
                
                // Check for multiple decimal points
                if (count(input.begin(), input.end(), '.') > 1) {
                    cout << "Error: Multiple decimal points found. Please enter a valid decimal number." << endl;
                    continue;
                }
                
                // Now check the regex pattern for a valid decimal number (including negative numbers)
                if (!regex_match(input, regex("^-?[0-9]+(\\.[0-9]+)?$"))) {
                    cout << "Error: Invalid format. Please enter a valid decimal number." << endl;
                    continue;
                }
                
                percentValue = stod(input);

                if (percentValue < 0.0 || percentValue > 1.0) {
                    cout << "Error: Invalid percentage. Please enter a number between 0.0 and 1.0." << endl;
                    continue;
                }
                
                isValid = true;
            }
            
            this->targetPercentage = percentValue;
        }

        void validateOutputPath() {
            
            string path;
            string extension;
            bool isValidPath = false;
            
            while (!isValidPath) {
                cout << "Enter image output path" << endl;
                cout << "Example: D:\\something_out." << inputExtension << endl;
                cout << "Output must use the same format as input: " << inputExtension << endl;
                
                getline(cin, path);
                
                // Check for empty input
                if (path.empty()) {
                    cout << "Error: Empty input. Please enter an output path." << endl;
                    continue;
                }
                
                // Extract and check extension
                extension = getExtension(path);
                if (extension.empty()) {
                    cout << "Error: No file extension found. Please include the ." << inputExtension << " extension." << endl;
                    continue;
                }
                
                // Check if extension matches input
                if (extension != inputExtension) {
                    cout << "Error: Output format must match input format (." << inputExtension << ")." << endl;
                    continue;
                }
                
                // Check if directory exists
                if (!directoryExists(path)) {
                    cout << "Error: Directory does not exist. Please enter a valid path." << endl;
                    continue;
                }
                
                // Check if output file is the same as input file
                if (path == inputPath) {
                    char confirm;
                    bool validResponse = false;
                    
                    while (!validResponse) {
                        cout << "Warning: Output file is the same as input file. This will overwrite your original image." << endl;
                        cout << "Do you want to continue? (Y/N): ";
                        
                        string response;
                        getline(cin, response);
                        
                        if (response.empty()) {
                            cout << "Error: Empty input. Please enter Y or N." << endl;
                            continue;
                        }
                        
                        if (response.length() != 1) {
                            cout << "Error: Invalid input. Please enter only Y or N." << endl;
                            continue;
                        }
                        
                        confirm = response[0];
                        if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                            validResponse = true;
                        } else {
                            cout << "Error: Invalid input. Please enter only Y or N." << endl;
                        }
                    }
                    
                    if (tolower(confirm) != 'y') {
                        cout << "Please enter a different output path." << endl;
                        continue;
                    }
                    
                    cout << "Original file will be overwritten." << endl;
                }
                
                isValidPath = true;
            }
            
            this->outputPath = path;
        }

        void validateGifPath() {
            string path;
            string extension;
            bool isValidPath = false;
            
            while (!isValidPath) {
                cout << "Enter gif output path" << endl;
                cout << "Example: D:\\something_out.gif" << endl;
                
                getline(cin, path);
                
                // Check for empty input
                if (path.empty()) {
                    cout << "Error: Empty input. Please enter a gif output path." << endl;
                    continue;
                }
                
                // Check file extension
                extension = getExtension(path);
                if (extension.empty()) {
                    cout << "Error: No file extension found. Please include the .gif extension." << endl;
                    continue;
                }
                
                if (extension != "gif") {
                    cout << "Error: Invalid file format. Only gif format is supported." << endl;
                    continue;
                }
                
                // Check if directory exists
                if (!directoryExists(path)) {
                    cout << "Error: Directory does not exist. Please enter a valid path." << endl;
                    continue;
                }
                
                // Check if gif file already exists
                ifstream file(path);
                if (file) {
                    char confirm;
                    bool validResponse = false;
                    
                    while (!validResponse) {
                        cout << "Warning: GIF file already exists. This will overwrite the existing file." << endl;
                        cout << "Do you want to continue? (Y/N): ";
                        
                        string response;
                        getline(cin, response);
                        
                        if (response.empty()) {
                            cout << "Error: Empty input. Please enter Y or N." << endl;
                            continue;
                        }
                        
                        if (response.length() != 1) {
                            cout << "Error: Invalid input. Please enter only Y or N." << endl;
                            continue;
                        }
                        
                        confirm = response[0];
                        if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                            validResponse = true;
                        } else {
                            cout << "Error: Invalid input. Please enter only Y or N." << endl;
                        }
                    }
                    
                    if (tolower(confirm) != 'y') {
                        cout << "Please enter a different gif path." << endl;
                        continue;
                    }
                    
                    cout << "Existing GIF file will be overwritten." << endl;
                }
                
                isValidPath = true;
            }
            
            this->gifPath = path;
        }

    public:

        Input() {
            validateInputPath();
            validateMode();
            validateThreshold();
            validateMinBlock();
            validateTargetPercentage();
            validateOutputPath();
            validateGifPath();            
        }
        
        ~Input() {}

        string getInputPath() {return inputPath;}
        string getInputExtension() {return inputExtension;}
        int getMode() {return mode;}
        double getThreshold() {return threshold;}
        int getMinBlock() {return minBlock;}
        double getTargetPercentage() {return targetPercentage;}
        string getOutputPath() {return outputPath;}
        string getGifPath() {return gifPath;}
};

#endif