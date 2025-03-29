#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "QuadTreeNode.hpp"

extern unsigned char* img_data; 
extern unsigned char* img_real;  
extern unsigned char* img2_data;
int img_width, img_height, img_channels;

struct ImageBuffer {
    vector<unsigned char> buf;
    static void write_func(void* context, void* data, int size) {
        ImageBuffer* b = (ImageBuffer*)context;
        b->buf.insert(b->buf.end(), (unsigned char*)data, (unsigned char*)data + size);
    }
};

size_t get_encoded_jpeg_size(unsigned char* image, int w, int h) {
    ImageBuffer out;
    stbi_write_jpg_to_func(ImageBuffer::write_func, &out, w, h, 3, image, 90);
    return out.buf.size();
}

class QuadTree {
  private:
    int width, height, channel;
    int mode;
    int minimum_block;
    int threshold;
    string image_input_path;
    string image_output_path;
    string gif_output_path;

    bool last; 
    //last == true if we need to write the image to gif
    //last become false if we need to perform binary search for the best threshold
    //[BONUS : Compression Percentage]
    
    QuadTreeNode root;
    GifWriter g;
    uint8_t* data;
  
  public:
    QuadTree() {
        cout << "Enter image input path\n";
        cout << "Example : D:\\something_in.jpg\n";
        cin >> image_input_path;

        cout << "Enter image output path\n";
        cout << "Example : D:\\something_out.jpg\n";
        cin >> image_output_path;

        cout << "Enter gif output path\n";
        cout << "Example : D:\\something_out.gif\n";
        cin >> gif_output_path;

        get_image();
        mode = 1;
        root = QuadTreeNode(0, 0, height, width, 0, mode);
        minimum_block = 1;
        threshold = 100;
        last = true;

        char* gif_output_path_c = new char[gif_output_path.length() + 1];
        strcpy(gif_output_path_c, gif_output_path.c_str());

        GifBegin(&g, gif_output_path_c, width, height, 50);
        data = (uint8_t*)malloc(width * height * 4);
    }

    void get_image() {
        img_data = stbi_load(image_input_path.c_str(), &width, &height, &channel, 3);
        if (!img_data) {
            cout << "Could not open or find the image\n";
            return;
        }
    
        channel = 3; // Force RGB
        img_real = (unsigned char*)malloc(width * height * 3);
        memcpy(img_real, img_data, width * height * 3);
    
        img2_data = (unsigned char*)malloc(width * height * 3);
        memcpy(img2_data, img_data, width * height * 3);

        img_width = width;
        img_height = height;
        img_channels = channel;
        cout << "Image loaded successfully\n";
        cout << "Image size: " << width << "x" << height << ", Channels: " << channel << endl;
    }

    void write_fake_image_to_gif() {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                int idx = (j * width + i) * 3;
                int out_idx = (j * width + i) * 4;

                data[out_idx + 0] = img2_data[idx + 0];
                data[out_idx + 1] = img2_data[idx + 1];
                data[out_idx + 2] = img2_data[idx + 2];
                data[out_idx + 3] = 255;
            }
        }

        GifWriteFrame(&g, data, width, height, 100);
    }

    void write_image_real(string path) {
        stbi_write_jpg(path.c_str(), width, height, 3, img_data, 90);
    }
    
    void write_image_fake(string path) {
        stbi_write_jpg(path.c_str(), width, height, 3, img2_data, 90);
    }

    void perform_quadtree() {
        queue<QuadTreeNode> q;
        q.push(root);
        int curMaxStep = 0;
        //img2 = img.clone();
        memcpy(img2_data, img_data, width * height * 3);

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
                        //write_image_fake("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna0" + to_string(curMaxStep) + ".jpg");
                        //GifWriteFrame(&g, img2.data, this->width, this->height, 100);
                    }
                }
                else {
                    if (last) {
                        write_fake_image_to_gif();
                        //write_image_fake("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\lenna" + to_string(curMaxStep) + ".jpg");
                        //GifWriteFrame(&g, img2.data, this->width, this->height, 100);
                    }
                }

                memcpy(img2_data, img_data, width * height * 3);
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
                    int idx = (j * this->width + i) * 3;
                    int out_idx = (j * this->width + i) * 4;

                    data[out_idx + 0] = img_data[idx + 0];
                    data[out_idx + 1] = img_data[idx + 1];
                    data[out_idx + 2] = img_data[idx + 2];
                    data[out_idx + 3] = 255;
                }
            }

            write_image_real(image_output_path);
            GifWriteFrame(&g, data, this->width, this->height, 100);

            GifEnd(&g);
            free(data);
        }
    }

    void perform_bs_quadtree(double ratio) {
        double l = 0.0, r = 5000.0;

        // IMPORTANT : NEED IF ELSE TO HANDLE RANGE OF L AND R
        // IMPORTANT : HANDLE PNG/JPG FORMAT (?)

        //imwrite("D:\\Tugas ITB\\Stima\\Tucil 2\\Tucil2_13523004_13523069\\src\\output\\aveORI.jpg", img);

        size_t bef_img_size = get_encoded_jpeg_size(img_data, width, height);
        size_t target_img_size = bef_img_size - (bef_img_size * ratio);

        cout << "original size : " << bef_img_size << endl;
        cout << "target size : " << target_img_size << endl;

        int best_threshold = -1;
        last = false;

        for (int i = 1; i <= 13; i++) {
            double mid = (l + r) / 2;
            threshold = mid;
            perform_quadtree();
 
            size_t cur_img_size = get_encoded_jpeg_size(img_data, width, height);

            cout << "threshold : " << mid << " size : " << cur_img_size << endl;

            if (cur_img_size <= target_img_size) {
                best_threshold = mid;
                r = mid;
            } else {
                l = mid;
            }
            memcpy(img_data, img_real, width * height * 3);
        }

        last = true;
        threshold = best_threshold;
        perform_quadtree();
    }
};

#endif