#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "parser.h"

struct GLObjects
{
	GLuint VAO, VBO, program;
};

#define PADDING 8

GLFWwindow* createWindow(int w, int h, const char* title);
int initGLEW();
void setUniform3f(GLuint program, const char* name, float x, float y, float z);
void updatePositions(int fbW, int fbH);

int createObjects(struct GLObjects* out, const struct Pixel* pixelObjects, size_t totalCount);
void destroyObjects(struct GLObjects* glObjects);
void render(const struct GLObjects* glObjects, GLsizei totalCount);
