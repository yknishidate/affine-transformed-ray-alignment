# Affine-Transformed Ray Alignment

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
