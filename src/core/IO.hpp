#ifndef INPUT_HPP
#define INPUT_HPP

// Libraries
#include <iostream>
#include <fstream>
#include <regex>
#include <filesystem>
#include <atomic>
#include <thread>
#include "QuadTree.hpp"

extern atomic<bool> done;

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

using namespace std;

/**
 * @brief Handles user input validation and processing
 * @param mode Selected error calculation mode (1-5)
 * @param minBlock Minimum block size in pixels
 * @param threshold Error threshold value
 * @param upperThreshold Upper threshold for error calculations
 * @param lowerThreshold Lower threshold for error calculations
 * @param targetPercentage Target compression percentage (0-1)
 * @param inputPath Input image path
 * @param inputPathDisplay Display version of input path
 * @param outputPath Output image path
 * @param outputPathDisplay Display version of output path
 * @param gifPath Output GIF path for visualization
 * @param gifPathDisplay Display version of GIF path
 * @param inputExtension Input image file extension
 * @param errorMethod Name of selected error method
 */
class IOHandler {

    private:
        int mode, minBlock;
        double threshold, upperThreshold, lowerThreshold, targetPercentage;
        string inputPath, inputPathDisplay;
        string outputPath, outputPathDisplay;
        string gifPath, gifPathDisplay;
        string inputExtension, errorMethod;

        /**
         * @brief Extract filename with extension from a path
         * @param path Full file path
         * @return Filename with extension
         */
        string getFilename(const string& path) {
            size_t lastSlash = path.find_last_of("/\\");
            string filename = path.substr(lastSlash + 1);
            return filename;
        }

        /**
         * @brief Extract extension from a path
         * @param path Full file path
         * @return File extension without dot
         */
        string getExtension(const string& path) {
            string extension = "";
            size_t dotPos = path.find_last_of('.');

            if (dotPos != string::npos) {
                extension = path.substr(dotPos + 1);
                return extension;
            }

            return "";
        }
        
        /**
         * @brief Detect the current system (WSL, UNIX, Windows)
         * @return String indicating system type
         */
        string getPlatformType() {
            FILE* f = fopen("/proc/version", "r");
            if (f) {
                char buffer[256];
                fread(buffer, sizeof(char), sizeof(buffer) - 1, f);
                fclose(f);
                buffer[255] = '\0';
        
                if (strstr(buffer, "Microsoft") != nullptr || strstr(buffer, "WSL") != nullptr) {
                    return "WSL";
                }
                return "UNIX";
            }
        
            // If /proc/version doesn't exist, assume native Windows
            return "Windows";
        }

        /**
         * @brief Convert WSL paths to Windows paths
         * @param originalPath Original path
         * @return Converted path
         */
        string convertPath(const string& originalPath) {
            
            if (originalPath.length() >= 6 && originalPath.substr(0, 5) == "/mnt/" && 
                isalpha(originalPath[5]) && (originalPath.length() == 6 || originalPath[6] == '/')) {
                
                char driveLetter = originalPath[5];
                
                // Create Windows-style path
                string windowsPath = string(1, toupper(driveLetter)) + ":";
                if (originalPath.length() > 6) {
                    string restOfPath = originalPath.substr(6);

                    for (char& c : restOfPath) {
                        if (c == '/') {
                            c = '\\';
                        }
                    }
                    
                    windowsPath += restOfPath;
                } 
                else {
                    windowsPath += "\\";
                }
                
                return windowsPath;
            }
            
            // Not a WSL path, return unchanged
            return originalPath;
        }

