#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

class Image {
    int width; // original's image width
    int height; // original's image height
    int maxValue; // maximum value of the pixels of the original image
    uint8_t *pixels; // aray of pixels
public:
    Image(const char *path);
    Image(int width, int height);

    Image ResizeNearestNeighbour(int newWidth, int newHeight); // nearest neighbors resize method
    Image ResizeBilinear(int newWidth, int newHeight); // bilinear interpolation resize method
    Image ResizeBicubic(int newWidth, int newHeight); // bicubic interpolation method (B-spline?)

    Image SwitchResizeMethods(int newWidth, int newHeight, int mode); // resize method switching function
    Image Shift(int dx, int dy); // center shift method

    void Save(const char *path); // write image method

    ~Image();
};

Image::Image(const char *path) {
    ifstream f;
    f.open(path, ios::in | ios::binary); // opening file in read mode
    string format;
    string size;

    f >> format; // reading file format
    f >> width >> height; // get width and height from the file
    f >> maxValue; // get maximum pixel value from the file

    pixels = new uint8_t[height * width]; // section the memory for storing pixel array
    f.read((char *) pixels, height * width); // read data from the pixels
    f.close(); // close the file

    cout << "format: " << format << endl;
    cout << "size: " << width << " " << height << endl;
    cout << "max value: " << maxValue << endl;
}

Image::Image(int width, int height) {
    this->width = width; // saving image width
    this->height = height; // saving image height
    this->maxValue = 255; //setting the max value
    this->pixels = new uint8_t[height * width]; // section the memory for storing pixel array

    // start the pixel array with all 0's
    for (int i = 0; i < height * width; i++)
        pixels[i] = 0;
}

// Nearest Neighbors resize method
Image Image::ResizeNearestNeighbour(int newWidth, int newHeight) {
    Image result(newWidth, newHeight);

    // finding the size relationship of original image and desired resulting image
    double dh = (double)height / newHeight;
    double dw = (double)width / newWidth;

    // resizing process
    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            int y = min(int(i * dh), height - 1);
            int x = min(int(j * dw), width - 1);

            result.pixels[i * newWidth + j] = pixels[y * height + x]; // write the results into the pixels of the new image
        }
    }

    return result;
}

// bilinear interpolatin resize method 
Image Image::ResizeBilinear(int newWidth, int newHeight) {
    Image result(newWidth, newHeight);

    // find the relationship of sizes of the desired image and the original image
    double dh = (double)height / newHeight;
    double dw = (double)width / newWidth;

    // resizing process
    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            int y = min(int(i * dh), height - 2);
            int x = min(int(j * dw), width - 2);

            double dy = i * dh - y;
            double dx = j * dw - x;

            double v = 0; // do bilinear interpolation
            v += (1 - dx) * (1 - dy) * pixels[y * width + x];
            v += dx * (1 - dy) * pixels[y * width + x + 1];
            v += (1 - dx) * dy * pixels[(y + 1) * width + x];
            v += dx * dy * pixels[(y + 1) * width + x + 1];

            result.pixels[i * newWidth + j] = (int8_t) min(255.0, max(0.0, v)); // write the results into the pixels of the new image
        }
    }

    return result; 
}

