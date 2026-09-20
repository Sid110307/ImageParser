# ImageParser

ImageParser is a from-scratch image-format parser and OpenGL renderer written in C.

## Supported formats

- [x] PPM P3
- [x] PPM P6
- [x] PGM P5
- [x] PBM P4
- [ ] BMP 24-bit and 32-bit, uncompressed
- [ ] TGA 24-bit, 32-bit, and RLE
- [ ] PNG color, transparency, palette, grayscale, 16-bit, and Adam7 variants
- [ ] Baseline TIFF
- [ ] Baseline JPEG

## Build

### Requirements

- CMake 3.20 or newer
- A C11 compiler
- OpenGL
- GLFW
- GLEW

```sh
cmake -S . -B build
cmake --build build
./build/imageParser
```
