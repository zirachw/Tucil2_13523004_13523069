#include "core/IO.hpp"

/**
 * @brief Image data buffers used throughout the compression process
 * @param currImgData Current image data buffer used for processing
 * @param initImgData Initial image data buffer kept as reference
 * @param tempImgData Temporary image data buffer for intermediate processing
 */
unsigned char *currImgData = nullptr, *initImgData = nullptr, *tempImgData = nullptr;
extern int compressionQuality;

int main()
{
    // ~~ IO ~~
    IOHandler IO;
    cout << BRIGHT_YELLOW << "Input" << BRIGHT_GREEN << " done." << endl;


    //~~ Quadtree Compression Process ~~
    QuadTree qt(IO.getInputPath(), 
                IO.getMode(), 
                IO.getThreshold(), 
                IO.getMinBlock(),
                IO.getTargetPercentage(), 
                IO.getOutputPath(), 
                IO.getGifPath(), 
                IO.getInputExtension());

    cout << RESET BRIGHT_CYAN << "Performing quadtree compression..." << endl;

    if (IO.getTargetPercentage() == 0) qt.performQuadTree();
    else qt.performBinserQuadTree(IO.getTargetPercentage());

    cout << BRIGHT_YELLOW << "Quadtree compression" << BRIGHT_GREEN << " done." << endl << endl;

    
    //~~ Output Results ~~
    cout << BRIGHT_WHITE ITALIC << "~" << BRIGHT_YELLOW << " Results " << BRIGHT_WHITE "~" << endl;
    cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " Executing time: " << BRIGHT_GREEN << qt.getExecutionTime() << " ms" << endl;
    cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " Initial size: " << BRIGHT_GREEN << qt.getInitialSize() << " bytes (" << Image::getSizeInKB(qt.getInitialSize()) << " KB)" << endl;
    cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " Final size: " << BRIGHT_GREEN << qt.getFinalSize() << " bytes (" << Image::getSizeInKB(qt.getFinalSize()) << " KB)" << endl;
    cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " Compression percentage: " << BRIGHT_GREEN << qt.getCompressionPercentage() << " %" << endl;
    cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " Quadtree depth: " << BRIGHT_GREEN << qt.getQuadtreeDepth() << endl;
    cout << RESET MAGENTA BOLD << "[-]" << RESET BRIGHT_WHITE << " Quadtree node: " << BRIGHT_GREEN << qt.getQuadtreeNode() << endl;
    cout << endl;

    
    //~~ Free Memory ~~
    if (currImgData != nullptr) {
        free(currImgData);
        currImgData = nullptr;
    }

    if (initImgData != nullptr) {
        free(initImgData);
        initImgData = nullptr;
    }

    if (tempImgData != nullptr) {
        free(tempImgData);
        tempImgData = nullptr;
    }

    cout << RESET;
    return 0;
}