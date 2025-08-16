#include <stdio.h>
#include <stdlib.h>

#include "include/renderer.h"

static const char* vertexSource = "#version 330 core\n"
	"layout(location = 0) in vec2 aPos;\n"
	"layout(location = 1) in vec4 aColor;\n"
	"layout(location = 2) in float aSize;\n"
	"out vec4 vColor;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
	"    vColor = aColor;\n"
	"    gl_PointSize = aSize;\n"
	"}\n";

static const char* fragmentSource = "#version 330 core\n"
	"in vec4 vColor;\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"    FragColor = vColor;\n"
	"}\n";

// ReSharper disable once CppParameterMayBeConstPtrOrRef
static void fbSizeCallback(GLFWwindow* window, const int width, const int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

GLFWwindow* createWindow(const int w, const int h, const char* title)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();

		return NULL;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, fbSizeCallback);
	glfwSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

int initGLEW()
{
	glewExperimental = GL_TRUE;
	const GLenum err = glewInit();
	glGetError();

	if (err != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		return 0;
	}
	return 1;
}

static GLuint compileShader(const GLenum type, const char* src)
{
	const GLuint id = glCreateShader(type);
	glShaderSource(id, 1, &src, NULL);
	glCompileShader(id);

	GLint ok = GL_FALSE;
	glGetShaderiv(id, GL_COMPILE_STATUS, &ok);

	if (!ok)
	{
		GLint len = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);

		char* log = len > 0 ? (char*)malloc((size_t)len) : NULL;
		if (log)
		{
			glGetShaderInfoLog(id, len, NULL, log);
			fprintf(stderr, "Shader compile error (%s):\n%s\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			        log);
			free(log);
		}
		else fprintf(stderr, "Shader compile error (%s): <no log>\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment");

		glDeleteShader(id);
		return 0;
	}
	return id;
}

static GLuint linkProgram(const GLuint vs, const GLuint fs)
{
	const GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	GLint ok = GL_FALSE;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);

	if (!ok)
	{
		GLint len = 0;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);

		char* log = len > 0 ? (char*)malloc((size_t)len) : NULL;
		if (log)
		{
			glGetProgramInfoLog(prog, len, NULL, log);
			fprintf(stderr, "Program link error:\n%s\n", log);
			free(log);
		}
		else fprintf(stderr, "Program link error: <no log>\n");

		glDeleteProgram(prog);
		return 0;
	}

	glDetachShader(prog, vs);
	glDetachShader(prog, fs);

	return prog;
}

int createObjects(struct GLObjects* out, const struct Pixel* pixels, const size_t count)
{
	const GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
	const GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (!vs || !fs) return 0;

	out->program = linkProgram(vs, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);
	if (!out->program) return 0;

	glGenVertexArrays(1, &out->VAO);
	glGenBuffers(1, &out->VBO);
	glBindVertexArray(out->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, out->VBO);

	const GLsizeiptr stride = 7 * sizeof(float);
	if (pixels && count > 0)
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(count * stride), pixels, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, stride, NULL, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (int)stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, (int)stride, (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, (int)stride, (void*)(6 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return 1;
}

void destroyObjects(struct GLObjects* gl)
{
	if (gl->VBO)
	{
		glDeleteBuffers(1, &gl->VBO);
		gl->VBO = 0;
	}
	if (gl->VAO)
	{
		glDeleteVertexArrays(1, &gl->VAO);
		gl->VAO = 0;
	}
	if (gl->program)
	{
		glDeleteProgram(gl->program);
		gl->program = 0;
	}
}

void render(const struct GLObjects* gl, const GLsizei count)
{
	if (!gl || count <= 0) return;

	glUseProgram(gl->program);
	glBindVertexArray(gl->VAO);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, count);

	glBindVertexArray(0);
	glUseProgram(0);
}

void setUniform3f(const GLuint program, const char* name, const float x, const float y, const float z)
{
	glUseProgram(program);
	const GLint loc = glGetUniformLocation(program, name);
	if (loc != -1)
		glUniform3f(loc, x, y, z);
	glUseProgram(0);
}
