#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct GLObjects
{
	GLuint VAO, VBO, program;
};

struct Pixel
{
	float x, y, r, g, b, a, size;
};

#define PADDING 8
#define PIXEL_SIZE 1

GLFWwindow* createWindow(int w, int h, const char* title);
int initGLEW();
void setUniform3f(GLuint program, const char* name, float x, float y, float z);
void updatePositions(int fbW, int fbH);

int createObjects(struct GLObjects* out, const struct Pixel* pixelObjects, size_t totalCount);
void destroyObjects(struct GLObjects* glObjects);
void render(const struct GLObjects* glObjects, GLsizei totalCount);