        /**
         * @brief Validate if a path is valid for the current platform
         * @param path Path to validate
         * @return Empty string if valid, error message if invalid
         */
        string validatePath(const string& path) {
            string directory;
            string platform = getPlatformType();

            // Restrict WSL/UNIX access to $HOME or /mnt/<drive>/...
            if (platform == "WSL" || platform == "UNIX") {
            
                bool startsWithHome = path.rfind("/home/", 0) == 0;
                bool startsWithMnt = path.rfind("/mnt/", 0) == 0 &&
                                     path.length() >= 7 &&
                                     isalpha(path[5]) &&
                                     path[6] == '/';
            
                if (!startsWithHome && !startsWithMnt) {
                    return "WSL dan UNIX hanya bisa akses path berawalan /home/<user>/ atau /mnt/<drive>/, pastiin sesuai yaa.";
                }
            }

            // Check if the path is a /mnt path
            if (path.length() >= 6 && path.substr(0, 5) == "/mnt/" && isalpha(path[5]) &&
                (path.length() == 6 || path[6] == '/')) {
                if (platform != "WSL") return "Format path (/mnt/...) hanya bisa digunakan pada WSL.";
        
                char driveLetter = toupper(path[5]);
                if (driveLetter < 'A' || driveLetter > 'Z') return "Drive tidak valid. Gunakan drive letter yang benar (A-Z).";
        
                string windowsPath = convertPath(path);
                size_t lastSlash = windowsPath.find_last_of("\\");
                
                if (lastSlash == string::npos) { 
                    return "Format path-nya salah bang.";
                }
                
                // Set directory to the Windows path's directory part
                if (lastSlash <= 2) directory = windowsPath.substr(0, 3);   // Take "D:\" or similar 
                else directory = windowsPath.substr(0, lastSlash);          // Subdirectory
            }

            // Check if the path is a UNIX / WSL path
            else if (path.length() > 0 && path[0] == '/') {
                if (platform == "Windows") return "Format path UNIX (dimulai dengan /) tidak bisa digunakan pada sistem Windows.";
                size_t lastSlash = path.find_last_of("/");
                if (lastSlash == string::npos) return "Format path-nya salah bang.";
                directory = (lastSlash == 0) ? "/" : path.substr(0, lastSlash);
            }

            // Check if the path is a Windows path
            else if (path.length() >= 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) {
                if (platform != "Windows") return "Format path Windows (C:\\...) tidak bisa digunakan pada sistem UNIX.";
                size_t lastSlash = path.find_last_of("/\\");
                if (lastSlash == string::npos) return "Format path-nya salah bang.";
                directory = (lastSlash <= 2) ? path.substr(0, 3) : path.substr(0, lastSlash);
            }

            // Not a recognized path format
            else {
                return "Format path-nya salah bang.";
            }
            
            // Check if the directory exists from the path
            if (!std::filesystem::is_directory(directory)) {
                return "Direktorinya engga ada, coba tambahin dulu deh atau cek typo di path-nya.";
            }
            
            // Valid path
            return "";
        }

        /**
         * @brief Process and validate input image path
         */
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

        /**
         * @brief Process and validate error calculation mode
         */
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

        /**
         * @brief Process and validate error threshold value
         */
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

                // Overflow or invalid argument
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

        /**
         * @brief Process and validate minimum block size
         */
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
        
        /**
         * @brief Process and validate target compression percentage
         */
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

                // Overflow or invalid argument
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

        /**
         * @brief Process and validate output image path
         */
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

                    if (validResponse && tolower(confirm) == 'n') 
                    {
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
                            cout << RESET RED BOLD << "[!]" << RESET BRIGHT_WHITE ITALIC << " Warning: ";
                            cout << RESET BRIGHT_GREEN << getFilename(path) << RESET BRIGHT_WHITE ITALIC << " file udah ada, jadinya bakal di-overwrite." << endl;
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

                        if (validResponse && tolower(confirm) == 'n') 
                        {
                            showLog(6);
                            continue;
                        }
                    }
                
