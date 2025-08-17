#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "./include/renderer.h"
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

	if (size >= 29 && data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47 && data[4] == 0x0D &&
		data[5] == 0x0A && data[6] == 0x1A && data[7] == 0x0A)
	{
		const unsigned char bitDepth = data[24], colorType = data[25], interlace = data[28];
		int hasTRNS = 0, hasPLTE = 0;

		if (interlace == 1) return IMAGE_TYPE_PNG_ADAM7;
		if (bitDepth == 16) return IMAGE_TYPE_PNG_16BIT;
		if (colorType == 0 && (bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8))
			return IMAGE_TYPE_PNG_GRAYSCALE;

		size_t off = 8;
		while (off + 8 <= size)
		{
			const unsigned int len = (unsigned int)data[off + 0] << 24 | (unsigned int)data[off + 1] << 16 | (unsigned
				int)data[off + 2] << 8 | (unsigned int)data[off + 3];
			if (off + 8u + len + 4u > size) break;

			const unsigned char* type = &data[off + 4];
			if (type[0] == 't' && type[1] == 'R' && type[2] == 'N' && type[3] == 'S') hasTRNS = 1;
			if (type[0] == 'P' && type[1] == 'L' && type[2] == 'T' && type[3] == 'E') hasPLTE = 1;

			off += 8u + len + 4u;
			if (type[0] == 'I' && type[1] == 'E' && type[2] == 'N' && type[3] == 'D') break;
		}

		if (colorType == 3 && bitDepth <= 8 && hasTRNS) return IMAGE_TYPE_PNG_TRNS;
		if (colorType == 3 && bitDepth <= 8 && hasPLTE) return IMAGE_TYPE_PNG_PLTE;
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

	if (size >= 18 && (data[1] == 0x00 || data[1] == 0x01))
	{
		const unsigned char imageType = data[2];
		const unsigned char pixelDepth = data[16];

		if (imageType == 10) return IMAGE_TYPE_TGA_RLE;
		if (imageType == 2)
		{
			if (pixelDepth == 24) return IMAGE_TYPE_TGA_24;
			if (pixelDepth == 32) return IMAGE_TYPE_TGA_32;
		}
	}

	return IMAGE_TYPE_UNKNOWN;
}

/*
struct Pixel p;
p.x = ((px + 0.5) / (WIDTH + PADDING)) * 2 - 1;
p.y = 1 - ((py + 0.5) / (HEIGHT + PADDING)) * 2;

// For 8-bit, divide by 255
// For 16-bit, divide by 65535
p.r = r8 / 255;
p.g = g8 / 255;
p.b = b8 / 255;
p.a = a8 / 255;
p.size = 1;
*/

static void skipWhitespace(const unsigned char** p, const unsigned char* end)
{
	const unsigned char* copy = *p;
	while (1)
	{
		while (copy < end && (*copy == ' ' || *copy == '\t' || *copy == '\n' || *copy == '\r')) copy++;
		if (copy < end && *copy == '#')
		{
			while (copy < end && *copy != '\n') copy++;
			continue;
		}

		break;
	}

	*p = copy;
}

static int readUint(const unsigned char** p, const unsigned char* end, int* out)
{
	skipWhitespace(p, end);
	if (*p >= end || **p < '0' || **p > '9') return 0;

	long v = 0;
	while (*p < end && **p >= '0' && **p <= '9')
	{
		v = v * 10 + (**p - '0');
		(*p)++;

		if (v > 0x7FFFFFFF) return 0;
	}

	*out = (int)v;
	return 1;
}

static int readPPMHeader(const unsigned char* data, const size_t size, const char* magic, int* width, int* height,
                         int* maxVal, const unsigned char** outP, const unsigned char** outEnd)
{
	if (!data || size < 2) return 0;
	const unsigned char *p = data, *end = data + size;

	skipWhitespace(&p, end);
	if (p + 2 > end || p[0] != (const unsigned char)magic[0] || p[1] != (const unsigned char)magic[1])
	{
		fprintf(stderr, "Invalid header: expected '%s'\n", magic);
		return 0;
	}
	p += 2;

	if (!readUint(&p, end, width))
	{
		fprintf(stderr, "Invalid header: expected width\n");
		return 0;
	}
	if (!readUint(&p, end, height))
	{
		fprintf(stderr, "Invalid header: expected height\n");
		return 0;
	}
	if (!readUint(&p, end, maxVal))
	{
		fprintf(stderr, "Invalid header: expected max value\n");
		return 0;
	}

	if (*width <= 0 || *height <= 0)
	{
		fprintf(stderr, "Invalid image dimensions: %d x %d\n", *width, *height);
		return 0;
	}
	if (*maxVal <= 0 || *maxVal > 65535)
	{
		fprintf(stderr, "Invalid header: max value must be between 1 and 65535, got %d\n", *maxVal);
		return 0;
	}

	skipWhitespace(&p, end);
	*outP = p;
	*outEnd = end;

	return 1;
}

