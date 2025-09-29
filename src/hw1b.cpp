#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <filesystem>  // 建立資料夾
using namespace std;
namespace fs = std::filesystem;

const int WIDTH = 512;
const int HEIGHT = 512;

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;

    uint32_t dib_header_size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t colors_used;
    uint32_t important_colors;
};
#pragma pack(pop)


// ---------------- RAW 讀檔 ----------------
vector<unsigned char> readRaw(const string& filename, int& width, int& height) {
    width = WIDTH;
    height = HEIGHT;
    vector<unsigned char> image(width * height);

    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Can't open RAW file: " << filename << endl;
        return {};
    }

    file.read((char*)image.data(), width * height);
    file.close();
    return image;
}


// ---------------- BMP 讀檔 ----------------
vector<unsigned char> readBMP(const string& filename, int& width, int& height) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Can't open BMP file: " << filename << endl;
        return {};
    }

    BMPHeader header;
    file.read((char*)&header, sizeof(BMPHeader));
    width = header.width;
    height = header.height;
    int bpp = header.bits_per_pixel;

    if (header.type != 0x4D42 || header.compression != 0) {
        cerr << "Unsupported BMP format: " << filename << endl;
        return {};
    }

    int row_padded = (width * (bpp/8) + 3) & (~3);
    vector<unsigned char> data(row_padded * height);
    file.seekg(header.offset, ios::beg);
    file.read((char*)data.data(), data.size());
    file.close();

    vector<unsigned char> gray(width * height);

    if (bpp == 24) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int idx = i * row_padded + j * 3;
                unsigned char B = data[idx];
                unsigned char G = data[idx+1];
                unsigned char R = data[idx+2];
                gray[(height-1-i) * width + j] = (R + G + B) / 3;
            }
        }
    } else if (bpp == 8) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int idx = i * row_padded + j;
                gray[(height-1-i) * width + j] = data[idx];
            }
        }
    }
    return gray;
}


// ---------------- 影像增強方法 ----------------
vector<unsigned char> logTransform(const vector<unsigned char>& img) {
    vector<unsigned char> out(img.size());
    float c = 255.0 / log(1 + 255.0);
    for (size_t i = 0; i < img.size(); i++) {
        out[i] = (unsigned char)(c * log(1 + img[i]));
    }
    return out;
}

vector<unsigned char> gammaTransform(const vector<unsigned char>& img, float gamma) {
    vector<unsigned char> out(img.size());
    float c = pow(255.0, 1 - gamma);
    for (size_t i = 0; i < img.size(); i++) {
        out[i] = (unsigned char)(c * pow(img[i], gamma));
    }
    return out;
}

vector<unsigned char> negativeTransform(const vector<unsigned char>& img) {
    vector<unsigned char> out(img.size());
    for (size_t i = 0; i < img.size(); i++) {
        out[i] = 255 - img[i];
    }
    return out;
}


// ---------------- 存檔 ----------------
void saveImage(const string& outDir, const string& filename, 
               const vector<unsigned char>& img, int width, int height) {
    fs::create_directories(outDir); // 若資料夾不存在則建立
    string outPath = outDir + "/" + filename;
    stbi_write_png(outPath.c_str(), width, height, 1, img.data(), width);
}


// ---------------- 主程式 ----------------
int main() {
    string outDir = "output/transform";  

    vector<string> rawFiles = {"data/lena.raw", "data/goldhill.raw", "data/peppers.raw"};
    vector<string> bmpFiles = {"data/baboon.bmp", "data/boat.bmp", "data/F16.bmp"};

    // RAW
    for (auto& f : rawFiles) {
        int w, h;
        auto img = readRaw(f, w, h);
        if (img.empty()) continue;

        string base = fs::path(f).stem().string(); // 去掉副檔名
        saveImage(outDir, base + "_log.png",     logTransform(img), w, h);
        saveImage(outDir, base + "_gamma05.png", gammaTransform(img, 0.5), w, h);
        saveImage(outDir, base + "_gamma20.png", gammaTransform(img, 2.0), w, h);
        saveImage(outDir, base + "_neg.png",     negativeTransform(img), w, h);

        cout << "Processed RAW: " << f << endl;
    }

    // BMP
    for (auto& f : bmpFiles) {
        int w, h;
        auto img = readBMP(f, w, h);
        if (img.empty()) continue;

        string base = fs::path(f).stem().string();
        saveImage(outDir, base + "_log.png",     logTransform(img), w, h);
        saveImage(outDir, base + "_gamma05.png", gammaTransform(img, 0.5), w, h);
        saveImage(outDir, base + "_gamma20.png", gammaTransform(img, 2.0), w, h);
        saveImage(outDir, base + "_neg.png",     negativeTransform(img), w, h);

        cout << "Processed BMP: " << f << endl;
    }

    return 0;
}
