#ifndef INPUT_HPP
#define INPUT_HPP

#include <iostream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include "QuadTree.hpp"

// Global variables for image data
extern unsigned char* currImgData;
extern unsigned char* initImgData;
extern unsigned char* tempImgData;
extern int imgWidth, imgHeight, imgChannels;

using namespace std;

class Input {
    private:

        int mode, minBlock;
        double threshold, upperThreshold, lowerThreshold, targetPercentage;
        string inputPath, inputPathDisplay;
        string outputPath, outputPathDisplay;
        string gifPath, gifPathDisplay;
        string inputExtension, errorMethod;

        // Helper function to extract filename with extension from a path
        string getFilename(const string& path) {
            size_t lastSlash = path.find_last_of("/\\");
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
        
        // Method to convert WSL paths to Windows paths
        string convertPath(const string& originalPath) {
            // Check if it's a WSL path
            if (originalPath.length() >= 6 && originalPath.substr(0, 5) == "/mnt/" && 
                isalpha(originalPath[5]) && (originalPath.length() == 6 || originalPath[6] == '/')) {
                
                // Get drive letter
                char driveLetter = originalPath[5];
                
                // Create Windows-style path
                string windowsPath = string(1, toupper(driveLetter)) + ":";
                
                // Add the rest of the path, replacing forward slashes with backslashes
                if (originalPath.length() > 6) {
                    string restOfPath = originalPath.substr(6);
                    
                    // Replace all forward slashes with backslashes
                    for (char& c : restOfPath) {
                        if (c == '/') {
                            c = '\\';
                        }
                    }
                    
                    windowsPath += restOfPath;
                } else {
                    // Just the drive root
                    windowsPath += "\\";
                }
                
                return windowsPath;
            }
            
            // Not a WSL path, return unchanged
            return originalPath;
        }

        // Check if the path is a fully qualified path (has drive letter and directory structure)
        string validatePath(const string& path) {
            string directory;
            
            // Check if it's a WSL-mounted Windows drive path
            if (path.length() >= 6 && path.substr(0, 5) == "/mnt/" && isalpha(path[5]) && 
                (path.length() == 6 || path[6] == '/')) {
                
                // Validate basic WSL path format
                char driveLetter = toupper(path[5]);
                
                // Make sure the drive letter is valid (typically A-Z)
                if (driveLetter < 'A' || driveLetter > 'Z') {
                    return "Drive tidak valid. Gunakan drive letter yang benar (A-Z).";
                }
                
                // Convert to Windows path
                string windowsPath = convertPath(path);
                
                // Extract directory part from the Windows path
                size_t lastSlash = windowsPath.find_last_of("\\");
                
                if (lastSlash == string::npos) { 
                    return "Format path-nya salah bang.";
                }
                
                // Set directory to the Windows path's directory part
                if (lastSlash <= 2) {
                    directory = windowsPath.substr(0, 3);  // Take "D:\" or similar
                } else {
                    directory = windowsPath.substr(0, lastSlash); // Subdirectory
                }
            }
            // UNIX path validation
            else if (path.length() > 0 && path[0] == '/') {
                // Extract directory part from the path
                size_t lastSlash = path.find_last_of("/");
                
                if (lastSlash == string::npos) { 
                    return "Format path-nya salah bang.";
                }
                
                // Handle root directory case
                if (lastSlash == 0) {
                    directory = "/"; // Root directory
                } else {
                    directory = path.substr(0, lastSlash);
                }
            }
            // Windows path validation
            else if (path.length() >= 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) {
                // Extract directory part from the path
                size_t lastSlash = path.find_last_of("/\\");
                
                if (lastSlash == string::npos) { 
                    return "Format path-nya salah bang.";
                }
                
                // Handle Windows root directory case
                if (lastSlash <= 2) {
                    directory = path.substr(0, 3);  // Take "D:\" or "D:/"
                } else {
                    directory = path.substr(0, lastSlash); // This is a file in a subdirectory
                }
            }
            else {
                return "Format path-nya salah bang.";
            }
            
            // Directory exists or not? (common check for all path types)
            struct stat info;
            if (stat(directory.c_str(), &info) != 0) {
                return "Direktorinya engga ada, coba tambahin dulu deh";
            }
        
            // Is it a directory?
            if (!(info.st_mode & S_IFDIR)) {
                return "Format path-nya salah bang.";
            }
            
            return ""; // Empty string means valid path
        }

        void validateInputPath() {
            string path;
            string convertedPath;
            string extension;
            bool isValidFile = false;
            vector<string> allowedExtensions = {"jpg", "jpeg", "png"};
            showLog(1);
            
            while (!isValidFile) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;
                getline(cin, path);
                

                // Empty input
                if (path.empty()) {
                    showLog(1);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
                
                // Valid path
                string pathError = validatePath(path);
                if (!pathError.empty()) {
                    showLog(1);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: " << pathError << endl << endl;
                    continue;
                }
                
                // Valid extension
                extension = getExtension(path);
                if (extension.empty()) {
                    showLog(1);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Jangan lupa extensionnya yaa... udah gede loh" << endl << endl;
                    continue;
                }

                if (find(allowedExtensions.begin(), allowedExtensions.end(), extension) == allowedExtensions.end()) {
                    showLog(1);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Extensionnya yang dibolehin aja yaa, contohnya ";
                    cout << RESET BRIGHT_GREEN << ".jpg" << BRIGHT_WHITE << ", " << BRIGHT_GREEN << ".jpeg" << BRIGHT_WHITE << ", " << BRIGHT_GREEN << ".png" << BRIGHT_WHITE;
                    cout << RESET BRIGHT_WHITE ITALIC << "." << endl << endl;
                    continue;
                }
                
                convertedPath = convertPath(path);

                // File exists
                ifstream file(convertedPath);
                if (!file) {
                    showLog(1);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Ga ada file image-nya bang, lupa taroh ya?" << endl << endl;
                    continue;
                }
                
                // Load image
                string errorMsg = Image::loadImage(convertedPath, extension);
                if (!errorMsg.empty()) {
                    showLog(1);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: " << errorMsg << endl << endl;
                    continue;
                }
        
                isValidFile = true;
            }
            
            this -> inputPathDisplay = path;
            this -> inputPath = convertedPath; 
            this -> inputExtension = extension;
        }

        void validateMode() {
            string input;
            int modeValue;
            bool isValid = false;
            showLog(2);
            
            while (!isValid) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(2);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
        
                // Non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789- ") != string::npos) {
                    showLog(2);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angka doang bolehnya yaa, input-nya jangan aneh-aneh pls." << endl << endl;
                    continue;
                }
        
                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(2);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Bener sih angka doang, tapi gabole multiple input yaa..." << endl << endl;
                    continue;
                }
                
