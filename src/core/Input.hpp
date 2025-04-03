#ifndef INPUT_HPP
#define INPUT_HPP

#include <iostream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include "QuadTree.hpp"

extern unsigned char* currImgData;
extern unsigned char* initImgData;
extern unsigned char* tempImgData;
extern int imgWidth, imgHeight, imgChannels;

using namespace std;

class Input {
    private:

        int mode, minBlock;
        double threshold, targetPercentage;
        string inputPath, inputExtension, errorMethod, outputPath, gifPath;

        // Helper function to extract filename with extension from a path
        string getFilename(const string& path) {
            size_t lastSlash = path.find_last_of("/\\");
            
            // Extract the filename part (everything after the last slash)
            string filename = path.substr(lastSlash + 1);
            return filename;
        }

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
        
        // Check if the path is a fully qualified path (has drive letter and directory structure)
        // For Windows, check for drive letter format (e.g., "D:\...")
        bool validatePath(const string& path) {
            if (path.length() < 3 || path[1] != ':' || (path[2] != '\\' && path[2] != '/')) {
                return false;
            }
            
            // Extract directory part from the path
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash == string::npos) { 
                return false;
            }
            
            // For root directory (e.g., "D:\file.jpeg"), the directory is "D:\"
            string directory;
            if (lastSlash <= 2) {
                directory = path.substr(0, 3);  // Take "D:\"
            } 
            else {
                // This is a file in a subdirectory
                directory = path.substr(0, lastSlash);
            }
            
            // Directory exists or not?
            struct stat info;
            if (stat(directory.c_str(), &info) != 0) {
                return false;
            }

