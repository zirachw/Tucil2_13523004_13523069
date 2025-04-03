#include "core/Input.hpp"

// Global variables for image data
unsigned char *currImgData = nullptr;
unsigned char *initImgData = nullptr;
unsigned char *tempImgData = nullptr;

int main()
{
    // ~~ Input Handler ~~
    Input input;
    cout << "Input done." << endl;

    //~~ Quadtree Compression Process ~~
    QuadTree qt(input.getInputPath(), input.getMode(), input.getThreshold(), input.getMinBlock(),
                input.getTargetPercentage(), input.getOutputPath(), input.getGifPath(), input.getInputExtension());
    cout << "Performing quadtree compression..." << endl;

    if (input.getTargetPercentage() == 0) qt.performQuadTree();
    else qt.performBinserQuadTree(input.getTargetPercentage());
    cout << "Quadtree compression completed." << endl << endl;

    //~~ Output Results ~~
    cout << "~ Results ~" << endl;
    cout << "[-] Executing time: " << qt.getExecutionTime() << " ms" << endl;
    cout << "[-] Initial size: " << qt.getInitialSize() << " bytes (" << Image::getSizeInKB(qt.getInitialSize()) << " KB)" << endl;
    cout << "[-] Final size: " << qt.getFinalSize() << " bytes (" << Image::getSizeInKB(qt.getFinalSize()) << " KB)" << endl;
    cout << "[-] Compression percentage: " << qt.getCompressionPercentage() << " %" << endl;
    cout << "[-] Quadtree depth: " << qt.getQuadtreeDepth() << endl;
    cout << "[-] Quadtree node: " << qt.getQuadtreeNode() << endl;
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

    return 0;
}