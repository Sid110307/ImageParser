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

GLFWwindow* createWindow(int w, int h, const char* title);
int initGLEW();
void setUniform3f(GLuint program, const char* name, float x, float y, float z);

int createObjects(struct GLObjects* out, const struct Pixel* pixels, size_t count);
void destroyObjects(struct GLObjects* gl);
void render(const struct GLObjects* gl, GLsizei count);
