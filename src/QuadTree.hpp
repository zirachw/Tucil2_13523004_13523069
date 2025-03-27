#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "QuadTreeNode.hpp"

extern cv::Mat real_img;
extern cv::Mat img;
cv::Mat img2;

class QuadTree {
  private:
    int width, height;
    int mode;
    int minimum_block;
    int threshold;

    bool last; 
    //last == true if we need to write the image to gif
    //last become false if we need to perform binary search for the best threshold
    //[BONUS : Compression Percentage]
    
    QuadTreeNode root;
    GifWriter g;
    uint8_t* data;
  
  public:
    QuadTree() {
        get_image();
        mode = 1;
        root = QuadTreeNode(0, 0, height, width, 0, mode);
        minimum_block = 1;
        threshold = 300;
        last = true;

        GifBegin(&g, "D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna.gif", width, height, 50);
        data = (uint8_t*)malloc(width * height * 4);
    }

    void get_image() {
        img = imread("D:\\lenna.png", IMREAD_COLOR);
        if (img.empty()) {
            cout << "Could not open or find the image" << endl;
            return;
        }
        real_img = img.clone();
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

            if (node.getWidth() == 0 || node.getHeight() == 0 || node.getWidth()*node.getHeight() <= minimum_block) {
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
                    if (last) {
                        write_fake_image_to_gif();
                        //write_image_fake("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna0" + to_string(curMaxStep) + ".png");
                        //GifWriteFrame(&g, img2.data, this->width, this->height, 100);
                    }
                }
                else {
                    if (last) {
                        write_fake_image_to_gif();
                        //write_image_fake("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna" + to_string(curMaxStep) + ".png");
                        //GifWriteFrame(&g, img2.data, this->width, this->height, 100);
                    }
                }

                img2 = img.clone();
            }

            double error = node.getError();

            if (error < threshold) {
                node.fill_rectangle_real();
                if (last) {
                    node.fill_rectangle_fake();
                }
                continue;
            } else {
                if (last) {
                    node.fill_rectangle_fake();
                }
                q.push(QuadTreeNode(X, Y, height / 2, width / 2, step + 1, mode));
                q.push(QuadTreeNode(X + height / 2, Y, height - height / 2, width / 2, step + 1, mode));
                q.push(QuadTreeNode(X, Y + width / 2, height / 2, width - width / 2, step + 1, mode));
                q.push(QuadTreeNode(X + height / 2, Y + width / 2, height - height / 2, width - width / 2, step + 1, mode));
            }
        }

        if (last) {
            for (int i = 0; i < this->width; i++) {
                for (int j = 0; j < this->height; j++) {
                    Vec3b color = img2.at<Vec3b>(j, i);
                    data[(j * this->width + i) * 4] = color[2];
                    data[(j * this->width + i) * 4 + 1] = color[1];
                    data[(j * this->width + i) * 4 + 2] = color[0];
                    data[(j * this->width + i) * 4 + 3] = 255;
                }
            }

            write_image_real("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lennaRES.png");
            GifWriteFrame(&g, data, this->width, this->height, 100);

            GifEnd(&g);
            free(data);
        }
    }

    void perform_bs_quadtree(double ratio) {
        double l = 0.0, r = 5000.0;

        // IMPORTANT : NEED IF ELSE TO HANDLE RANGE OF L AND R
        // IMPORTANT : HANDLE PNG/JPG FORMAT (?)

        vector<uchar> buf;
        imencode(".png", img, buf);
        size_t bef_img_size = buf.size();
        size_t target_img_size = bef_img_size - (bef_img_size * ratio);

        // cout << "original size : " << bef_img_size << endl;
        // cout << "target size : " << target_img_size << endl;

        int best_threshold = -1;
        last = false;

        for (int i = 1; i <= 15; i++) {
            double mid = (l + r) / 2;
            threshold = mid;
            perform_quadtree();

            vector<uchar> buf;
            imencode(".png", img, buf);   
            size_t cur_img_size = buf.size();

            // cout << "threshold : " << mid << " size : " << cur_img_size << endl;

            if (cur_img_size <= target_img_size) {
                best_threshold = mid;
                r = mid;
            } else {
                l = mid;
            }

            img = real_img.clone();
        }

        last = true;
        threshold = best_threshold;
        perform_quadtree();
    }
};

#endif