struct Pixel* parsePPM_P3(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	const unsigned char *p, *end;
	int maxVal;

	if (!count || !width || !height) return NULL;
	if (!readPPMHeader(data, size, "P3", width, height, &maxVal, &p, &end)) return NULL;

	*count = (size_t)*width * (size_t)*height;
	if (*count > SIZE_MAX / sizeof(struct Pixel))
	{
		fprintf(stderr, "Image too large: %dx%d exceeds maximum pixel count\n", *width, *height);
		return NULL;
	}

	struct Pixel* pixels = malloc(*count * sizeof(struct Pixel));
	if (!pixels)
	{
		fprintf(stderr, "Failed to allocate memory for %zu pixels\n", *count);
		return NULL;
	}

	const float invMax = 1.0f / (float)maxVal;
	for (int y = 0; y < *height; ++y)
		for (int x = 0; x < *width; ++x)
		{
			int r, g, b;
			if (!readUint(&p, end, &r) || !readUint(&p, end, &g) || !readUint(&p, end, &b))
			{
				fprintf(stderr, "Invalid pixel data at (%d, %d)\n", x, y);
				free(pixels);

				return NULL;
			}

			if (r < 0 || g < 0 || b < 0 || r > maxVal || g > maxVal || b > maxVal)
			{
				fprintf(stderr, "Invalid pixel value at (%d, %d): r=%d, g=%d, b=%d (max=%d)\n", x, y, r, g, b, maxVal);
				free(pixels);

				return NULL;
			}

			struct Pixel* px = &pixels[(size_t)y * (size_t)*width + (size_t)x];
			px->x = (float)x;
			px->y = (float)y;
			px->r = (float)r * invMax;
			px->g = (float)g * invMax;
			px->b = (float)b * invMax;
			px->a = 1.0f;
			px->size = PIXEL_SIZE;
		}

	return pixels;
}

struct Pixel* parsePPM_P6(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	const unsigned char *p, *end;
	int maxVal;

	if (!count || !width || !height) return NULL;
	if (!readPPMHeader(data, size, "P6", width, height, &maxVal, &p, &end)) return NULL;

	*count = (size_t)*width * (size_t)*height;
	if (*count > SIZE_MAX / sizeof(struct Pixel))
	{
		fprintf(stderr, "Image too large: %dx%d exceeds maximum pixel count\n", *width, *height);
		return NULL;
	}

	struct Pixel* pixels = malloc(*count * sizeof(struct Pixel));
	if (!pixels)
	{
		fprintf(stderr, "Failed to allocate memory for %zu pixels\n", *count);
		return NULL;
	}

	const float invMax = 1.0f / (float)maxVal;
	const int bytesPerSample = maxVal > 255 ? 2 : 1;

	for (int y = 0; y < *height; ++y)
		for (int x = 0; x < *width; ++x)
		{
			if ((size_t)(end - p) < (size_t)(3 * bytesPerSample))
			{
				fprintf(stderr, "Unexpected end of data at (%d, %d)\n", x, y);
				free(pixels);

				return NULL;
			}

			int r, g, b;
			if (bytesPerSample == 1)
			{
				r = p[0];
				g = p[1];
				b = p[2];
				p += 3;
			}
			else
			{
				r = p[0] << 8 | p[1];
				g = p[2] << 8 | p[3];
				b = p[4] << 8 | p[5];
				p += 6;
			}

			struct Pixel* px = &pixels[(size_t)y * (size_t)*width + (size_t)x];
			px->x = (float)x;
			px->y = (float)y;
			px->r = (float)r * invMax;
			px->g = (float)g * invMax;
			px->b = (float)b * invMax;
			px->a = 1.0f;
			px->size = PIXEL_SIZE;
		}

	return pixels;
}

struct Pixel* parsePGM_P5(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	const unsigned char *p, *end;
	int maxVal;

	if (!count || !width || !height) return NULL;
	if (!readPPMHeader(data, size, "P5", width, height, &maxVal, &p, &end)) return NULL;

	*count = (size_t)*width * (size_t)*height;
	if (*count > SIZE_MAX / sizeof(struct Pixel))
	{
		fprintf(stderr, "Image too large: %dx%d exceeds maximum pixel count\n", *width, *height);
		return NULL;
	}

	struct Pixel* pixels = malloc(*count * sizeof(struct Pixel));
	if (!pixels)
	{
		fprintf(stderr, "Failed to allocate memory for %zu pixels\n", *count);
		return NULL;
	}

	const int bytesPerSample = maxVal > 255 ? 2 : 1;
	const float invMax = 1.0f / (float)maxVal;

	for (int y = 0; y < *height; ++y)
		for (int x = 0; x < *width; ++x)
		{
			if ((size_t)(end - p) < (size_t)bytesPerSample)
			{
				fprintf(stderr, "Unexpected end of data at (%d, %d)\n", x, y);
				free(pixels);

				return NULL;
			}

			int value;
			if (bytesPerSample == 1) value = *p++;
			else
			{
				value = p[0] << 8 | p[1];
				p += 2;
			}

			if (value < 0 || value > maxVal)
			{
				fprintf(stderr, "Invalid pixel value at (%d, %d): %d (max=%d)\n", x, y, value, maxVal);
				free(pixels);

				return NULL;
			}

			struct Pixel* px = &pixels[(size_t)y * (size_t)*width + (size_t)x];
			const float g = (float)value * invMax;

			px->x = (float)x;
			px->y = (float)y;
			px->r = g;
			px->g = g;
			px->b = g;
			px->a = 1.0f;
			px->size = PIXEL_SIZE;
		}

	return pixels;
}

struct Pixel* parsePBM_P4(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseBMP_24(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseBMP_32(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseTGA_24(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseTGA_32(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseTGA_RLE(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parsePNG_8bit(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parsePNG_TRNS(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parsePNG_PLTE(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parsePNG_Grayscale(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parsePNG_16bit(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parsePNG_ADAM7(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseTIFF_Baseline(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}

struct Pixel* parseJPEG_Baseline(const unsigned char* data, const size_t size, size_t* count, int* width, int* height)
{
	fprintf(stderr, "Not implemented yet!\n");
	return NULL;
}
