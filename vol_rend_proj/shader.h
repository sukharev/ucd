#include <GL/glew.h>
#include <sys/types.h>

#ifndef WIN32
    #include <unistd.h>
#endif

#ifdef WIN32
    #include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int readShaderSource(char *fileName, GLchar **vertexShader, GLchar **fragmentShader);

int installShaders(const GLchar *brickVertex, const GLchar *brickFragment, GLuint * brickProg);

GLint getUniLoc(GLuint program, const GLchar *name);
