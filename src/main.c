#include <stdio.h>
#include <stdlib.h>

#include "include/renderer.h"
#include "include/parser.h"

const char* readFile(const char* filename)
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
	return buffer;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
		return EXIT_FAILURE;
	}

	const char* content = readFile(argv[1]);
	if (!content)
	{
		fprintf(stderr, "Failed to read file: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	GLFWwindow* window = createWindow(800, 600, "ImageParser");
	if (!window)
	{
		free((void*)content);
		return EXIT_FAILURE;
	}

	if (!initGLEW())
	{
		free((void*)content);
		glfwTerminate();

		return EXIT_FAILURE;
	}

	struct GLObjects gl = {0};
	const struct Pixel pixels[] = {
		{-0.5f, -0.5f, 1.0f, 0.0f, 0.0f},
		{0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.5f, 0.0f, 0.0f, 1.0f},
		{0.0f, 0.0f, 1.0f, 1.0f, 1.0f}
	};
	const int pixelCount = sizeof(pixels) / sizeof(pixels[0]);

	if (!createObjects(&gl, pixels, pixelCount))
	{
		free((void*)content);
		destroyObjects(&gl);
		glfwTerminate();

		return EXIT_FAILURE;
	}

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glViewport(0, 0, w, h);
	glClearColor(0.08f, 0.09f, 0.12f, 1.0f);

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
		glClear(GL_COLOR_BUFFER_BIT);
		render(&gl, pixelCount);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	free((void*)content);
	destroyObjects(&gl);
	glfwTerminate();

	return EXIT_SUCCESS;
}
