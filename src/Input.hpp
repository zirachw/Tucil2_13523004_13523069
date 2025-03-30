#ifndef INPUT_HPP
#define INPUT_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <limits>
#include <regex>
#include <sys/stat.h>
#include "QuadTree.hpp"

extern unsigned char* imgData;
extern unsigned char* imgReal;
extern unsigned char* img2Data;
int imgWidth, imgHeight, imgChannels;

using namespace std;

class Input {
private:
    string imageInputPath;
    string inputExtension;
    int mode;
    int threshold;
    int minimumBlock;
    double targetPercentage;
    string imageOutputPath;
    string gifOutputPath;
    
    // Image properties
    int width = 0;
    int height = 0;
    int channel = 0;
    bool imageLoaded = false;
    
    // Helper function to extract extension from a path
    string getExtension(const string& path) {
        string extension = "";
        size_t dotPos = path.find_last_of('.');
        if (dotPos != string::npos) {
            extension = path.substr(dotPos + 1);
            transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
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

    // Load image from the validated path
    void getImage() {
        imgData = stbi_load(imageInputPath.c_str(), &width, &height, &channel, 3);
        if (!imgData) {
            cout << "Could not open or find the image\n";
            imageLoaded = false;
            return;
        }
    
        channel = 3;
        imgReal = (unsigned char*) malloc (width * height * 3);
        memcpy(imgReal, imgData, width * height * 3);
    
        img2Data = (unsigned char*) malloc (width * height * 3);
        memcpy(img2Data, imgData, width * height * 3);

        imgWidth = width;
        imgHeight = height;
        imgChannels = channel;
        imageLoaded = true;
        cout << "Image loaded successfully\n";
        cout << "Image size: " << width << "x" << height << ", Channels: " << channel << endl;
    }

public:

    Input() : imageLoaded(false), width(0), height(0), channel(0) {}
    
    ~Input() {
    }

    string getImageInputPath() {
        string path;
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
            string extension = getExtension(path);
            if (extension.empty()) {
                cout << "Error: No file extension found. Please include the file extension." << endl;
                continue;
            }
            
            // Check if extension is allowed
            if (find(allowedExtensions.begin(), allowedExtensions.end(), extension) == allowedExtensions.end()) {
                cout << "Invalid file format. Only jpg, jpeg, and png formats are supported." << endl;
                continue;
            }
            
            isValidFile = true;
            
            // Store the input file extension
            this -> inputExtension = extension;
        }
        
        this -> imageInputPath = path;
        
        // Load the image immediately after valid path is obtained
        getImage();
        
        return imageInputPath;
    }

    int getMode() {
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
        
        this -> mode = modeValue;
        return modeValue;
    }

    int getThreshold() {
        string input;
        int thresholdValue;
        bool isValid = false;
        
        while (!isValid) {
            cout << "Enter threshold (0-255)" << endl;
            cout << "Example: 100" << endl;
            
            getline(cin, input);
            
            // Check for empty input
            if (input.empty()) {
                cout << "Error: Empty input. Please enter a threshold value." << endl;
                continue;
            }
            
            // Check for multiple inputs (whitespace) FIRST
            if (input.find(' ') != string::npos) {
                cout << "Error: Multiple values entered. Please enter a single number." << endl;
                continue;
            }
            
            // Check for non-numeric characters SECOND (allow minus sign)
            if (input.find_first_not_of("0123456789-") != string::npos) {
                cout << "Error: Non-numeric character found. Please enter numbers only." << endl;
                continue;
            }
            
            // Check for valid range
            try {
                thresholdValue = stoi(input);
                if (thresholdValue < 0 || thresholdValue > 255) {
                    cout << "Error: Invalid threshold. Please enter a number between 0 and 255." << endl;
                    continue;
                }
            } 
            catch (const exception& e) {
                cout << "Error: Invalid input. Please enter a valid number." << endl;
                continue;
            }
            
            isValid = true;
        }
        
        this -> threshold = thresholdValue;
        return thresholdValue;
    }

    int getMinimumBlock() {
        // Check if image is loaded
        if (!imageLoaded) {
            cout << "Error: No image loaded. Please load an image first." << endl;
            return -1;
        }
        
        string input;
        int blockSize;
        bool isValid = false;
        
        // Calculate the smallest dimension of the image
        int minDimension = min(width, height);
        
        while (!isValid) {
            cout << "Enter minimum block size (must be a power of 2, e.g., 2, 4, 8, 16, ...)" << endl;
            cout << "Value must not exceed the smallest image dimension (" << minDimension << ")" << endl;
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
            
            try {
                blockSize = stoi(input);
                
                // Check if the number is greater than 0
                if (blockSize <= 0) {
                    cout << "Error: Invalid block size. Please enter a positive number." << endl;
                    continue;
                }
                
                // Check if the number is a power of 2
                if ((blockSize & (blockSize - 1)) != 0) {
                    cout << "Error: Invalid block size. Please enter a power of 2 (2, 4, 8, 16, etc.)." << endl;
                    continue;
                }
                
                // Check if it exceeds the image dimension
                if (blockSize > minDimension) {
                    cout << "Error: Block size cannot be greater than the smallest image dimension (" << minDimension << ")." << endl;
                    continue;
                }
                
                isValid = true;
            } 
            catch (const exception& e) {
                cout << "Error: Invalid input. Please enter a valid number." << endl;
                continue;
            }
        }
        
        this -> minimumBlock = blockSize;
        return blockSize;
    }
    
    double getTargetPercentage() {
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
            
            try {
                percentValue = stod(input);
                if (percentValue < 0.0 || percentValue > 1.0) {
                    cout << "Error: Invalid percentage. Please enter a number between 0.0 and 1.0." << endl;
                    continue;
                }
            } 
            catch (const exception& e) {
                cout << "Error: Invalid input. Please enter a valid number." << endl;
                continue;
            }
            
            isValid = true;
        }
        
        this -> targetPercentage = percentValue;
        return percentValue;
    }

    string getImageOutputPath() {
        if (inputExtension.empty()) {
            cout << "Error: Input image not loaded. Please load an input image first." << endl;
            return "";
        }
        
        string path;
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
            string extension = getExtension(path);
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
            if (path == imageInputPath) {
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
        
        this -> imageOutputPath = path;
        return path;
    }

    string getGifOutputPath() {
        string path;
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
            string extension = getExtension(path);
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
            
            isValidPath = true;
        }
        
        this -> gifOutputPath = path;
        return path;
    }
};

#endif