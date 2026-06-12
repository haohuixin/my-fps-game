#pragma once

#include <cstddef>
#include <cstdint>

using GLenum = unsigned int;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
using GLvoid = void;
using GLbyte = std::int8_t;
using GLshort = short;
using GLint = int;
using GLsizei = int;
using GLubyte = std::uint8_t;
using GLushort = unsigned short;
using GLuint = unsigned int;
using GLfloat = float;
using GLclampf = float;
using GLdouble = double;

#ifndef APIENTRY
#define APIENTRY
#endif

constexpr GLenum GL_FALSE = 0;
constexpr GLenum GL_TRUE = 1;
constexpr GLenum GL_DEPTH_TEST = 0x0B71;
constexpr GLenum GL_BLEND = 0x0BE2;
constexpr GLenum GL_SRC_ALPHA = 0x0302;
constexpr GLenum GL_ONE_MINUS_SRC_ALPHA = 0x0303;
constexpr GLenum GL_ONE = 1;
constexpr GLenum GL_COLOR_BUFFER_BIT = 0x00004000;
constexpr GLenum GL_DEPTH_BUFFER_BIT = 0x00000100;
constexpr GLenum GL_TEXTURE_2D = 0x0DE1;
constexpr GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
constexpr GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
constexpr GLenum GL_TEXTURE_WRAP_S = 0x2802;
constexpr GLenum GL_TEXTURE_WRAP_T = 0x2803;
constexpr GLenum GL_NEAREST = 0x2600;
constexpr GLenum GL_CLAMP_TO_EDGE = 0x812F;
constexpr GLenum GL_RGBA8 = 0x8058;
constexpr GLenum GL_RGBA = 0x1908;
constexpr GLenum GL_UNSIGNED_BYTE = 0x1401;
constexpr GLenum GL_PROJECTION = 0x1701;
constexpr GLenum GL_MODELVIEW = 0x1700;
constexpr GLenum GL_QUADS = 0x0007;
constexpr GLenum GL_LINES = 0x0001;

extern "C" {
void APIENTRY glEnable(GLenum cap);
void APIENTRY glDisable(GLenum cap);
void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor);
void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY glClear(GLbitfield mask);
void APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures);
void APIENTRY glGenTextures(GLsizei n, GLuint* textures);
void APIENTRY glBindTexture(GLenum target, GLuint texture);
void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param);
void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                           GLint border, GLenum format, GLenum type, const void* pixels);
void APIENTRY glMatrixMode(GLenum mode);
void APIENTRY glLoadMatrixf(const GLfloat* m);
void APIENTRY glLoadIdentity(void);
void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal,
                      GLdouble farVal);
void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue);
void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void APIENTRY glLineWidth(GLfloat width);
void APIENTRY glBegin(GLenum mode);
void APIENTRY glEnd(void);
void APIENTRY glVertex2f(GLfloat x, GLfloat y);
void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glTexCoord2f(GLfloat s, GLfloat t);
}
