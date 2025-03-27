#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "QuadTreeNode.hpp"

extern cv::Mat img;
cv::Mat img2;


class QuadTree {
  private:
    int width, height;
    int mode;
    QuadTreeNode root;
    GifWriter g;
    uint8_t* data;
  
  public:
    QuadTree() {
        get_image();
        cout << width << " " << height << endl;
        GifBegin(&g, "D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna.gif", width+5, height+5, 100);
        mode = 1;
        root = QuadTreeNode(0, 0, height, width, 0, mode);
        data = (uint8_t*)malloc(width * height * 4);
    }

    void get_image() {
        img = imread("D:\\lenna.png", IMREAD_COLOR);
        if (img.empty()) {
            cout << "Could not open or find the image" << endl;
            return;
        }
        width = img.cols;
        height = img.rows;
    }

    void write_fake_image_to_gif() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                Vec3b color = img2.at<Vec3b>(j, i);
                data[(j * width + i) * 4] = color[2];
                data[(j * width + i) * 4 + 1] = color[1];
                data[(j * width + i) * 4 + 2] = color[0];
                data[(j * width + i) * 4 + 3] = 255;
            }
        }

        GifWriteFrame(&g, data, width, height, 100);
    }

    void write_image_real(string path) {
        imwrite(path, img);
    }

    void write_image_fake(string path) {
        imwrite(path, img2);
    }

    void perform_quadtree() {
        queue<QuadTreeNode> q;
        q.push(root);
        int curMaxStep = 0;
        img2 = img.clone();

        while (!q.empty()) {
            QuadTreeNode node = q.front();
            q.pop();

            if (node.getWidth() == 0 || node.getHeight() == 0) {
                continue;
            }
            
            int step = node.getStep();
            int X = node.getX();
            int Y = node.getY();
            int width = node.getWidth();
            int height = node.getHeight();

            if (step > curMaxStep) {
                curMaxStep = step;
                if (curMaxStep < 10) {
                    //write_image_fake("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna0" + to_string(curMaxStep) + ".jpg");

                    write_fake_image_to_gif();
                    //GifWriteFrame(&g, img2.data, this->width, this->height, 100);
                }
                else {
                    //write_image_fake("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna" + to_string(curMaxStep) + ".jpg");
                    //GifWriteFrame(&g, img2.data, this->width, this->height, 100);
                    write_fake_image_to_gif();
                }

                img2 = img.clone();
            }

            double error = node.getError();
            double threshold = node.getThreshold();

            if (error < threshold) {
                node.fill_rectangle_real();
                node.fill_rectangle_fake();
                continue;
            } else {
                node.fill_rectangle_fake();
                q.push(QuadTreeNode(X, Y, height / 2, width / 2, step + 1, mode));
                q.push(QuadTreeNode(X + height / 2, Y, height - height / 2, width / 2, step + 1, mode));
                q.push(QuadTreeNode(X, Y + width / 2, height / 2, width - width / 2, step + 1, mode));
                q.push(QuadTreeNode(X + height / 2, Y + width / 2, height - height / 2, width - width / 2, step + 1, mode));
            }
        }
        
        for (int i = 0; i < this->width; i++) {
            for (int j = 0; j < this->height; j++) {
                Vec3b color = img2.at<Vec3b>(j, i);
                data[(j * this->width + i) * 4] = color[2];
                data[(j * this->width + i) * 4 + 1] = color[1];
                data[(j * this->width + i) * 4 + 2] = color[0];
                data[(j * this->width + i) * 4 + 3] = 255;
            }
        }

        write_image_real("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lennaRES.jpg");
        GifWriteFrame(&g, data, this->width, this->height, 100);
    }
};

#endif