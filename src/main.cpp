#include "QuadTree.hpp"
#include <opencv2/core.hpp> 

cv::Mat img; 
cv::Mat real_img;

int main() {
    QuadTree qt;
    
    //no target compression
    //qt.perform_quadtree();

    //with target compression reducing 30% of the original image size
    qt.perform_bs_quadtree(0.3);
}