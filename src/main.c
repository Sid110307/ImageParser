#include <stdio.h>
#include <stdlib.h>

#include "include/renderer.h"
#include "include/parser.h"

struct Pixel* pixels = NULL;
int imageWidth = 0, imageHeight = 0;
size_t count = 0;
struct GLObjects gl = {0};

char* readFile(const char* filename, size_t* outSize)
{
	FILE* file = fopen(filename, "rb");
	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	if (size < 0)
	{
		fclose(file);
		return NULL;
	}
	fseek(file, 0, SEEK_SET);

	char* buffer = malloc(size + 1);
	if (!buffer)
	{
		fclose(file);
		return NULL;
	}

	const size_t read = fread(buffer, 1, size, file);
	fclose(file);

	if (read != (size_t)size)
	{
		free(buffer);
		return NULL;
	}

	buffer[size] = '\0';
	if (outSize) *outSize = (size_t)size;

	return buffer;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
		return EXIT_FAILURE;
	}

	size_t contentSize = 0;
	char* content = readFile(argv[1], &contentSize);
	if (!content)
	{
		fprintf(stderr, "Failed to read file: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	pixels = parseImage((const unsigned char*)content, contentSize, &count, &imageWidth, &imageHeight);
	if (!pixels || count == 0)
	{
		fprintf(stderr, "Failed to parse image: %s\n", argv[1]);
		free(content);
		freePixels(pixels, count);

		return EXIT_FAILURE;
	}

	if (imageWidth <= 0 || imageHeight <= 0)
	{
		fprintf(stderr, "Invalid image dimensions: %dx%d\n", imageWidth, imageHeight);
		free(content);
		freePixels(pixels, count);

		return EXIT_FAILURE;
	}

	GLFWwindow* window = createWindow(imageWidth + PADDING * 2, imageHeight + PADDING * 2, "ImageParser");
	if (!window)
	{
		free(content);
		freePixels(pixels, count);

		return EXIT_FAILURE;
	}

	if (!initGLEW())
	{
		free(content);
		freePixels(pixels, count);
		glfwTerminate();

		return EXIT_FAILURE;
	}

	if (!createObjects(&gl, pixels, count))
	{
		free(content);
		freePixels(pixels, count);
		destroyObjects(&gl);
		glfwTerminate();

		return EXIT_FAILURE;
	}

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	updatePositions(w, h);
	glClearColor(0.08f, 0.09f, 0.12f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
		glClear(GL_COLOR_BUFFER_BIT);
		render(&gl, (int)count);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	free(content);
	freePixels(pixels, count);
	destroyObjects(&gl);
	glfwTerminate();

	return EXIT_SUCCESS;
}
