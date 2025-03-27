#include "QuadTree.hpp"
#include <opencv2/core.hpp> 

cv::Mat img; 

int main() {
    QuadTree qt;
    qt.perform_quadtree();
}