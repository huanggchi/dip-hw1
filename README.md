# dip-hw1
 Digital Image Processing Functions: Reading and Point Operation

# DIP HW1 – Image Reading, Enhancement, and Resampling

This repository (**dip-hw1**) contains three independent C++ programs developed for the **Multimodal Image Processing** course.  
Each program demonstrates fundamental digital image processing operations, including image reading, pixel-wise intensity transformations, and image interpolation.

---
## Folder Structure
dip-hw1

├─ src/ # C++ source code (hw1a.cpp, hw1b.cpp, hw1c.cpp)

├─ include/ # header files (e.g., stb_image_write.h)

├─ bin/ # compiled executables (.exe)

├─ data/ # input images (RAW and BMP)

└─ output/ # generated results (PNG/JPG from each task)

---

## Programs Overview
- **hw1a.exe**  
  Reads RAW and BMP images, converts them into PNG/JPG, and prints the centered 10×10 pixel values in the console.  

- **hw1b.exe**  
  Applies intensity transformations: logarithmic, gamma correction (γ = 0.5, γ = 2.0), and negative transformation.  

- **hw1c.exe**  
  Implements image downsampling and upsampling using nearest-neighbor and bilinear interpolation, allowing comparison between different resizing methods.  

---

## How to Run
> Run all commands **from the `dip-hw1/` root directory**, where `data/` and `output/` folders are located.

### A. Image Reading
```powershell
.\bin\hw1a.exe
### B. Image Enhancement
.\bin\hw1b.exe
### C. Image Down/Up Sampling
.\bin\hw1c.exe
