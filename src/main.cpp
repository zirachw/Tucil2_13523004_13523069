#include "core/Input.hpp"

unsigned char *currImgData = nullptr;
unsigned char *initImgData = nullptr;
unsigned char *tempImgData = nullptr;

int main()
{
    cout << CLEAR_SCREEN;
    cout << "Welcome to Quadtree Compression!" << endl;
    Input input;

    cout << "Input done." << endl;
    QuadTree qt(input.getInputPath(), input.getMode(), input.getThreshold(), input.getMinBlock(),
                input.getTargetPercentage(), input.getOutputPath(), input.getGifPath());

    cout << "QuadTree initialized, target percentage: " << input.getTargetPercentage() << endl;
    cout << "Performing quadtree compression..." << endl;
    if (input.getTargetPercentage() == 0) qt.performQuadTree();
    else qt.performBinserQuadTree(input.getTargetPercentage());

    cout << "Quadtree compression completed." << endl;
    cout << "Executing time: " << qt.getExecutionTime() << "ms" << endl;
    cout << "Initial size: " << qt.getInitialSize() << endl;
    cout << "Final size: " << qt.getFinalSize() << endl;
    cout << "Compression percentage: " << qt.getCompressionPercentage() << endl;
    cout << "Quadtree depth: " << qt.getQuadtreeDepth() << endl;
    cout << "Quadtree node: " << qt.getQuadtreeNode() << endl;

    stbi_image_free(currImgData);
    free(initImgData);
    free(tempImgData);
}