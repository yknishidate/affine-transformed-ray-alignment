# Affine-Transformed Ray Alignment

Yuki Nishidate and Issei Fujishiro. 2022. Affine-Transformed Ray Alignment for Fast Ray Traversal. In SIGGRAPH Asia 2022 Technical Communications (SA '22 Technical Communications). Association for Computing Machinery, New York, NY, USA, Article 13, 1–4. https://doi.org/10.1145/3550340.3564228

```
// 1. Clone
git clone https://github.com/yknishidate/affine-transformed-ray-alignment --recursive
cd affine-transformed-ray-alignment

// 2. Download files
// example: Windows PowerShell
// Invoke-WebRequest -Uri https://casual-effects.com/g3d/data10/research/model/mitsuba_knob/mitsuba.zip -OutFile .\asset\mitsuba.zip -UseBasicParsing
// Expand-Archive -Path .\asset\mitsuba.zip -DestinationPath .\asset\mitsuba

// 3. Run cmake
cmake . -B build

// 4. Build and Run
cmake --build build
cd build
.\Debug\affine_transformed_ray_alignment.exe
```
