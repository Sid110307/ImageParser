#include <stdio.h>
#include <stdlib.h>

#include "./include/parser.h"

struct Pixel* parseImage(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	if (!data || !size || !count || !width || !height) return NULL;

	const int type = getImageType(data, size);
	switch (type)
	{
		case IMAGE_TYPE_PPM_P3:
			return parsePPM_P3(data, size, count, width, height);
		case IMAGE_TYPE_PPM_P6:
			return parsePPM_P6(data, size, count, width, height);
		case IMAGE_TYPE_PGM_P5:
			return parsePGM_P5(data, size, count, width, height);
		case IMAGE_TYPE_PBM_P4:
			return parsePBM_P4(data, size, count, width, height);
		case IMAGE_TYPE_BMP_24:
			return parseBMP_24(data, size, count, width, height);
		case IMAGE_TYPE_BMP_32:
			return parseBMP_32(data, size, count, width, height);
		case IMAGE_TYPE_TGA_24:
			return parseTGA_24(data, size, count, width, height);
		case IMAGE_TYPE_TGA_32:
			return parseTGA_32(data, size, count, width, height);
		case IMAGE_TYPE_TGA_RLE:
			return parseTGA_RLE(data, size, count, width, height);
		case IMAGE_TYPE_PNG_8BIT:
			return parsePNG_8bit(data, size, count, width, height);
		case IMAGE_TYPE_PNG_TRNS:
			return parsePNG_TRNS(data, size, count, width, height);
		case IMAGE_TYPE_PNG_PLTE:
			return parsePNG_PLTE(data, size, count, width, height);
		case IMAGE_TYPE_PNG_GRAYSCALE:
			return parsePNG_Grayscale(data, size, count, width, height);
		case IMAGE_TYPE_PNG_16BIT:
			return parsePNG_16bit(data, size, count, width, height);
		case IMAGE_TYPE_PNG_ADAM7:
			return parsePNG_ADAM7(data, size, count, width, height);
		case IMAGE_TYPE_TIFF_BASELINE:
			return parseTIFF_Baseline(data, size, count, width, height);
		case IMAGE_TYPE_JPEG_BASELINE:
			return parseJPEG_Baseline(data, size, count, width, height);
		default:
			fprintf(stderr, "Unknown image type: %d\n", type);
			break;
	}

	*count = 0;
	return NULL;
}

