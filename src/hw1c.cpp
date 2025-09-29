#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <filesystem>
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

// ---------------- 最近鄰插值 ----------------
vector<unsigned char> resizeNearest(const vector<unsigned char>& img, int w, int h, int newW, int newH) {
    vector<unsigned char> out(newW * newH);
    float scaleX = (float)w / newW;
    float scaleY = (float)h / newH;

    for (int y = 0; y < newH; y++) {
        for (int x = 0; x < newW; x++) {
            int srcX = round(x * scaleX);
            int srcY = round(y * scaleY);
            if (srcX >= w) srcX = w - 1;
            if (srcY >= h) srcY = h - 1;
            out[y * newW + x] = img[srcY * w + srcX];
        }
    }
    return out;
}

// ---------------- 雙線性插值 ----------------
vector<unsigned char> resizeBilinear(const vector<unsigned char>& img, int w, int h, int newW, int newH) {
    vector<unsigned char> out(newW * newH);
    float scaleX = (float)(w - 1) / (newW - 1);
    float scaleY = (float)(h - 1) / (newH - 1);

    for (int y = 0; y < newH; y++) {
        for (int x = 0; x < newW; x++) {
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            int x1 = floor(srcX);
            int y1 = floor(srcY);
            int x2 = min(x1 + 1, w - 1);
            int y2 = min(y1 + 1, h - 1);

            float a = srcX - x1;
            float b = srcY - y1;

            float val = (1 - a) * (1 - b) * img[y1 * w + x1] +
                        a * (1 - b) * img[y1 * w + x2] +
                        (1 - a) * b * img[y2 * w + x1] +
                        a * b * img[y2 * w + x2];

            out[y * newW + x] = (unsigned char)val;
        }
    }
    return out;
}

// ---------------- 存檔 ----------------
void saveImage(const string& outDir, const string& filename,
               const vector<unsigned char>& img, int width, int height) {
    fs::create_directories(outDir);
    string outPath = outDir + "/" + filename;
    stbi_write_png(outPath.c_str(), width, height, 1, img.data(), width);
}

// ────────── 針對單一影像輸出 ──────────
void processOne(const string& baseName,
                const vector<unsigned char>& img, int w, int h,
                const string& outDir)
{
    // (1) 512 -> 128
    auto nn_128  = resizeNearest (img, w, h, 128, 128);
    auto bi_128  = resizeBilinear(img, w, h, 128, 128);
    saveImage(outDir, baseName + "_nn_128.png", nn_128, 128, 128);
    saveImage(outDir, baseName + "_bilinear_128.png", bi_128, 128, 128);

    // (2) 512 -> 32
    auto nn_32   = resizeNearest (img, w, h, 32, 32);
    auto bi_32   = resizeBilinear(img, w, h, 32, 32);
    saveImage(outDir, baseName + "_nn_32.png", nn_32, 32, 32);
    saveImage(outDir, baseName + "_bilinear_32.png", bi_32, 32, 32);

    // (3) 32 -> 512（這裡各自用「它的」32x32 來放大，便於比較差異）
    auto nn_32to512 = resizeNearest (nn_32, 32, 32, 512, 512);
    auto bi_32to512 = resizeBilinear(bi_32, 32, 32, 512, 512);
    saveImage(outDir, baseName + "_nn_32to512.png", nn_32to512, 512, 512);
    saveImage(outDir, baseName + "_bilinear_32to512.png", bi_32to512, 512, 512);

    // (4) 512 -> 1024x512（寬放大 2 倍，髙不變）
    auto nn_1024x512 = resizeNearest (img, w, h, 1024, 512);
    auto bi_1024x512 = resizeBilinear(img, w, h, 1024, 512);
    saveImage(outDir, baseName + "_nn_1024x512.png", nn_1024x512, 1024, 512);
    saveImage(outDir, baseName + "_bilinear_1024x512.png", bi_1024x512, 1024, 512);

    // (5) 128 -> 256x512（先等比縮到 128x128，再非等比放大）
    auto nn_128src   = resizeNearest (img, w, h, 128, 128);
    auto bi_128src   = resizeBilinear(img, w, h, 128, 128);
    auto nn_256x512  = resizeNearest (nn_128src, 128, 128, 256, 512);
    auto bi_256x512  = resizeBilinear(bi_128src, 128, 128, 256, 512);
    saveImage(outDir, baseName + "_nn_256x512.png", nn_256x512, 256, 512);
    saveImage(outDir, baseName + "_bilinear_256x512.png", bi_256x512, 256, 512);
}

// ────────── 主程式：處理 data/ 的 6 張圖 ──────────
int main() {
    const string outDir = "output/interpolation";

    // 3 張 RAW（固定 512x512）
    vector<string> raws = {"data/lena.raw", "data/goldhill.raw", "data/peppers.raw"};
    for (auto& path : raws) {
        int w, h;
        auto img = readRaw(path, w, h);
        if (img.empty()) continue;
        string base = fs::path(path).stem().string(); 
        cout << "Done: " << base << "\n";
    }

    // 3 張 BMP（讀入後轉灰階）
    vector<string> bmps = {"data/baboon.bmp", "data/boat.bmp", "data/F16.bmp"};
    for (auto& path : bmps) {
        int w, h;
        auto img = readBMP(path, w, h);
        if (img.empty()) continue;
        string base = fs::path(path).stem().string(); 
        processOne(base, img, w, h, outDir);
        cout << "Done: " << base << "\n";
    }

    cout << "All results saved to: " << outDir << "\n";
    return 0;
}