// bicubic interpolation resize method
Image Image::ResizeBicubic(int newWidth, int newHeight) {
    Image result(newWidth, newHeight);

    // finding the size relationship of original image and desired resulting image
    double dh = (double)height / newHeight;
    double dw = (double)width / newWidth;

    // resizing process
    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            int x = max(1, min(int(j * dw), width - 3));
            int y = max(1, min(int(i * dh), height - 3));

            double dx = j * dw - x;
            double dy = i * dh - y;

            double v = 0; // do bicubic interpolation
            v += 1.0 / 4 * (dx - 1) * (dx - 2) * (dx + 1) * (dy - 1) * (dy - 2) * (dy + 1) * pixels[y * width + x];
            v += -1.0 / 4 * dx * (dx + 1) * (dx - 2) * (dy - 1) * (dy - 2) * (dy + 1) * pixels[y * width + x + 1];
            v += -1.0 / 4 * dy * (dx - 1) * (dx - 2) * (dx + 1) * (dy + 1) * (dy - 2) * pixels[(y + 1) * width + x];
            v += 1.0 / 4 * dx * dy * (dx + 1) * (dx - 2) * (dy + 1) * (dy - 2) * pixels[(y + 1) * width + x + 1];
            v += -1.0 / 12 * dx * (dx - 1) * (dx - 2) * (dy - 1) * (dy - 2) * (dy + 1) * pixels[y * width + x - 1];
            v += -1.0 / 12 * dy * (dx - 1) * (dx - 2) * (dx + 1) * (dy - 1) * (dy - 2) * pixels[(y - 1) * width + x];
            v += 1.0 / 12 * dx * dy * (dx - 1) * (dx - 2) * (dy + 1) * (dy - 2) * pixels[(y + 1) * width + x - 1];
            v += 1.0 / 12 * dx * dy * (dx + 1) * (dx - 2) * (dy - 1) * (dy - 2) * pixels[(y - 1) * width + x + 1];
            v += 1.0 / 12 * dx * (dx - 1) * (dx + 1) * (dy - 1) * (dy - 2) * (dy + 1) * pixels[y * width + x + 2];
            v += 1.0 / 12 * dy * (dx - 1) * (dx - 2) * (dx + 1) * (dy - 1) * (dy + 1) * pixels[(y + 2) * width + x];
            v += 1.0 / 36 * dx * dy * (dx - 1) * (dx - 2) * (dy - 1) * (dy - 2) * pixels[(y - 1) * width + x - 1];
            v += -1.0 / 12 * dx * dy * (dx - 1) * (dx + 1) * (dy + 1) * (dy - 2) * pixels[(y + 1) * width + x + 2];
            v += -1.0 / 12 * dx * dy * (dx + 1) * (dx - 2) * (dy - 1) * (dy + 1) * pixels[(y + 2) * width + x + 1];
            v += -1.0 / 36 * dx * dy * (dx - 1) * (dx + 1) * (dy - 1) * (dy - 2) * pixels[(y - 1) * width + x + 2];
            v += -1.0 / 36 * dx * dy * (dx - 1) * (dx - 2) * (dy - 1) * (dy + 1) * pixels[(y + 2) * width + x - 1];
            v += 1.0 / 36 * dx * dy * (dx - 1) * (dx + 1) * (dy - 1) * (dy + 1) * pixels[(y + 2) * width + x + 2];

            result.pixels[i * newWidth + j] = (uint8_t) min(255.0, max(0.0, v)); // write the resultsi nto the pixels of the new image
        }
    }

    return result; 
}

// resize method switching function
Image Image::SwitchResizeMethods(int newWidth, int newHeight, int mode) {
    if (mode == 1)
        return ResizeBilinear(newWidth, newWidth);

    if (mode == 2)
        return ResizeBicubic(newWidth, newWidth);

    return ResizeNearestNeighbour(newWidth, newWidth);
}

// center displacement function
Image Image::Shift(int dx, int dy) {
    Image result(width, height); // create a new image to save the displaced results

    // go through each pixel
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // if after displacement there is not correct coordinate
            if (i - dy < 0 || i - dy >= height || j - dx < 0 || j - dx >= width)
                continue; // then just ignore it

            result.pixels[i * width + j] = pixels[(i - dy) * width + j - dx]; // do displacement
        }
    }

    return result; 
}

// saving the image
void Image::Save(const char *path) {
    ofstream f;

    f.open(path, ios::out | ios::binary); // create a new file

    f << "P5" << endl; // write the type
    f << width << " " << height << endl; // write the size
    f << maxValue << endl; // write the maximum pixel value

    f.write((const char *)pixels, width * height); // write the pixel matrix
    f.close(); 
}

Image::~Image() {
    delete[] pixels; // free memory (!)
}

int main(int argc, char **argv) {
    if (argc != 8) { // if the given argument in command line is none of the available
        cout << "Invalid arguments" << endl; // let user know
        cout << "Usage: ./resizer [input path] [output path] [new width] [new height] [dx] [dy] [mode]" << endl;
        cout << "mode: 0 for nearest neighbour, 1 for bilinear interpolation, 2 for bicubic interpolation" << endl;
        return -1; // and exit
    }

    char *inputPath = argv[1]; // input image <input.pgm>
    char *outputPath = argv[2]; // output image <output.pgm>
    int width = atoi(argv[3]); // new width after resize <width>
    int height = atoi(argv[4]); // new height after resize <height>
    int dx = atoi(argv[5]); // center displacement on x <dx>
    int dy = atoi(argv[6]); // center displacement on y <dy>
    int mode = atoi(argv[7]); // resize method <method>

    Image source(inputPath); // read image
    Image resized = source.SwitchResizeMethods(width, height, mode); // resize the image
    Image shifted = resized.Shift(dx, dy); // displace the image

    shifted.Save(outputPath); // save resulting image
}
