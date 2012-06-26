/*
** シェーダのソースプログラムの読み込みに使う関数
*/
#ifndef SHADERSOURCE_H
#define SHADERSOURCE_H

#if defined(__APPLE__) || defined(MACOSX)
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

extern int readShaderSource(GLuint shader, const char *file);
extern void printShaderInfoLog(GLuint shader);
extern void printProgramInfoLog(GLuint program);

#endif