                    isValidPath = true;
                }
            }

            this -> outputPathDisplay = path;
            this -> outputPath = convertPath(path);
        }

        /**
         * @brief Process and validate output GIF path
         */
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
                        cout << RESET BRIGHT_GREEN << getFilename(path) << RESET BRIGHT_WHITE ITALIC << " file udah ada, jadinya bakal di-overwrite" << endl;
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

                    if (validResponse && tolower(confirm) == 'n') 
                    {   
                        showLog(7);
                        continue;
                    }
                }
                
                isValidPath = true;
            }
            
            this -> gifPathDisplay = path;
            this -> gifPath = convertPath(path);
        }

        /**
         * @brief Display the log messages for each step of the process
         * @param step The current step of the process
         */
        void showLog(int step)
        {
            cout << CLEAR_SCREEN;
            cout << ITALIC MAGENTA << "Quadpressor" << BRIGHT_WHITE << " ~ made with " << BRIGHT_RED << "love " << BRIGHT_WHITE << "by " << BRIGHT_CYAN << "FaRzi" << BRIGHT_WHITE << " :D" << endl;
            cout << endl << "~ " << BRIGHT_YELLOW << "Tasks" << BRIGHT_WHITE << " ~" << endl;
        
            // Input path
            if (step == 1) 
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Enter image input path, supported formats: ";
                cout << RESET BRIGHT_GREEN << "jpg" << BRIGHT_WHITE << ", " << BRIGHT_GREEN << "jpeg" << BRIGHT_WHITE << ", " << BRIGHT_GREEN << "png" << BRIGHT_WHITE << "." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Example: " << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_YELLOW << " D:\\bla-bla-bla\\example_in.jpg" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_CYAN << "Windows" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_YELLOW << " /home/<user>/example_in.jpg" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_CYAN << "Linux" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_YELLOW << " /home/<user>/example_in.jpg" << RESET ITALIC BRIGHT_WHITE << " or";
                cout << RESET BRIGHT_YELLOW << " /mnt/<drive>/example_in.jpg" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_CYAN << "WSL" << endl;
            }
        
            // Error method (mode)
            else if (step == 2) 
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Enter error measurement method..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Available modes:" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 1 " << BRIGHT_RED << "~" << RESET ITALIC << " Variance" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 2 " << BRIGHT_RED << "~" << RESET ITALIC << " Mean Absolute Deviation (MAD)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 3 " << BRIGHT_RED << "~" << RESET ITALIC << " Max Pixel Difference (MPD)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 4 " << BRIGHT_RED << "~" << RESET ITALIC << " Entropy" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " 5 " << BRIGHT_RED << "~" << RESET ITALIC << " Structural Similarity Index (SSIM)" << endl;
            }
        
            // Threshold
            else if (step == 3)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Measurement Method: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Enter threshold..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Threshold range: " << RESET BRIGHT_CYAN << lowerThreshold << " - " << upperThreshold << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example: " << RESET MAGENTA << (lowerThreshold + upperThreshold) / 2 << endl;
            }
            
            // Minimum block size
            else if (step == 4)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Measurement Method: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Enter minimum block size..." << endl;
                cout << endl;
                
                int maxBlockSize = imgWidth * imgHeight;
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Must not exceed the image area ";
                cout << RESET BRIGHT_CYAN << "(" << maxBlockSize << " px)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example: " << RESET MAGENTA << "8" << endl;
            }

            // Target percentage
            else if (step == 5)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Measurement Method: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Enter target percentage..." << endl;
                cout << endl;
                
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Decimal " << RESET BRIGHT_CYAN << "(0.0 - 1.0 where 1.0 = 100%)" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE ITALIC << " Example: ";
                cout << RESET MAGENTA << 0.85 << BRIGHT_WHITE ITALIC << " for " << RESET MAGENTA << "85%" << endl;
            }

            // Output image path
            else if (step == 6)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Measurement Method: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Target Percentage: " << RESET MAGENTA << targetPercentage;
        
                if (targetPercentage == 0) cout << RESET BRIGHT_RED << " (disabled)" << endl;
                else cout << " (" << targetPercentage * 100.0f << "%)" << endl;
        
                cout << RESET GREEN BOLD << "[6/7]" << RESET BRIGHT_WHITE ITALIC << " Enter image output path..."<<endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Output extension must be the same as input. Example: " << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_YELLOW << " D:\\bla-bla-bla\\example_out.jpg" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_CYAN << "Windows" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_YELLOW << " /home/<user>/example_out.jpg" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_CYAN << "Linux" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_YELLOW << " /home/<user>/example_out.jpg" << RESET ITALIC BRIGHT_WHITE << " or";
                cout << RESET BRIGHT_YELLOW << " /mnt/<drive>/example_out.jpg" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_CYAN << "WSL" << endl;
            }
            
            // Output GIF path
            else if (step == 7)
            {
                cout << RESET GREEN BOLD << "[1/7]" << RESET BRIGHT_WHITE ITALIC << " Image: " << RESET MAGENTA << getFilename(inputPathDisplay) << BRIGHT_YELLOW << " (" << imgWidth << " x " << imgHeight << ") ";
                cout << BRIGHT_CYAN << fixed << setprecision(2) << Image::getSizeInKB(Image::getOriginalSize(inputPath)) << " KB" << BRIGHT_WHITE << ". Path: " << MAGENTA << inputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[2/7]" << RESET BRIGHT_WHITE ITALIC << " Error Measurement Method: " << RESET MAGENTA << errorMethod << endl;
                cout << RESET GREEN BOLD << "[3/7]" << RESET BRIGHT_WHITE ITALIC << " Threshold: " << RESET MAGENTA << threshold << endl;
                cout << RESET GREEN BOLD << "[4/7]" << RESET BRIGHT_WHITE ITALIC << " Minimum Block Size: " << RESET MAGENTA << minBlock << endl;
                cout << RESET GREEN BOLD << "[5/7]" << RESET BRIGHT_WHITE ITALIC << " Target Percentage: " << RESET MAGENTA << targetPercentage;
        
                if (targetPercentage == 0) cout << RESET BRIGHT_RED << " (disabled)" << endl;
                else cout << " (" << targetPercentage * 100.0f << "%)" << endl;
        
                cout << RESET GREEN BOLD << "[6/7]" << RESET BRIGHT_WHITE ITALIC << " Output Path: " << RESET MAGENTA << outputPathDisplay << endl;
                cout << RESET GREEN BOLD << "[7/7]" << RESET BRIGHT_WHITE ITALIC << " Enter GIF output path..." << endl;
                cout << endl;
        
                cout << RESET BRIGHT_CYAN BOLD << "[?]" << RESET BRIGHT_WHITE ITALIC << " Example: " << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_GREEN << " D:\\bla-bla-bla\\example_out.gif" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_YELLOW << "Windows" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_GREEN << " /home/<user>/example_out.gif" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_YELLOW << "Linux" << endl;
                cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_GREEN << " /home/<user>/example_out.gif" << RESET ITALIC BRIGHT_WHITE << " or";
                cout << RESET BRIGHT_GREEN << " /mnt/<drive>/example_out.gif" << RESET ITALIC BRIGHT_WHITE << " for " << RESET BRIGHT_YELLOW << "WSL" << endl;
            }
            
            // Complete
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
        /**
         * @brief Constructor that validates all required inputs
         */
        IOHandler() {
            validateInputPath();
            validateMode();
            validateThreshold();
            validateMinBlock();
            validateTargetPercentage();
            validateOutputPath();
            validateGifPath();      
            showLog(8); 
        }

        /**
         * @brief Destructor
         */
        ~IOHandler() {}

        /**
         * @brief Get the validated input image path
         * @return Input image path
         */
        string getInputPath() {return inputPath;}

        /**
         * @brief Get the input image extension
         * @return Input image extension
         */
        string getInputExtension() {return inputExtension;}

        /**
         * @brief Get the selected error calculation mode
         * @return Error mode (1-5)
         */
        int getMode() {return mode;}

        /**
         * @brief Get the selected error method
         * @return Error method name
         */
        double getThreshold() {return threshold;}

        /**
         * @brief Get the minimum block size
         * @return Minimum block size
         */
        int getMinBlock() {return minBlock;}

        /**
         * @brief Get the target compression percentage
         * @return Target compression percentage (0.0 - 1.0)
         */
        double getTargetPercentage() {return targetPercentage;}

        /**
         * @brief Get the output image path
         * @return Output image path
         */
        string getOutputPath() {return outputPath;}

        /**
         * @brief Get the output GIF path
         * @return Output GIF path
         */
        string getGifPath() {return gifPath;}


        static void showAnimation() {
            using namespace std::chrono;
            using namespace std;

            const int GRID_SIZE = 5;
            const int FRAMES = 16;
        
            pair<int, int> pos[FRAMES] = {
                {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4},
                {1, 4}, {2, 4}, {3, 4}, {4, 4},
                {4, 3}, {4, 2}, {4, 1}, {4, 0},
                {3, 0}, {2, 0}, {1, 0}
            };

            
            bool first = true;
            int frame = 0;
            while (!done) {
                char grid[GRID_SIZE][GRID_SIZE];

                // Isi semua grid dengan spasi
                for (int i = 0; i < GRID_SIZE; ++i)
                    for (int j = 0; j < GRID_SIZE; ++j)
                        grid[i][j] = ' ';

                // Tempatkan titik di posisi saat ini
                auto [r, c] = pos[frame % FRAMES];
                grid[r][c] = '#';
                
                auto [r2, c2] = pos[(frame + FRAMES/2) % FRAMES];
                grid[r2][c2] = '#';

                // Tampilkan grid
                if (!first) {
                    for (int i = 0; i < GRID_SIZE; ++i) cout << "\x1b[A";
                }
                else first = !first;

                for (int i = 0; i < GRID_SIZE; ++i) {
                    for (int j = 0; j < GRID_SIZE; ++j) {
                        cout << grid[i][j] << ' ';
                    }
                    cout << '\n';
                }

                frame++;
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }

            for (int i = 0; i < GRID_SIZE; ++i) cout << "\x1b[A";
            for (int i = 0; i < GRID_SIZE; ++i) {
                for (int j = 0; j < GRID_SIZE+10; ++j) {
                    cout << ' ';
                }
                cout << endl;
            }
            for (int i = 0; i < GRID_SIZE; ++i) cout << "\x1b[A";
        }
};

#endif