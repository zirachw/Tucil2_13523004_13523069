#include "Input.hpp"

unsigned char* imgData = nullptr;
unsigned char* imgReal = nullptr;
unsigned char* img2Data = nullptr;

int main() {

    // Initialize input object to get image path and other parameters
    Input input;
    string imageInputPath = input.getImageInputPath();
    int mode = input.getMode();
    int threshold = input.getThreshold();
    int minimumBlock = input.getMinimumBlock();
    double targetPercentage = input.getTargetPercentage();
    string imageOutputPath = input.getImageOutputPath();
    string gifOutputPath = input.getGifOutputPath();
    
    QuadTree qt(imageInputPath, mode, threshold, minimumBlock, targetPercentage, imageOutputPath, gifOutputPath);
    
    //no target compression
    qt.performQuadTree();

    cout << "Performing quadtree compression..." << endl;
    stbi_image_free(imgData);
    free(imgReal);
    free(img2Data);

    // cout << "Performing quadtree compression with target compression..." << endl;
    // with target compression reducing 30% of the original image size
    // qt.performBinserQuadTree(0.1);

    cout << "Quadtree compression completed." << endl;
}