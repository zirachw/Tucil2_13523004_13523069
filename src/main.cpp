#include "QuadTree.hpp"

unsigned char* img_data = nullptr;
unsigned char* img_real = nullptr;
unsigned char* img2_data = nullptr;

int main() {
    QuadTree qt;
    
    //no target compression
    qt.perform_quadtree();

    stbi_image_free(img_data);
    free(img_real);
    free(img2_data);

    //with target compression reducing 30% of the original image size
    //qt.perform_bs_quadtree(0.1);
}