int getImageType(const unsigned char* data, const size_t size)
{
	if (!data || size < 4) return IMAGE_TYPE_UNKNOWN;

	if (data[0] == 'P')
	{
		if (data[1] == '3') return IMAGE_TYPE_PPM_P3;
		if (data[1] == '6') return IMAGE_TYPE_PPM_P6;
		if (data[1] == '5') return IMAGE_TYPE_PGM_P5;
		if (data[1] == '4') return IMAGE_TYPE_PBM_P4;
	}

	if (size >= 34 && data[0] == 'B' && data[1] == 'M')
	{
		const unsigned int compression = (unsigned int)data[30] | (unsigned int)data[31] << 8 | (unsigned int)data[32]
			                   << 16 | (unsigned int)data[33] << 24,
		                   bpp = (unsigned int)data[28] | (unsigned int)data[29] << 8;
		if (compression == 0)
		{
			if (bpp == 24) return IMAGE_TYPE_BMP_24;
			if (bpp == 32) return IMAGE_TYPE_BMP_32;
		}

		fprintf(stderr, "Unsupported BMP compression or BPP: %u, %u\n", compression, bpp);
		return IMAGE_TYPE_UNKNOWN;
	}

	if (size >= 18)
	{
		const unsigned char colorMapType = data[1];
		const unsigned char imageType = data[2];
		const unsigned char pixelDepth = data[16];

		if (colorMapType != 0 && colorMapType != 1)
		{
			fprintf(stderr, "Unsupported TGA color map type: %u\n", colorMapType);
			return IMAGE_TYPE_UNKNOWN;
		}

		if (imageType == 10) return IMAGE_TYPE_TGA_RLE;
		if (imageType == 2)
		{
			if (pixelDepth == 24) return IMAGE_TYPE_TGA_24;
			if (pixelDepth == 32) return IMAGE_TYPE_TGA_32;
		}
	}

	if (size >= 29 && data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 && data[4] == 0x0D &&
		data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A)
	{
		const unsigned char bitDepth = data[24], colorType = data[25], interlace = data[28];

		if (interlace == 1) return IMAGE_TYPE_PNG_ADAM7;
		if (bitDepth == 16) return IMAGE_TYPE_PNG_16BIT;
		if (colorType == 0 && (bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8))
			return IMAGE_TYPE_PNG_GRAYSCALE;
		if (colorType == 3) return IMAGE_TYPE_PNG_PLTE;

		size_t off = 8;
		while (off + 8 <= size)
		{
			const unsigned int len = (unsigned int)data[off + 0] << 24 | (unsigned int)data[off + 1] << 16 | (unsigned
				int)data[off + 2] << 8 | (unsigned int)data[off + 3];
			if (off + 8u + len + 4u > size) break;

			const unsigned char* type = &data[off + 4];
			if (type[0] == 't' && type[1] == 'R' && type[2] == 'N' && type[3] == 'S') return IMAGE_TYPE_PNG_TRNS;
			if (type[0] == 'P' && type[1] == 'L' && type[2] == 'T' && type[3] == 'E') return IMAGE_TYPE_PNG_PLTE;

			off += 8u + len + 4u;
			if (type[0] == 'I' && type[1] == 'E' && type[2] == 'N' && type[3] == 'D') break;
		}
		if (bitDepth == 8 && interlace == 0) return IMAGE_TYPE_PNG_8BIT;

		fprintf(stderr, "Unsupported PNG image type with bit depth %u, color type %u, interlace %u\n",
		        bitDepth, colorType, interlace);
		return IMAGE_TYPE_UNKNOWN;
	}

	if ((data[0] == 'I' && data[1] == 'I' && data[2] == 0x2A && data[3] == 0x00) || (data[0] == 'M' && data[1] == 'M' &&
		data[2] == 0x00 && data[3] == 0x2A))
		return IMAGE_TYPE_TIFF_BASELINE;

	if (data[0] == 0xFF && data[1] == 0xD8)
	{
		size_t p = 2;
		while (p + 3 < size)
		{
			while (p < size && data[p] == 0xFF) p++;
			if (p >= size) break;

			const unsigned char marker = data[p++];
			if (marker == 0xD9 || marker == 0xDA) break;
			if (marker == 0x01 || (marker >= 0xD0 && marker <= 0xD7)) continue;

			if (p + 1 >= size) break;
			const unsigned int segmentLength = (unsigned int)(data[p] << 8 | data[p + 1]);
			if (segmentLength < 2 || p + segmentLength >= size) break;

			p += 2;
			if (marker == 0xC0) return IMAGE_TYPE_JPEG_BASELINE;
			p += segmentLength - 2;
		}

		fprintf(stderr, "Unsupported JPEG format or missing EOI marker\n");
		return IMAGE_TYPE_UNKNOWN;
	}

	return IMAGE_TYPE_UNKNOWN;
}

void freePixels(struct Pixel* pixels, const size_t count) { if (pixels && count > 0) free(pixels); }

struct Pixel* parsePPM_P3(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePPM_P6(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePGM_P5(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePBM_P4(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseBMP_24(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseBMP_32(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseTGA_24(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseTGA_32(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseTGA_RLE(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePNG_8bit(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePNG_TRNS(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePNG_PLTE(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePNG_Grayscale(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePNG_16bit(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parsePNG_ADAM7(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseTIFF_Baseline(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}

struct Pixel* parseJPEG_Baseline(const unsigned char* data, size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!");
	return NULL;
}
