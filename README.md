# ImageParser

> Slow ass image parser and renderer with OpenGL.

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
cmake -S . -B bin
cmake --build bin
./bin/imageParser <path/to/image>
```

Or via `run.sh`, which does the same and also runs the result.
