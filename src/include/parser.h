#pragma once

enum ImageType
{
	IMAGE_TYPE_UNKNOWN = 0,
	IMAGE_TYPE_PPM_P3,
	IMAGE_TYPE_PPM_P6,
	IMAGE_TYPE_PGM_P5,
	IMAGE_TYPE_PBM_P4,
	IMAGE_TYPE_BMP_24,
	IMAGE_TYPE_BMP_32,
	IMAGE_TYPE_TGA_24,
	IMAGE_TYPE_TGA_32,
	IMAGE_TYPE_TGA_RLE,
	IMAGE_TYPE_PNG_8BIT,
	IMAGE_TYPE_PNG_TRNS,
	IMAGE_TYPE_PNG_PLTE,
	IMAGE_TYPE_PNG_GRAYSCALE,
	IMAGE_TYPE_PNG_16BIT,
	IMAGE_TYPE_PNG_ADAM7,
	IMAGE_TYPE_TIFF_BASELINE,
	IMAGE_TYPE_JPEG_BASELINE
};

struct Pixel* parseImage(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
int getImageType(const unsigned char* data, size_t size);

struct Pixel* parsePPM_P3(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePPM_P6(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePGM_P5(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePBM_P4(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseBMP_24(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseBMP_32(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseTGA_24(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseTGA_32(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseTGA_RLE(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePNG_8bit(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePNG_TRNS(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePNG_PLTE(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePNG_Grayscale(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePNG_16bit(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parsePNG_ADAM7(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseTIFF_Baseline(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
struct Pixel* parseJPEG_Baseline(const unsigned char* data, size_t size, size_t* count, int* width, int* height);