                // Valid range
                try {
                    long long modeValueLL = stoll(input);
                    if (modeValueLL < 1 || modeValueLL > 5) {
                        showLog(2);
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Bolehnya angka";
                        cout << RESET BRIGHT_CYAN << " 1" << BRIGHT_WHITE ITALIC << " and " << RESET BRIGHT_CYAN << "5" << BRIGHT_WHITE ITALIC << " doang, yok literasinya" << endl << endl;
                        continue;
                    }
                    
                    modeValue = static_cast<int>(modeValueLL);
                    isValid = true;
                }
                catch (const std::out_of_range& e) {
                    showLog(2);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angkanya terlalu besar, coba yang lebih kecil ya" << endl << endl;
                    continue;
                }
                catch (const std::invalid_argument& e) {
                    showLog(2);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Format angkanya salah, coba lagi yaa" << endl << endl;
                    continue;
                }
            }
            
            this->mode = modeValue;

            if (mode == 1) errorMethod = "Variance";
            else if (mode == 2) errorMethod = "Mean Absolute Deviation (MAD)";
            else if (mode == 3) errorMethod = "Max Pixel Difference (MPD)";
            else if (mode == 4) errorMethod = "Entropy";
            else if (mode == 5) errorMethod = "Structural Similarity Index (SSIM)";

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
        }

        void validateThreshold() {
            string input;
            double thresholdValue;
            bool isValid = false;
            showLog(3);
        
            while (!isValid) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(3);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
                
                // Non-numeric characters (allow minus sign and decimal point)
                if (input.find_first_not_of("0123456789-. ") != string::npos) {
                    showLog(3);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angka doang bolehnya yaa, input-nya jangan aneh-aneh pls." << endl << endl;
                    continue;
                }
        
                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(3);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Bener sih angka doang, tapi gabole multiple input yaa..." << endl << endl;
                    continue;
                }
                
                // Multiple decimal points
                if (count(input.begin(), input.end(), '.') > 1) {
                    showLog(3);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Desimal titiknya cuma satu yaa..." << endl << endl;
                    continue;
                }
                
                // Valid range
                try {
                    thresholdValue = stod(input);
                    if (thresholdValue < lowerThreshold || thresholdValue > upperThreshold) {
                        showLog(3);
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid threshold, coba dibaca lagi yaa batasnya dari ";
                        cout << RESET BRIGHT_CYAN << lowerThreshold << BRIGHT_WHITE ITALIC << " sampai " << RESET BRIGHT_CYAN << upperThreshold << BRIGHT_YELLOW ITALIC << " (Inklusif)." << endl << endl;
                        continue;
                    }
                    
                    isValid = true;
                } 
                catch (const std::out_of_range& e) {
                    showLog(3);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angkanya terlalu besar atau terlalu kecil." << endl << endl;
                    continue;
                } 
                catch (const std::invalid_argument& e) {
                    showLog(3);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Format angkanya salah, coba lagi yaa." << endl << endl;
                    continue;
                }
            }

            this->threshold = thresholdValue;
        }

        void validateMinBlock() {
            string input;
            int blockSize;
            bool isValid = false;
            showLog(4);
            
            int maxBlockSize = imgWidth * imgHeight;
            
            while (!isValid) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(4);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
                
                // Non-numeric characters (allow minus sign)
                if (input.find_first_not_of("0123456789- ") != string::npos) {
                    showLog(4);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angka doang bolehnya yaa, input-nya jangan aneh-aneh pls." << endl << endl;
                    continue;
                }
        
                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(4);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Bener sih angka doang, tapi gabole multiple input yaa..." << endl << endl;
                    continue;
                }
                
                // Valid range
                try {
                    long long blockSizeLL = stoll(input);
                    if (blockSizeLL <= 0) {
                        showLog(4);
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Sejak kapan area ada yang negatif :) coba lagi ya" << endl << endl;
                        continue;
                    }
                    
                    if (blockSizeLL > maxBlockSize) {
                        showLog(4);
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Block size terlalu besar, maksimal ";
                        cout << RESET BRIGHT_CYAN << maxBlockSize << " px" << BRIGHT_WHITE ITALIC << " yaa." << endl << endl;
                        continue;
                    }
                    
                    blockSize = static_cast<int>(blockSizeLL);
                    isValid = true;
                }
                catch (const std::out_of_range& e) {
                    showLog(4);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angkanya terlalu besar, coba yang lebih kecil ya" << endl << endl;
                    continue;
                }
                catch (const std::invalid_argument& e) {
                    showLog(4);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Format angkanya salah, coba lagi yaa" << endl << endl;
                    continue;
                }
            }
            
            this->minBlock = blockSize;
        }
        
        void validateTargetPercentage() {
            string input;
            double percentValue;
            bool isValid = false;
            showLog(5);
            
            while (!isValid) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;
                getline(cin, input);
                
                // Empty input
                if (input.empty()) {
                    showLog(5);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
                
                // Non-numeric characters (allow minus sign and decimal point)
                if (input.find_first_not_of("0123456789-. ") != string::npos) {
                    showLog(5);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angka doang bolehnya yaa, input-nya jangan aneh-aneh pls." << endl << endl;
                    continue;
                }

                // Multiple inputs (whitespace)
                if (input.find(' ') != string::npos) {
                    showLog(5);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Bener sih angka doang, tapi gabole multiple input yaa..." << endl << endl;
                    continue;
                }
                
                // Multiple decimal points
                if (count(input.begin(), input.end(), '.') > 1) {
                    showLog(5);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Desimal titiknya cuma satu yaa..." << endl << endl;
                    continue;
                }

                // Valid range
                try {
                    percentValue = stod(input);
                    
                    
                    if (percentValue < 0.0 || percentValue > 1.0) {
                        showLog(5);
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Bolehnya di rentang ";
                        cout << RESET BRIGHT_CYAN << "0.0" << BRIGHT_WHITE ITALIC << " sampai " << RESET BRIGHT_CYAN << "1.0" << BRIGHT_WHITE ITALIC << " aja yaa." << endl << endl;
                        continue;
                    }
                    
                    isValid = true;
                } 
                catch (const std::out_of_range& e) {
                    showLog(5);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Angkanya terlalu besar atau terlalu kecil." << endl << endl;
                    continue;
                } 
                catch (const std::invalid_argument& e) {
                    showLog(5);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Format angkanya salah, coba lagi yaa." << endl << endl;
                    continue;
                }
            }
            
            this->targetPercentage = percentValue;
}

        void validateOutputPath() {
            string path;
            string extension;
            bool isValidPath = false;
            showLog(6);
            
            while (!isValidPath) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;               
                getline(cin, path);
                
                // Empty input
                if (path.empty()) {
                    showLog(6);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
                
                // Valid path
                string pathError = validatePath(path);
                if (!pathError.empty()) {
                    showLog(6);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: " << pathError << endl << endl;
                    continue;
                }
                
                // Valid extension
                extension = getExtension(path);
                if (extension.empty()) {
                    showLog(6);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Jangan lupa extensionnya yaa... udah gede loh" << endl << endl;
                    continue;
                }

                if (extension != inputExtension) {
                    showLog(6);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Extensionnya ga sesuai sama input-nya harusnya ";
                    cout << RESET BRIGHT_GREEN << inputExtension << BRIGHT_WHITE ITALIC << ", coba lagi yaa..." << endl << endl;
                    continue;
                }
                
                // Case 1: Output file same as input file
                if (path == inputPath) {
                    char confirm;
                    bool validResponse = false;
                    
                    while (!validResponse) {
                        cout << endl;
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Warning: Output file bakal overwrite input file." << endl;
                        cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Yakin ga nih? " << RESET BRIGHT_WHITE << "[";
                        cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE << "/" << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "]" << endl;
                        cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;            
                        
                        string response;
                        getline(cin, response);
                        
                        if (response.empty()) {
                            showLog(6);
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, pls enter ";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                            continue;
                        }
                        
                        if (response.length() != 1) {
                            showLog(6);
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid input, pls enter hanya ";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                            continue;
                        }
                        
                        confirm = response[0];
                        if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                            validResponse = true;
                        } 
                        else {
                            showLog(6);
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid input, pls enter hanya ";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                            continue;
                        }
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
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Warning: ";
                            cout << RESET BRIGHT_GREEN << "." << getInputExtension() << RESET BRIGHT_WHITE ITALIC << " file udah ada, jadinya bakal di-overwrite." << endl;
                            cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Yakin ga nih? " << RESET BRIGHT_WHITE << "[";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE << "/" << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "]" << endl;
                            cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;            
                            
                            string response;
                            getline(cin, response);
                            
                            if (response.empty()) {
                                showLog(6);
                                cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, pls enter ";
                                cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                                continue;
                            }
                            
                            if (response.length() != 1) {
                                showLog(6);
                                cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid input, pls enter hanya ";
                                cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                                continue;
                            }
                            
                            confirm = response[0];
                            if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                                validResponse = true;
                            } 
                            else {
                                showLog(6);
                                cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid input, pls enter hanya ";
                                cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                                continue;
                            }
                        }
                    }
                }
                
                isValidPath = true;
            }
            
            this -> outputPathDisplay = path;
            this -> outputPath = convertPath(path);
        }

        void validateGifPath() {
            string path;
            string extension;
            bool isValidPath = false;
            showLog(7);
            
            while (!isValidPath) {
                cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;               
                getline(cin, path);

                // Empty input
                if (path.empty()) {
                    showLog(7);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, coba lagi ya bang :D" << endl << endl;
                    continue;
                }
                
                // Valid path
                string pathError = validatePath(path);
                if (!pathError.empty()) {
                    showLog(7);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: " << pathError << endl << endl;
                    continue;
                }
                
                // Valid extension
                extension = getExtension(path);
                if (extension.empty()) {
                    showLog(7);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Jangan lupa extensionnya yaa... udah gede loh" << endl << endl;
                    continue;
                }

                if (extension != "gif") {
                    showLog(7);
                    cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Extensionnya harusnya ";
                    cout << RESET BRIGHT_GREEN << ".gif" << RESET BRIGHT_WHITE ITALIC << " ya bang..." << endl << endl;
                    continue;
                }
                
                // Gif file already exists
                ifstream file(path);
                if (file) {
                    char confirm;
                    bool validResponse = false;
                    
                    while (!validResponse) {
                        cout << endl;
                        cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Warning: ";
                        cout << RESET BRIGHT_GREEN << ".gif" << RESET BRIGHT_WHITE ITALIC << " file udah ada, jadinya bakal di-overwrite" << endl;
                        cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Yakin ga nih? " << RESET BRIGHT_WHITE << "[";
                        cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE << "/" << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "]" << endl;
                        cout << RESET BRIGHT_YELLOW << ">>  " << BRIGHT_WHITE BAR_CURSOR;
                        
                        string response;
                        getline(cin, response);
                        
                        if (response.empty()) {
                            showLog(7);
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Classic empty input, pls enter ";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                            continue;
                        }
                        
                        if (response.length() != 1) {
                            showLog(7);
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid input, pls enter hanya ";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                            continue;
                        }
                        
                        confirm = response[0];
                        if (tolower(confirm) == 'y' || tolower(confirm) == 'n') {
                            validResponse = true;
                        } 
                        else {
                            showLog(7);
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Error: Invalid input, pls enter hanya ";
                            cout << RESET GREEN BOLD << "Y" << RESET BRIGHT_WHITE ITALIC << " atau " << RESET RED BOLD << "N" << RESET BRIGHT_WHITE << "." << endl << endl;
                            continue;
                        }
                    }
                }
                
                isValidPath = true;
            }
            
            this -> gifPathDisplay = path;
            this -> gifPath = convertPath(path);
        }

        void showLog(int step)
        {
            cout << CLEAR_SCREEN;
            cout << ITALIC MAGENTA << "Quadpressor" << BRIGHT_WHITE << " ~ made with " << BRIGHT_RED << "love " << BRIGHT_WHITE << "by " << BRIGHT_CYAN << "FaRzi" << BRIGHT_WHITE << " :D" << endl;
            cout << endl << "~ " << BRIGHT_YELLOW << "Tasks" << BRIGHT_WHITE << " ~" << endl;
        
            if (step == 1) 
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Getting input path..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter image input path, supported formats: ";
                cout << RESET BRIGHT_GREEN << "jpg" << BRIGHT_WHITE << ", " << BRIGHT_GREEN << "jpeg" << BRIGHT_WHITE << ", " << BRIGHT_GREEN << "png" << BRIGHT_WHITE << "." << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example:" << RESET MAGENTA << " D:\\something_in.jpg" << endl;
            }
        
            else if (step == 2) 
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Getting Error Method..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter mode:" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 1 " << BRIGHT_RED << "~" << RESET ITALIC << " Variance" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 2 " << BRIGHT_RED << "~" << RESET ITALIC << " Mean Absolute Deviation (MAD)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 3 " << BRIGHT_RED << "~" << RESET ITALIC << " Max Pixel Difference (MPD)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 4 " << BRIGHT_RED << "~" << RESET ITALIC << " Entropy" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 5 " << BRIGHT_RED << "~" << RESET ITALIC << " Structural Similarity Index (SSIM)" << endl;
            }
        
            else if (step == 3)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Method Measurement: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Getting threshold..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter threshold " << RESET BRIGHT_CYAN << "(" << lowerThreshold << " - " << upperThreshold << ")" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example: " << RESET MAGENTA << (lowerThreshold + upperThreshold) / 2 << endl;
            }
        
            else if (step == 4)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Method Measurement: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Getting minimum block size..." << endl;
                cout << endl;
                
                int maxBlockSize = imgWidth * imgHeight;
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter minimum block size, must not exceed the image area ";
                cout << RESET BRIGHT_CYAN << "(" << maxBlockSize << " px)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example: " << RESET MAGENTA << "8" << endl;
            }
            else if (step == 5)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Method Measurement: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Getting target percentage..." << endl;
                cout << endl;
                
                
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter target percentage as a decimal " << RESET BRIGHT_CYAN << "(0.0 - 1.0 where 1.0 = 100%)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example: ";
                cout << RESET MAGENTA << 0.85 << BRIGHT_WHITE ITALIC << " for " << RESET MAGENTA << "85%" << endl;
            }
            else if (step == 6)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Method Measurement: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Target Percentage: " << RESET MAGENTA << targetPercentage;
        
                if (targetPercentage == 0) cout << RESET BRIGHT_RED << " (disabled)" << endl;
                else cout << " (" << targetPercentage * 100.0f << "%)" << endl;
        
                cout << RESET GREEN BOLD << "[6/7]" << RESET BRIGHT_WHITE ITALIC << " Getting output path..."<<endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter image output path, output extension must be the same as input." << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example:" << RESET MAGENTA << " D:\\something_out.jpg" << endl;
            }
        
            else if (step == 7)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Method Measurement: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Target Percentage: " << RESET MAGENTA << targetPercentage;
        
                if (targetPercentage == 0) cout << RESET BRIGHT_RED << " (disabled)" << endl;
                else cout << " (" << targetPercentage * 100.0f << "%)" << endl;
        
                cout << RESET GREEN BOLD << "[6/7]" << RESET BRIGHT_WHITE ITALIC << " Output Path: " << RESET MAGENTA << outputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[7/7]" << RESET BRIGHT_WHITE ITALIC << " Getting gif path..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Enter gif output path" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example:" << RESET MAGENTA << " D:\\something_out.gif" << endl;
            }
        
            else if (step == 8)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Method Measurement: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Target Percentage: " << RESET MAGENTA << targetPercentage;
        
                if (targetPercentage == 0) cout << RESET BRIGHT_RED << " (disabled)" << endl;
                else cout << " (" << targetPercentage * 100.0f << "%)" << endl;
        
                cout << RESET GREEN BOLD << "[6/7]" << RESET BRIGHT_WHITE ITALIC << " Output Path: " << RESET MAGENTA << outputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[7/7]" << RESET BRIGHT_WHITE ITALIC << " GIF Path: " << RESET MAGENTA << gifPathDisplay << endl;
            }
        
            cout << RESET << endl;
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