            // Is it a directory?
            return (info.st_mode & S_IFDIR) != 0;  
        }

        void validateInputPath() {
            string path;
            string extension;
            bool isValidFile = false;
            vector<string> allowedExtensions = {"jpg", "jpeg", "png"};
            showLog(1);
            
            while (!isValidFile) {
                cout << "[?] Enter image input path, supported formats: jpg, jpeg, png" << endl;
                cout << "[-] Example: D:\\something_in.jpg" << endl;
                cout << endl << ">>  ";
                
                getline(cin, path);
                
                // Empty input
                if (path.empty()) {
                    showLog(1);
                    cout << "[!] Error: Empty input, please enter a path." << endl << endl;
                    continue;
                }
                
                // Valid path
                if (!validatePath(path)) {
                    showLog(1);
                    cout << "[!] Error: Invalid path format or directory does not exist." << endl << endl;
                    continue;
                }
                
                // Valid extension
                extension = getExtension(path);
                if (extension.empty()) {
                    showLog(1);
                    cout << "[!] Error: No file extension found, please include the file extension." << endl << endl;
                    continue;
                }

                if (find(allowedExtensions.begin(), allowedExtensions.end(), extension) == allowedExtensions.end()) {
                    showLog(1);
                    cout << "[!] Error: Invalid file format, only jpg, jpeg, and png formats are supported." << endl << endl;
                    continue;
                }
                
                // File exists
                ifstream file(path);
                if (!file) {
                    showLog(1);
                    cout << "[!] Error: File not found, please enter a valid path." << endl << endl;
                    continue;
                }
                
                // Load image
                if (!Image::loadImage(path, extension)) {
                    showLog(1);
                    cout << "[!] Error: Image cannot be loaded, please check the file." << endl << endl;
                    continue;
                }
        
                isValidFile = true;
            }
            
            this -> inputPath = path;
            this -> inputExtension = extension;
        }

        void validateMode() {
            string input;
            int modeValue;
            bool isValid = false;
            showLog(2);
            
            while (!isValid) {
                cout << "[?] Enter mode:" << endl;
                cout << "[-] 1 ~ Variance" << endl;
                cout << "[-] 2 ~ Mean Absolute Deviation (MAD)" << endl;
                cout << "[-] 3 ~ Max Pixel Difference (MPD)" << endl;
                cout << "[-] 4 ~ Entropy" << endl;
                cout << "[-] 5 ~ Structural Similarity Index (SSIM)" << endl;
                cout << endl << ">>  ";
                
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(2);
                    cout << "[!] Error: Empty input, please enter a mode." << endl << endl;
                    continue;
                }

                // Non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789- ") != string::npos) {
                    showLog(2);
                    cout << "[!] Error: Non-numeric character found, please enter numbers only." << endl << endl;
                    continue;
                }

                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(2);
                    cout << "[!] Error: Multiple values entered, please enter a single number." << endl << endl;
                    continue;
                }
                
                // Valid range
                modeValue = stoi(input);
                if (modeValue < 1 || modeValue > 5) {
                    showLog(2);
                    cout << "[!] Error: Invalid mode, please enter a number between 1 and 5." << endl << endl;
                    continue;
                }
                
                isValid = true;
            }
            
            this -> mode = modeValue;

            if (mode == 1) errorMethod = "Variance";
            else if (mode == 2) errorMethod = "Mean Absolute Deviation (MAD)";
            else if (mode == 3) errorMethod = "Max Pixel Difference (MPD)";
            else if (mode == 4) errorMethod = "Entropy";
            else if (mode == 5) errorMethod = "Structural Similarity Index (SSIM)";
        }

        void validateThreshold() {
            string input;
            double thresholdValue;
            bool isValid = false;
            double upperThreshold = 0;
            double lowerThreshold = 0;
            showLog(3);
            
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
                cout << "[?] Enter threshold (" << lowerThreshold << "-" << upperThreshold << ")" << endl;
                cout << "[-] Example: " << (lowerThreshold + upperThreshold) / 2 << endl;
                cout << endl << ">>  ";
                
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(3);
                    cout << "[!] Error: Empty input, please enter a threshold value." << endl << endl;
                    continue;
                }
                
                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(3);
                    cout << "[!] Error: Multiple values entered, please enter a single number." << endl << endl;
                    continue;
                }
                
                // Non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789-.") != string::npos) {
                    showLog(3);
                    cout << "[!] Error: Non-numeric character found, please enter numbers only." << endl << endl;
                    continue;
                }
                
                // Valid range
                thresholdValue = stod(input);
                if (thresholdValue < lowerThreshold || thresholdValue > upperThreshold) {
                    showLog(3);
                    cout << "[!] Error: Invalid threshold, please enter a number between " << lowerThreshold << " and " << upperThreshold << " (Inclusive)." << endl << endl;
                    continue;
                }
            
                isValid = true;
            }

            this -> threshold = thresholdValue;
        }

        void validateMinBlock() {
            
            string input;
            int blockSize;
            bool isValid = false;
            showLog(4);
            
            // Calculate the smallest dimension of the image
            int minDimension = min(imgWidth, imgHeight);
            
            while (!isValid) {
                cout << "[!] Enter minimum block size, must not exceed the smallest image dimension (" << minDimension << ")" << endl;
                cout << "[-] Example: 8" << endl;
                cout << endl << ">>  ";
                
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(4);
                    cout << "[!] Error: Empty input, please enter a block size." << endl << endl;
                    continue;
                }
                
                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(4);
                    cout << "[!] Error: Multiple values entered, please enter a single number." << endl << endl;
                    continue;
                }
                
                // Non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789-") != string::npos) {
                    showLog(4);
                    cout << "[!] Error: Non-numeric character found, please enter numbers only." << endl << endl;
                    continue;
                }
            
                // Valid range
                blockSize = stoi(input);
                if (blockSize <= 0) {
                    showLog(4);
                    cout << "[!] Error: Invalid block size, please enter a positive number." << endl << endl;
                    continue;
                }
                
                if (blockSize > minDimension) {
                    showLog(4);
                    cout << "[!] Error: Block size cannot be greater than the smallest image dimension (" << minDimension << ")." << endl << endl;
                    continue;
                }
                
                isValid = true;
            }
            
            this -> minBlock = blockSize;
        }
        
        void validateTargetPercentage() {
            string input;
            double percentValue;
            bool isValid = false;
            showLog(5);
            
            while (!isValid) {
                cout << "[?] Enter target percentage as a decimal (0.0-1.0 where 1.0 = 100%)" << endl;
                cout << "[-] Example: 0.85 for 85%" << endl;
                cout << endl << ">>  ";
                
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(5);
                    cout << "[!] Error: Empty input, please enter a value." << endl;
                    continue;
                }
                
                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(5);
                    cout << "[!] Error: Multiple values entered, please enter a single number." << endl << endl;
                    continue;
                }
                
                // Non-numeric values (allow minus sign and decimal point)
                if (input.find_first_not_of("0123456789.-") != string::npos) {
                    showLog(5);
                    cout << "[!] Error: Non-numeric character found, please enter numbers and decimal point only." << endl << endl;
                    continue;
                }
                
                // Multiple decimal points
                if (count(input.begin(), input.end(), '.') > 1) {
                    showLog(5);
                    cout << "[!] Error: Multiple decimal points found, please enter a valid decimal number." << endl << endl;
                    continue;
                }
                
                // Regex pattern for a valid decimal number (including negative numbers)
                if (!regex_match(input, regex("^-?[0-9]+(\\.[0-9]+)?$"))) {
                    showLog(5);
                    cout << "[!] Error: Invalid format, please enter a valid decimal number." << endl << endl;
                    continue;
                }
                
                // Valid range
                percentValue = stod(input);
                if (percentValue < 0.0 || percentValue > 1.0) {
                    showLog(5);
                    cout << "[!] Error: Invalid percentage, please enter a number between 0.0 and 1.0." << endl << endl;
                    continue;
                }
                
                isValid = true;
            }
            
            this -> targetPercentage = percentValue;
        }

        void validateOutputPath() {
            string path;
            string extension;
            bool isValidPath = false;
            showLog(6);
            
            while (!isValidPath) {
                cout << "[?] Enter image output path, output extension must be the same as input." << endl;
                cout << "[-] Example: D:\\something_out." << inputExtension << endl;
                cout << endl << ">>  ";
                
                getline(cin, path);
                
                // Empty input
                if (path.empty()) {
                    showLog(6);
                    cout << "[!] Error: Empty input, please enter a path." << endl << endl;
                    continue;
                }
                
                // Valid path
                if (!validatePath(path)) {
                    showLog(6);
                    cout << "[!] Error: Invalid path format or directory does not exist." << endl << endl;
                    continue;
                }
                
                // Valid extension
                extension = getExtension(path);
                if (extension.empty()) {
                    showLog(6);
                    cout << "[!] Error: No file extension found, please include the file extension." << endl << endl;
                    continue;
                }

                if (extension != inputExtension) {
                    showLog(6);
                    cout << "[!] Error: Invalid file format, only jpg, jpeg, and png formats are supported." << endl << endl;
                    continue;
                }
                
                // Case 1: Output file same as input file
                if (path == inputPath) {
                    char confirm;
                    bool validResponse = false;
                    
                    while (!validResponse) {
                        cout << endl;
                        cout << "[!] Warning: Output file is identical to input file. This will overwrite your original image." << endl;
                        cout << "[?] Do you want to continue? (Y/N): ";
                        cout << endl << ">>  ";
                        
                        string response;
                        getline(cin, response);
                        
                        if (response.empty()) {
                            showLog(6);
                            cout << "[!] Error: Empty input. Please enter Y or N." << endl << endl;
                            continue;
                        }
                        
                        if (response.length() != 1) {
                            showLog(6);
                            cout << "[!] Error: Invalid input. Please enter only Y or N." << endl << endl;
                            continue;
                        }
                        
                        confirm = response[0];
                        if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                            validResponse = true;
                        } 
                        else {
                            showLog(6);
                            cout << "[!] Error: Invalid input. Please enter only Y or N." << endl << endl;
                        }
                    }
                    
                    if (tolower(confirm) != 'y') {
                        showLog(6);
                        continue;
                    }
                }

                // Case 2: Different file already exists at the output path
                else {
                    ifstream file(path);
                    if (file) {
                        char confirm;
                        bool validResponse = false;
                        
                        while (!validResponse) {
                            cout << endl;
                            cout << "[!] Warning: " << getInputExtension() << " file already exists at this location. This will overwrite the existing file." << endl;
                            cout << "[?] Do you want to continue? (Y/N): ";
                            cout << endl << ">>  ";
                            
                            string response;
                            getline(cin, response);
                            
                            if (response.empty()) {
                                showLog(6);
                                cout << "[!] Error: Empty input. Please enter Y or N." << endl << endl;
                                continue;
                            }
                            
                            if (response.length() != 1) {
                                showLog(6);
                                cout << "[!] Error: Invalid input. Please enter only Y or N." << endl << endl;
                                continue;
                            }
                            
                            confirm = response[0];
                            if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                                validResponse = true;
                            } 
                            else {
                                showLog(6);
                                cout << "[!] Error: Invalid input. Please enter only Y or N." << endl << endl;
                            }
                        }
                        
                        if (tolower(confirm) != 'y') {
                            showLog(6);
                            continue;
                        }
                    }
                }
                
                isValidPath = true;
            }
            
            this -> outputPath = path;
        }

        void validateGifPath() {
            string path;
            string extension;
            bool isValidPath = false;
            showLog(7);
            
            while (!isValidPath) {
                cout << "[?] Enter gif output path" << endl;
                cout << "[-] Example: D:\\something_out.gif" << endl;
                cout << endl << ">>  ";
                
                getline(cin, path);
                
                // Empty input
                if (path.empty()) {
                    showLog(7);
                    cout << "[!] Error: Empty input, please enter a path." << endl << endl;
                    continue;
                }
                
                // Valid path
                if (!validatePath(path)) {
                    showLog(7);
                    cout << "[!] Error: Invalid path format or directory does not exist." << endl << endl;
                    continue;
                }
                
                // Valid extension
                extension = getExtension(path);
                if (extension.empty()) {
                    showLog(7);
                    cout << "[!] Error: No file extension found, please include the file extension." << endl << endl;
                    continue;
                }

                if (extension != "gif") {
                    showLog(7);
                    cout << "[!] Error: Invalid file format, only jpg, jpeg, and png formats are supported." << endl << endl;
                    continue;
                }
                
                // Gif file already exists
                ifstream file(path);
                if (file) {
                    char confirm;
                    bool validResponse = false;
                    
                    while (!validResponse) {
                        cout << endl;
                        cout << "Warning: GIF file already exists. This will overwrite the existing file." << endl;
                        cout << "[?] Do you want to continue? (Y/N): ";
                        cout << endl << ">>  ";
                        
                        string response;
                        getline(cin, response);
                        
                        if (response.empty()) {
                            showLog(7);
                            cout << "[!] Error: Empty input. Please enter Y or N." << endl << endl;
                            continue;
                        }
                        
                        if (response.length() != 1) {
                            showLog(7);
                            cout << "[!] Error: Invalid input. Please enter only Y or N." << endl << endl;
                            continue;
                        }
                        
                        confirm = response[0];
                        if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                            validResponse = true;
                        } 
                        else {
                            showLog(7);
                            cout << "[!] Error: Invalid input. Please enter only Y or N." << endl << endl;
                        }
                    }
                    
                    if (tolower(confirm) != 'y') {
                        showLog(7);
                        continue;
                    }
                    
                    cout << "[!] Existing GIF file will be overwritten." << endl;
                }
                
                isValidPath = true;
            }
            
            this -> gifPath = path;
        }

        void showLog(int step)
        {
            cout << CLEAR_SCREEN;
            cout << "Quadpressor ~ made with love by FaRzi" << endl;
            cout << endl << "~ Tasks ~" << endl;

            if (step == 1) cout << "[1/7] Getting input path..." << endl;

            else if (step == 2) 
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Getting Error Method..." << endl;
            }

            else if (step == 3)
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Error Method Measurement: " << errorMethod << endl;
                cout << "[3/7] Getting threshold..." << endl;
            }

            else if (step == 4)
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Error Method Measurement: " << errorMethod << endl;
                cout << "[3/7] Threshold: " << threshold << endl;
                cout << "[4/7] Getting minimum block size..." << endl;
            }
            else if (step == 5)
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Error Method Measurement: " << errorMethod << endl;
                cout << "[3/7] Threshold: " << threshold << endl;
                cout << "[4/7] Minimum Block Size: " << minBlock << endl;
                cout << "[5/7] Getting target percentage..." << endl;
            }
            else if (step == 6)
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Error Method Measurement: " << errorMethod << endl;
                cout << "[3/7] Threshold: " << threshold << endl;
                cout << "[4/7] Minimum Block Size: " << minBlock << endl;
                cout << "[5/7] Target Percentage: " << targetPercentage * 100.0f << " %";

                if (targetPercentage == 0) cout << " (disabled)" << endl;
                else cout << endl;

                cout << "[6/7] Getting output path..."<<endl; 
            }
            else if (step == 7)
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Error Method Measurement: " << errorMethod << endl;
                cout << "[3/7] Threshold: " << threshold << endl;
                cout << "[4/7] Minimum Block Size: " << minBlock << endl;
                cout << "[5/7] Target Percentage: " << targetPercentage * 100.0f << " %";

                if (targetPercentage == 0) cout << " (disabled)" << endl;
                else cout << endl;

                cout << "[6/7] Output Path: " << outputPath << endl;
                cout << "[7/7] Getting gif path..." << endl;
            }

            else if (step == 8)
            {
                cout << "[1/7] Image: " << getFilename(inputPath) << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB. Path: " << inputPath << endl;
                cout << "[2/7] Error Method Measurement: " << errorMethod << endl;
                cout << "[3/7] Threshold: " << threshold << endl;
                cout << "[4/7] Minimum Block Size: " << minBlock << endl;
                cout << "[5/7] Target Percentage: " << targetPercentage * 100.0f << " %";

                if (targetPercentage == 0) cout << " (disabled)" << endl;
                else cout << endl;

                cout << "[6/7] Output Path: " << outputPath << endl;
                cout << "[7/7] GIF Path: " << gifPath << endl;
            }

            cout << endl;
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
            showLog(8);      
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