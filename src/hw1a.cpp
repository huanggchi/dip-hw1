#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <filesystem>   

using namespace std;
namespace fs = std::filesystem;

const int WIDTH = 512;
const int HEIGHT = 512;

// ------------------ RAW 處理 ------------------
void processRaw(const string& rawfile, const string& pngname) {
    unsigned char image[HEIGHT][WIDTH];

    ifstream file(rawfile, ios::binary);
    if (!file) {
        cout << "Can't open " << rawfile << endl;
        return;
    }
    file.read((char*)image, WIDTH * HEIGHT);
    file.close();

    // 確保 output 資料夾存在
    fs::create_directories("output");

    // 存成 PNG (灰階)，輸出到 output 資料夾
    string outpath = "output/" + pngname;
    stbi_write_png(outpath.c_str(), WIDTH, HEIGHT, 1, image, WIDTH);

    // 輸出中心 10×10
    // 影像大小為 512×512，中心位置約為 (256, 256)
    // 若要取出以中心點為基準的 10×10 區塊：
    //   256 - 5 = 251  → 起始索引
    //   256 + 5 = 261  → 結束索引 (迴圈不包含261，實際範圍是251~260，共10個像素)
    // 因此 i 與 j 都從 251 跑到 260，正好擷取中心的 10×10 區域。
    cout << "Center 10x10 pixels of " << rawfile << ":" << endl;
    for (int i = 251; i < 261; i++) {
        for (int j = 251; j < 261; j++) {
            cout << (int)image[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

// ------------------ BMP 處理 ------------------
#pragma pack(push, 1)
struct BMPHeader {
    uint16_t type;          
    uint32_t size;          
    uint16_t reserved1;     
    uint16_t reserved2;     
    uint32_t offset;        
    
    uint32_t dib_header_size; 
    int32_t width;            
    int32_t height;           
    uint16_t planes;          
    uint16_t bits_per_pixel;  
    uint32_t compression;     
    uint32_t image_size;      
    int32_t x_ppm;            
    int32_t y_ppm;            
    uint32_t colors_used;     
    uint32_t important_colors;
};
#pragma pack(pop)


void processBMP(const string& bmpfile, const string& jpgname) {
    ifstream file(bmpfile, ios::binary);
    if (!file) {
        cout << "Can't open " << bmpfile << endl;
        return;
    }

    BMPHeader header;
    file.read((char*)&header, sizeof(BMPHeader));

    int width = header.width;
    int height = header.height;
    int bpp = header.bits_per_pixel;

    cout << bmpfile << " -> " << width << "x" << height
         << ", " << bpp << " bpp" << endl;

    if (bpp == 8) {
        file.seekg(header.offset - 1024, ios::beg);
    } else {
        file.seekg(header.offset, ios::beg);
    }

    int row_padded = (width * (bpp/8) + 3) & (~3);
    vector<unsigned char> data(row_padded * height);
    file.read((char*)data.data(), data.size());
    file.close();

    vector<unsigned char> gray(width * height);

    if (bpp == 24) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int index = i * row_padded + j * 3;
                unsigned char B = data[index];
                unsigned char G = data[index+1];
                unsigned char R = data[index+2];
                gray[(height-1-i) * width + j] = (R + G + B) / 3;
            }
        }
    } else if (bpp == 8) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int index = i * row_padded + j;
                gray[(height-1-i) * width + j] = data[index];
            }
        }
    } else {
        cout << "Unsupported BMP format: " << bpp << " bpp" << endl;
        return;
    }

    
    fs::create_directories("output");

    // 存成 JPG，輸出到 output 資料夾
    string outpath = "output/" + jpgname;
    stbi_write_jpg(outpath.c_str(), width, height, 1, gray.data(), 95);

    // 輸出中心 10×10
    cout << "Center 10x10 pixels of " << bmpfile << ":" << endl;
    for (int i = height/2 - 5; i < height/2 + 5; i++) {
        for (int j = width/2 - 5; j < width/2 + 5; j++) {
            cout << (int)gray[i * width + j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}


// ------------------ 主程式 ------------------
int main() {
    // RAW → PNG
    processRaw("data/lena.raw", "output/lena.png");
    processRaw("data/goldhill.raw", "output/goldhill.png");
    processRaw("data/peppers.raw", "output/peppers.png");

    // BMP → JPG
    processBMP("data/baboon.bmp", "output/baboon.jpg");
    processBMP("data/boat.bmp", "output/boat.jpg");
    processBMP("data/F16.bmp", "output/F16.jpg");

    return 0;
}
