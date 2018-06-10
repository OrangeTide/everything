/* glload.h : single file loader for GL Core functions - public domain. */
/*
 * Written in 2018 by Jon Mayo <jon@rm-f.net>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */
#ifndef GLLOAD_H
#define GLLOAD_H
#include <GL/gl.h>
#include <GL/glext.h>
#if defined(_WIN32)
#  include <windows.h>
#  include <GL/wglext.h>
#  if NDEBUG
#    define load_proc(name) wglGetProcAddress(name);
#  else
#    define load_proc(name) ({ \
	void *_proc = wglGetProcAddress(name); \
	if (_proc) pr_dbg("loaded %s", name); \
	else die("failed to load GL extension"); \
	_proc; })
#  endif
/** WGL function pointers **/
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB __attribute__((unused));
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB __attribute__((unused));
#elif defined(__unix)
#  include <GL/glx.h>
#  define load_proc(name) (void*)glXGetProcAddress((const GLubyte*)(name))
#else
#  error Unsupported platform
#endif

/* generates dispatch stub for functions that don't return (void) */
#define GEN_GL_VOID_PROC(func, type, params, args) \
	type _gldispatch_ ## func; \
	static inline void func params __attribute__((unused)); \
	static inline void func params { \
		if (!_gldispatch_ ## func) \
			_gldispatch_ ## func = (type)load_proc(#func); \
		_gldispatch_ ## func args; \
	}
/* generates dispatch stub */
#define GEN_GL_PROC(func, type, ret, params, args) \
	type _gldispatch_ ## func; \
	static inline ret func params __attribute__((unused)); \
	static inline ret func params {	\
		if (!_gldispatch_ ## func) \
			_gldispatch_ ## func = (type)load_proc(#func); \
		return _gldispatch_ ## func args; \
	}
/* use this for old 1.1 APIs that opengl32.dll would already have */
#define GEN_GL_OLD_PROC(func, type, ret, params, args) /* nothing to do - gl 1.1 stuff is already linked in */

GEN_GL_OLD_PROC(glBlendFunc, PFNGLBLENDFUNCPROC, void, (GLenum sfactor, GLenum dfactor), (sfactor, dfactor));
GEN_GL_OLD_PROC(glClear, PFNGLCLEARPROC, void, (GLbitfield mask), (mask));
GEN_GL_OLD_PROC(glClearColor, PFNGLCLEARCOLORPROC, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha), (red, geren, blue, alpha));
GEN_GL_OLD_PROC(glClearDepth, PFNGLCLEARDEPTHPROC, void, (GLdouble depth), (depth));
GEN_GL_OLD_PROC(glClearStencil, PFNGLCLEARSTENCILPROC, void, (GLint s), (s));
GEN_GL_OLD_PROC(glColorMask, PFNGLCOLORMASKPROC, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha), (red, green, blue, alpha));
GEN_GL_OLD_PROC(glCullFace, PFNGLCULLFACEPROC, void, (GLenum mode), (mode));
GEN_GL_OLD_PROC(glDepthFunc, PFNGLDEPTHFUNCPROC, void, (GLenum func), (func));
GEN_GL_OLD_PROC(glDepthMask, PFNGLDEPTHMASKPROC, void, (GLboolean flag), (flag));
GEN_GL_OLD_PROC(glDepthRange, PFNGLDEPTHRANGEPROC, void, (GLdouble ren_near, GLdouble ren_far), (ren_near, ren_far));
GEN_GL_OLD_PROC(glDisable, PFNGLDISABLEPROC, void, (GLenum cap), (cap));
GEN_GL_OLD_PROC(glDrawBuffer, PFNGLDRAWBUFFERPROC, void, (GLenum buf), (buf));
GEN_GL_OLD_PROC(glEnable, PFNGLENABLEPROC, void, (GLenum cap), (cap));
GEN_GL_OLD_PROC(glFinish, PFNGLFINISHPROC, void, (void), ());
GEN_GL_OLD_PROC(glFlush, PFNGLFLUSHPROC, void, (void), ());
GEN_GL_OLD_PROC(glFrontFace, PFNGLFRONTFACEPROC, void, (GLenum mode), (mode));
GEN_GL_OLD_PROC(glGetBooleanv, PFNGLGETBOOLEANVPROC, void, (GLenum pname, GLboolean * data), (pname, data));
GEN_GL_OLD_PROC(glGetDoublev, PFNGLGETDOUBLEVPROC, void, (GLenum pname, GLdouble * data), (pname, data));
GEN_GL_OLD_PROC(glGetError, PFNGLGETERRORPROC, GLenum, (void), ());
GEN_GL_OLD_PROC(glGetFloatv, PFNGLGETFLOATVPROC, void, (GLenum pname, GLfloat * data), (pname, data));
GEN_GL_OLD_PROC(glGetIntegerv, PFNGLGETINTEGERVPROC, void, (GLenum pname, GLint * data), (pname, data));
GEN_GL_OLD_PROC(glGetString, PFNGLGETSTRINGPROC, const GLubyte *, (GLenum name), (name));
GEN_GL_OLD_PROC(glGetTexImage, PFNGLGETTEXIMAGEPROC, void, (GLenum target, GLint level, GLenum format, GLenum type, void * pixels), (target, level, format, type, pixels));
GEN_GL_OLD_PROC(glGetTexLevelParameterfv, PFNGLGETTEXLEVELPARAMETERFVPROC, void, (GLenum target, GLint level, GLenum pname, GLfloat * params), (target, level, pname, params));
GEN_GL_OLD_PROC(glGetTexLevelParameteriv, PFNGLGETTEXLEVELPARAMETERIVPROC, void, (GLenum target, GLint level, GLenum pname, GLint * params), (target, level, pname, params));
GEN_GL_OLD_PROC(glGetTexParameterfv, PFNGLGETTEXPARAMETERFVPROC, void, (GLenum target, GLenum pname, GLfloat * params), (target, pname, params));
GEN_GL_OLD_PROC(glGetTexParameteriv, PFNGLGETTEXPARAMETERIVPROC, void, (GLenum target, GLenum pname, GLint * params), (target, pname, params));
GEN_GL_OLD_PROC(glHint, PFNGLHINTPROC, void, (GLenum target, GLenum mode), (target, mode));
GEN_GL_OLD_PROC(glIsEnabled, PFNGLISENABLEDPROC, GLboolean, (GLenum cap), (cap));
GEN_GL_OLD_PROC(glLineWidth, PFNGLLINEWIDTHPROC, void, (GLfloat width), (width));
GEN_GL_OLD_PROC(glLogicOp, PFNGLLOGICOPPROC, void, (GLenum opcode), (opcode));
GEN_GL_OLD_PROC(glPixelStoref, PFNGLPIXELSTOREFPROC, void, (GLenum pname, GLfloat param), (pname, param));
GEN_GL_OLD_PROC(glPixelStorei, PFNGLPIXELSTOREIPROC, void, (GLenum pname, GLint param), (pname, param));
GEN_GL_OLD_PROC(glPointSize, PFNGLPOINTSIZEPROC, void, (GLfloat size), (size));
GEN_GL_OLD_PROC(glPolygonMode, PFNGLPOLYGONMODEPROC, void, (GLenum face, GLenum mode), (mode));
GEN_GL_OLD_PROC(glReadBuffer, PFNGLREADBUFFERPROC, void, (GLenum src), (src));
GEN_GL_OLD_PROC(glReadPixels, PFNGLREADPIXELSPROC, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels), (x, y, width, height, format, type, pixels));
GEN_GL_OLD_PROC(glScissor, PFNGLSCISSORPROC, void, (GLint x, GLint y, GLsizei width, GLsizei height), (x, y, width, height));
GEN_GL_OLD_PROC(glStencilFunc, PFNGLSTENCILFUNCPROC, void, (GLenum func, GLint ref, GLuint mask), (func, ref, mask));
GEN_GL_OLD_PROC(glStencilMask, PFNGLSTENCILMASKPROC, void, (GLuint mask), (mask));
GEN_GL_OLD_PROC(glStencilOp, PFNGLSTENCILOPPROC, void, (GLenum fail, GLenum zfail, GLenum zpass), (fail, zfail, zpass));
GEN_GL_OLD_PROC(glTexImage1D, PFNGLTEXIMAGE1DPROC, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void * pixels), (target, level, internalformat, width, border, format, type, pixels));
GEN_GL_OLD_PROC(glTexImage2D, PFNGLTEXIMAGE2DPROC, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels), (target, level, internalformat, width, height, border, format, type, pixels));
GEN_GL_OLD_PROC(glTexParameterf, PFNGLTEXPARAMETERFPROC, void, (GLenum target, GLenum pname, GLfloat param), (target, pname, param));
GEN_GL_OLD_PROC(glTexParameterfv, PFNGLTEXPARAMETERFVPROC, void, (GLenum target, GLenum pname, const GLfloat * params), (target, pname, params));
GEN_GL_OLD_PROC(glTexParameteri, PFNGLTEXPARAMETERIPROC, void, (GLenum target, GLenum pname, GLint param), (target, pname, params));
GEN_GL_OLD_PROC(glTexParameteriv, PFNGLTEXPARAMETERIVPROC, void, (GLenum target, GLenum pname, const GLint * params), (target, pname, params));
#if 0 // TODO
GEN_GL_PROC(glViewport, PFNGLVIEWPORTPROC, void, (GLint x, GLint y, GLsizei width, GLsizei height));
GEN_GL_PROC(glBindTexture, PFNGLBINDTEXTUREPROC, void, (GLenum target, GLuint texture));
GEN_GL_PROC(glCopyTexImage1D, PFNGLCOPYTEXIMAGE1DPROC, void, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border));
GEN_GL_PROC(glCopyTexImage2D, PFNGLCOPYTEXIMAGE2DPROC, void, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border));
GEN_GL_PROC(glCopyTexSubImage1D, PFNGLCOPYTEXSUBIMAGE1DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width));
GEN_GL_PROC(glCopyTexSubImage2D, PFNGLCOPYTEXSUBIMAGE2DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height));
GEN_GL_PROC(glDeleteTextures, PFNGLDELETETEXTURESPROC, void, (GLsizei n, const GLuint * textures));
GEN_GL_PROC(glDrawArrays, PFNGLDRAWARRAYSPROC, void, (GLenum mode, GLint first, GLsizei count));
GEN_GL_PROC(glDrawElements, PFNGLDRAWELEMENTSPROC, void, (GLenum mode, GLsizei count, GLenum type, const void * indices));
GEN_GL_PROC(glGenTextures, PFNGLGENTEXTURESPROC, void, (GLsizei n, GLuint * textures));
GEN_GL_PROC(glIsTexture, PFNGLISTEXTUREPROC, GLboolean, (GLuint texture));
GEN_GL_PROC(glPolygonOffset, PFNGLPOLYGONOFFSETPROC, void, (GLfloat factor, GLfloat units));
GEN_GL_PROC(glTexSubImage1D, PFNGLTEXSUBIMAGE1DPROC, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels));
GEN_GL_PROC(glTexSubImage2D, PFNGLTEXSUBIMAGE2DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels));
GEN_GL_PROC(glCopyTexSubImage3D, PFNGLCOPYTEXSUBIMAGE3DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height));
GEN_GL_PROC(glDrawRangeElements, PFNGLDRAWRANGEELEMENTSPROC, void, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices));
GEN_GL_PROC(glTexImage3D, PFNGLTEXIMAGE3DPROC, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels));
GEN_GL_PROC(glTexSubImage3D, PFNGLTEXSUBIMAGE3DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels));
GEN_GL_PROC(glActiveTexture, PFNGLACTIVETEXTUREPROC, void, (GLenum texture));
GEN_GL_PROC(glCompressedTexImage1D, PFNGLCOMPRESSEDTEXIMAGE1DPROC, void, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void * data));
GEN_GL_PROC(glCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC, void, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data));
GEN_GL_PROC(glCompressedTexImage3D, PFNGLCOMPRESSEDTEXIMAGE3DPROC, void, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data));
GEN_GL_PROC(glCompressedTexSubImage1D, PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data));
GEN_GL_PROC(glCompressedTexSubImage2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data));
GEN_GL_PROC(glCompressedTexSubImage3D, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data));
GEN_GL_PROC(glGetCompressedTexImage, PFNGLGETCOMPRESSEDTEXIMAGEPROC, void, (GLenum target, GLint level, void * img));
GEN_GL_PROC(glSampleCoverage, PFNGLSAMPLECOVERAGEPROC, void, (GLfloat value, GLboolean invert));
GEN_GL_PROC(glBlendColor, PFNGLBLENDCOLORPROC, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha));
GEN_GL_PROC(glBlendEquation, PFNGLBLENDEQUATIONPROC, void, (GLenum mode));
GEN_GL_PROC(glBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC, void, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha));
GEN_GL_PROC(glMultiDrawArrays, PFNGLMULTIDRAWARRAYSPROC, void, (GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount));
GEN_GL_PROC(glMultiDrawElements, PFNGLMULTIDRAWELEMENTSPROC, void, (GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount));
GEN_GL_PROC(glPointParameterf, PFNGLPOINTPARAMETERFPROC, void, (GLenum pname, GLfloat param));
GEN_GL_PROC(glPointParameterfv, PFNGLPOINTPARAMETERFVPROC, void, (GLenum pname, const GLfloat * params));
GEN_GL_PROC(glPointParameteri, PFNGLPOINTPARAMETERIPROC, void, (GLenum pname, GLint param));
GEN_GL_PROC(glPointParameteriv, PFNGLPOINTPARAMETERIVPROC, void, (GLenum pname, const GLint * params));
GEN_GL_PROC(glBeginQuery, PFNGLBEGINQUERYPROC, void, (GLenum target, GLuint id));
GEN_GL_PROC(glBindBuffer, PFNGLBINDBUFFERPROC, void, (GLenum target, GLuint buffer));
GEN_GL_PROC(glBufferData, PFNGLBUFFERDATAPROC, void, (GLenum target, GLsizeiptr size, const void * data, GLenum usage));
GEN_GL_PROC(glBufferSubData, PFNGLBUFFERSUBDATAPROC, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void * data));
GEN_GL_PROC(glDeleteBuffers, PFNGLDELETEBUFFERSPROC, void, (GLsizei n, const GLuint * buffers));
GEN_GL_PROC(glDeleteQueries, PFNGLDELETEQUERIESPROC, void, (GLsizei n, const GLuint * ids));
GEN_GL_PROC(glEndQuery, PFNGLENDQUERYPROC, void, (GLenum target));
GEN_GL_PROC(glGenBuffers, PFNGLGENBUFFERSPROC, void, (GLsizei n, GLuint * buffers));
GEN_GL_PROC(glGenQueries, PFNGLGENQUERIESPROC, void, (GLsizei n, GLuint * ids));
GEN_GL_PROC(glGetBufferParameteriv, PFNGLGETBUFFERPARAMETERIVPROC, void, (GLenum target, GLenum pname, GLint * params));
GEN_GL_PROC(glGetBufferPointerv, PFNGLGETBUFFERPOINTERVPROC, void, (GLenum target, GLenum pname, void ** params));
GEN_GL_PROC(glGetBufferSubData, PFNGLGETBUFFERSUBDATAPROC, void, (GLenum target, GLintptr offset, GLsizeiptr size, void * data));
GEN_GL_PROC(glGetQueryObjectiv, PFNGLGETQUERYOBJECTIVPROC, void, (GLuint id, GLenum pname, GLint * params));
GEN_GL_PROC(glGetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC, void, (GLuint id, GLenum pname, GLuint * params));
GEN_GL_PROC(glGetQueryiv, PFNGLGETQUERYIVPROC, void, (GLenum target, GLenum pname, GLint * params));
GEN_GL_PROC(glIsBuffer, PFNGLISBUFFERPROC, GLboolean, (GLuint buffer));
GEN_GL_PROC(glIsQuery, PFNGLISQUERYPROC, GLboolean, (GLuint id));
extern void * (CODEGEN_FUNCPTR *_ptrc_glMapBuffer)(GLenum target, GLenum access);
GEN_GL_PROC(glUnmapBuffer, PFNGLUNMAPBUFFERPROC, GLboolean, (GLenum target));
#endif // TODO
GEN_GL_VOID_PROC(glAttachShader, PFNGLATTACHSHADERPROC, (GLuint program, GLuint shader), (program, shader));
GEN_GL_VOID_PROC(glBindAttribLocation, PFNGLBINDATTRIBLOCATIONPROC, (GLuint program, GLuint index, const GLchar * name), (program, index, name));
GEN_GL_VOID_PROC(glBlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC, (GLenum modeRGB, GLenum modeAlpha), (modeRGB, modeAlpha));
GEN_GL_VOID_PROC(glCompileShader, PFNGLCOMPILESHADERPROC, (GLuint shader), (shader));
GEN_GL_PROC(glCreateProgram, PFNGLCREATEPROGRAMPROC, GLuint, (void), ());
GEN_GL_PROC(glCreateShader, PFNGLCREATESHADERPROC, GLuint, (GLenum type), (type));
GEN_GL_VOID_PROC(glDeleteProgram, PFNGLDELETEPROGRAMPROC, (GLuint program), (program));
GEN_GL_VOID_PROC(glDeleteShader, PFNGLDELETESHADERPROC, (GLuint shader), (shader));
GEN_GL_VOID_PROC(glDetachShader, PFNGLDETACHSHADERPROC, (GLuint program, GLuint shader), (program, shader));
GEN_GL_VOID_PROC(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC, (GLuint index), (index));
GEN_GL_VOID_PROC(glDrawBuffers, PFNGLDRAWBUFFERSPROC, (GLsizei n, const GLenum * bufs), (n, bufs));
GEN_GL_VOID_PROC(glEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC, (GLuint index), (index));
#if 0 // TODO
GEN_GL_PROC(glGetActiveAttrib, PFNGLGETACTIVEATTRIBPROC, void, (GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name));
GEN_GL_PROC(glGetActiveUniform, PFNGLGETACTIVEUNIFORMPROC, void, (GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name));
GEN_GL_PROC(glGetAttachedShaders, PFNGLGETATTACHEDSHADERSPROC, void, (GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders));
#endif // TODO
GEN_GL_PROC(glGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC, GLint, (GLuint program, const GLchar * name), (program, name));
GEN_GL_VOID_PROC(glGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC, (GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog), (program, bufSize, length, infoLog));
GEN_GL_VOID_PROC(glGetProgramiv, PFNGLGETPROGRAMIVPROC, (GLuint program, GLenum pname, GLint * params), (program, pname, params));
GEN_GL_PROC(glGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC, void, (GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog), (shader, bufSize, length, infoLog));
GEN_GL_PROC(glGetShaderSource, PFNGLGETSHADERSOURCEPROC, void, (GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source), (shader, bufSize, length, source));
GEN_GL_PROC(glGetShaderiv, PFNGLGETSHADERIVPROC, void, (GLuint shader, GLenum pname, GLint * params), (shader, pname, params));
GEN_GL_PROC(glGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC, GLint, (GLuint program, const GLchar * name), (program, name));
GEN_GL_PROC(glGetUniformfv, PFNGLGETUNIFORMFVPROC, void, (GLuint program, GLint location, GLfloat * params), (program, location, params));
GEN_GL_PROC(glGetUniformiv, PFNGLGETUNIFORMIVPROC, void, (GLuint program, GLint location, GLint * params), (program, location, params));
GEN_GL_PROC(glGetVertexAttribPointerv, PFNGLGETVERTEXATTRIBPOINTERVPROC, void, (GLuint index, GLenum pname, void ** pointer), (index, pname, pointer));
#if 0 // TODO
GEN_GL_PROC(glGetVertexAttribdv, PFNGLGETVERTEXATTRIBDVPROC, void, (GLuint index, GLenum pname, GLdouble * params));
GEN_GL_PROC(glGetVertexAttribfv, PFNGLGETVERTEXATTRIBFVPROC, void, (GLuint index, GLenum pname, GLfloat * params));
GEN_GL_PROC(glGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC, void, (GLuint index, GLenum pname, GLint * params));
#endif // TODO
GEN_GL_PROC(glIsProgram, PFNGLISPROGRAMPROC, GLboolean, (GLuint program), (program));
GEN_GL_PROC(glIsShader, PFNGLISSHADERPROC, GLboolean, (GLuint shader), (shader));
GEN_GL_VOID_PROC(glLinkProgram, PFNGLLINKPROGRAMPROC, (GLuint program), (program));
GEN_GL_VOID_PROC(glShaderSource, PFNGLSHADERSOURCEPROC, (GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length), (shader, count, string, length));
#if 0 // TODO
GEN_GL_PROC(glStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC, void, (GLenum face, GLenum func, GLint ref, GLuint mask));
GEN_GL_PROC(glStencilMaskSeparate, PFNGLSTENCILMASKSEPARATEPROC, void, (GLenum face, GLuint mask));
GEN_GL_PROC(glStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC, void, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass));
GEN_GL_PROC(glUniform1f, PFNGLUNIFORM1FPROC, void, (GLint location, GLfloat v0));
GEN_GL_PROC(glUniform1fv, PFNGLUNIFORM1FVPROC, void, (GLint location, GLsizei count, const GLfloat * value));
GEN_GL_PROC(glUniform1i, PFNGLUNIFORM1IPROC, void, (GLint location, GLint v0));
GEN_GL_PROC(glUniform1iv, PFNGLUNIFORM1IVPROC, void, (GLint location, GLsizei count, const GLint * value));
GEN_GL_PROC(glUniform2f, PFNGLUNIFORM2FPROC, void, (GLint location, GLfloat v0, GLfloat v1));
GEN_GL_PROC(glUniform2fv, PFNGLUNIFORM2FVPROC, void, (GLint location, GLsizei count, const GLfloat * value));
GEN_GL_PROC(glUniform2i, PFNGLUNIFORM2IPROC, void, (GLint location, GLint v0, GLint v1));
GEN_GL_PROC(glUniform2iv, PFNGLUNIFORM2IVPROC, void, (GLint location, GLsizei count, const GLint * value));
GEN_GL_PROC(glUniform3f, PFNGLUNIFORM3FPROC, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2));
GEN_GL_PROC(glUniform3fv, PFNGLUNIFORM3FVPROC, void, (GLint location, GLsizei count, const GLfloat * value));
GEN_GL_PROC(glUniform3i, PFNGLUNIFORM3IPROC, void, (GLint location, GLint v0, GLint v1, GLint v2));
GEN_GL_PROC(glUniform3iv, PFNGLUNIFORM3IVPROC, void, (GLint location, GLsizei count, const GLint * value));
GEN_GL_PROC(glUniform4f, PFNGLUNIFORM4FPROC, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3));
GEN_GL_PROC(glUniform4fv, PFNGLUNIFORM4FVPROC, void, (GLint location, GLsizei count, const GLfloat * value));
GEN_GL_PROC(glUniform4i, PFNGLUNIFORM4IPROC, void, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3));
GEN_GL_PROC(glUniform4iv, PFNGLUNIFORM4IVPROC, void, (GLint location, GLsizei count, const GLint * value));
#endif // TODO
GEN_GL_VOID_PROC(glUniformMatrix2fv, PFNGLUNIFORMMATRIX2FVPROC, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value), (location, count, transpose, value));
GEN_GL_VOID_PROC(glUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value), (location, count, transpose, value));
GEN_GL_VOID_PROC(glUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value), (location, count, transpose, value));
GEN_GL_VOID_PROC(glUseProgram, PFNGLUSEPROGRAMPROC, (GLuint program), (program));
#if 0 // TODO
GEN_GL_PROC(glValidateProgram, PFNGLVALIDATEPROGRAMPROC, void, (GLuint program));
GEN_GL_PROC(glVertexAttrib1d, PFNGLVERTEXATTRIB1DPROC, void, (GLuint index, GLdouble x));
GEN_GL_PROC(glVertexAttrib1dv, PFNGLVERTEXATTRIB1DVPROC, void, (GLuint index, const GLdouble * v));
GEN_GL_PROC(glVertexAttrib1f, PFNGLVERTEXATTRIB1FPROC, void, (GLuint index, GLfloat x));
GEN_GL_PROC(glVertexAttrib1fv, PFNGLVERTEXATTRIB1FVPROC, void, (GLuint index, const GLfloat * v));
GEN_GL_PROC(glVertexAttrib1s, PFNGLVERTEXATTRIB1SPROC, void, (GLuint index, GLshort x));
GEN_GL_PROC(glVertexAttrib1sv, PFNGLVERTEXATTRIB1SVPROC, void, (GLuint index, const GLshort * v));
GEN_GL_PROC(glVertexAttrib2d, PFNGLVERTEXATTRIB2DPROC, void, (GLuint index, GLdouble x, GLdouble y));
GEN_GL_PROC(glVertexAttrib2dv, PFNGLVERTEXATTRIB2DVPROC, void, (GLuint index, const GLdouble * v));
GEN_GL_PROC(glVertexAttrib2f, PFNGLVERTEXATTRIB2FPROC, void, (GLuint index, GLfloat x, GLfloat y));
GEN_GL_PROC(glVertexAttrib2fv, PFNGLVERTEXATTRIB2FVPROC, void, (GLuint index, const GLfloat * v));
GEN_GL_PROC(glVertexAttrib2s, PFNGLVERTEXATTRIB2SPROC, void, (GLuint index, GLshort x, GLshort y));
GEN_GL_PROC(glVertexAttrib2sv, PFNGLVERTEXATTRIB2SVPROC, void, (GLuint index, const GLshort * v));
GEN_GL_PROC(glVertexAttrib3d, PFNGLVERTEXATTRIB3DPROC, void, (GLuint index, GLdouble x, GLdouble y, GLdouble z));
GEN_GL_PROC(glVertexAttrib3dv, PFNGLVERTEXATTRIB3DVPROC, void, (GLuint index, const GLdouble * v));
GEN_GL_PROC(glVertexAttrib3f, PFNGLVERTEXATTRIB3FPROC, void, (GLuint index, GLfloat x, GLfloat y, GLfloat z));
GEN_GL_PROC(glVertexAttrib3fv, PFNGLVERTEXATTRIB3FVPROC, void, (GLuint index, const GLfloat * v));
GEN_GL_PROC(glVertexAttrib3s, PFNGLVERTEXATTRIB3SPROC, void, (GLuint index, GLshort x, GLshort y, GLshort z));
GEN_GL_PROC(glVertexAttrib3sv, PFNGLVERTEXATTRIB3SVPROC, void, (GLuint index, const GLshort * v));
GEN_GL_PROC(glVertexAttrib4Nbv, PFNGLVERTEXATTRIB4NBVPROC, void, (GLuint index, const GLbyte * v));
GEN_GL_PROC(glVertexAttrib4Niv, PFNGLVERTEXATTRIB4NIVPROC, void, (GLuint index, const GLint * v));
GEN_GL_PROC(glVertexAttrib4Nsv, PFNGLVERTEXATTRIB4NSVPROC, void, (GLuint index, const GLshort * v));
GEN_GL_PROC(glVertexAttrib4Nub, PFNGLVERTEXATTRIB4NUBPROC, void, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w));
GEN_GL_PROC(glVertexAttrib4Nubv, PFNGLVERTEXATTRIB4NUBVPROC, void, (GLuint index, const GLubyte * v));
GEN_GL_PROC(glVertexAttrib4Nuiv, PFNGLVERTEXATTRIB4NUIVPROC, void, (GLuint index, const GLuint * v));
GEN_GL_PROC(glVertexAttrib4Nusv, PFNGLVERTEXATTRIB4NUSVPROC, void, (GLuint index, const GLushort * v));
GEN_GL_PROC(glVertexAttrib4bv, PFNGLVERTEXATTRIB4BVPROC, void, (GLuint index, const GLbyte * v));
GEN_GL_PROC(glVertexAttrib4d, PFNGLVERTEXATTRIB4DPROC, void, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w));
GEN_GL_PROC(glVertexAttrib4dv, PFNGLVERTEXATTRIB4DVPROC, void, (GLuint index, const GLdouble * v));
GEN_GL_PROC(glVertexAttrib4f, PFNGLVERTEXATTRIB4FPROC, void, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w));
GEN_GL_PROC(glVertexAttrib4fv, PFNGLVERTEXATTRIB4FVPROC, void, (GLuint index, const GLfloat * v));
GEN_GL_PROC(glVertexAttrib4iv, PFNGLVERTEXATTRIB4IVPROC, void, (GLuint index, const GLint * v));
GEN_GL_PROC(glVertexAttrib4s, PFNGLVERTEXATTRIB4SPROC, void, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w));
GEN_GL_PROC(glVertexAttrib4sv, PFNGLVERTEXATTRIB4SVPROC, void, (GLuint index, const GLshort * v));
GEN_GL_PROC(glVertexAttrib4ubv, PFNGLVERTEXATTRIB4UBVPROC, void, (GLuint index, const GLubyte * v));
GEN_GL_PROC(glVertexAttrib4uiv, PFNGLVERTEXATTRIB4UIVPROC, void, (GLuint index, const GLuint * v));
GEN_GL_PROC(glVertexAttrib4usv, PFNGLVERTEXATTRIB4USVPROC, void, (GLuint index, const GLushort * v));
#endif // TODO
GEN_GL_VOID_PROC(glVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer), (index, size, type, normalized, stride, pointer));
#if 0 // TODO
GEN_GL_PROC(glUniformMatrix2x3fv, PFNGLUNIFORMMATRIX2X3FVPROC, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value));
GEN_GL_PROC(glUniformMatrix2x4fv, PFNGLUNIFORMMATRIX2X4FVPROC, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value));
GEN_GL_PROC(glUniformMatrix3x2fv, PFNGLUNIFORMMATRIX3X2FVPROC, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value));
GEN_GL_PROC(glUniformMatrix3x4fv, PFNGLUNIFORMMATRIX3X4FVPROC, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value));
GEN_GL_PROC(glUniformMatrix4x2fv, PFNGLUNIFORMMATRIX4X2FVPROC, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value));
GEN_GL_PROC(glUniformMatrix4x3fv, PFNGLUNIFORMMATRIX4X3FVPROC, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat * value));
GEN_GL_PROC(glBeginConditionalRender, PFNGLBEGINCONDITIONALRENDERPROC, void, (GLuint id, GLenum mode));
GEN_GL_PROC(glBeginTransformFeedback, PFNGLBEGINTRANSFORMFEEDBACKPROC, void, (GLenum primitiveMode));
GEN_GL_PROC(glBindBufferBase, PFNGLBINDBUFFERBASEPROC, void, (GLenum target, GLuint index, GLuint buffer));
GEN_GL_PROC(glBindBufferRange, PFNGLBINDBUFFERRANGEPROC, void, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size));
GEN_GL_PROC(glBindFragDataLocation, PFNGLBINDFRAGDATALOCATIONPROC, void, (GLuint program, GLuint color, const GLchar * name));
GEN_GL_PROC(glBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC, void, (GLenum target, GLuint framebuffer));
GEN_GL_PROC(glBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC, void, (GLenum target, GLuint renderbuffer));
GEN_GL_PROC(glBindVertexArray, PFNGLBINDVERTEXARRAYPROC, void, (GLuint ren_array));
GEN_GL_PROC(glBlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC, void, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter));
GEN_GL_PROC(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC, GLenum, (GLenum target));
GEN_GL_PROC(glClampColor, PFNGLCLAMPCOLORPROC, void, (GLenum target, GLenum clamp));
GEN_GL_PROC(glClearBufferfi, PFNGLCLEARBUFFERFIPROC, void, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil));
GEN_GL_PROC(glClearBufferfv, PFNGLCLEARBUFFERFVPROC, void, (GLenum buffer, GLint drawbuffer, const GLfloat * value));
GEN_GL_PROC(glClearBufferiv, PFNGLCLEARBUFFERIVPROC, void, (GLenum buffer, GLint drawbuffer, const GLint * value));
GEN_GL_PROC(glClearBufferuiv, PFNGLCLEARBUFFERUIVPROC, void, (GLenum buffer, GLint drawbuffer, const GLuint * value));
GEN_GL_PROC(glColorMaski, PFNGLCOLORMASKIPROC, void, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a));
GEN_GL_PROC(glDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC, void, (GLsizei n, const GLuint * framebuffers));
GEN_GL_PROC(glDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC, void, (GLsizei n, const GLuint * renderbuffers));
GEN_GL_PROC(glDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC, void, (GLsizei n, const GLuint * arrays));
GEN_GL_PROC(glDisablei, PFNGLDISABLEIPROC, void, (GLenum target, GLuint index));
GEN_GL_PROC(glEnablei, PFNGLENABLEIPROC, void, (GLenum target, GLuint index));
GEN_GL_PROC(glEndConditionalRender, PFNGLENDCONDITIONALRENDERPROC, void, (void));
GEN_GL_PROC(glEndTransformFeedback, PFNGLENDTRANSFORMFEEDBACKPROC, void, (void));
GEN_GL_PROC(glFlushMappedBufferRange, PFNGLFLUSHMAPPEDBUFFERRANGEPROC, void, (GLenum target, GLintptr offset, GLsizeiptr length));
GEN_GL_PROC(glFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC, void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer));
GEN_GL_PROC(glFramebufferTexture1D, PFNGLFRAMEBUFFERTEXTURE1DPROC, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));
GEN_GL_PROC(glFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level));
GEN_GL_PROC(glFramebufferTexture3D, PFNGLFRAMEBUFFERTEXTURE3DPROC, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset));
GEN_GL_PROC(glFramebufferTextureLayer, PFNGLFRAMEBUFFERTEXTURELAYERPROC, void, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer));
GEN_GL_PROC(glGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC, void, (GLsizei n, GLuint * framebuffers));
GEN_GL_PROC(glGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC, void, (GLsizei n, GLuint * renderbuffers));
GEN_GL_PROC(glGenVertexArrays, PFNGLGENVERTEXARRAYSPROC, void, (GLsizei n, GLuint * arrays));
GEN_GL_PROC(glGenerateMipmap, PFNGLGENERATEMIPMAPPROC, void, (GLenum target));
GEN_GL_PROC(glGetBooleani_v, PFNGLGETBOOLEANI_VPROC, void, (GLenum target, GLuint index, GLboolean * data));
GEN_GL_PROC(glGetFragDataLocation, PFNGLGETFRAGDATALOCATIONPROC, GLint, (GLuint program, const GLchar * name));
GEN_GL_PROC(glGetFramebufferAttachmentParameteriv, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC, void, (GLenum target, GLenum attachment, GLenum pname, GLint * params));
GEN_GL_PROC(glGetIntegeri_v, PFNGLGETINTEGERI_VPROC, void, (GLenum target, GLuint index, GLint * data));
GEN_GL_PROC(glGetRenderbufferParameteriv, PFNGLGETRENDERBUFFERPARAMETERIVPROC, void, (GLenum target, GLenum pname, GLint * params));
extern const GLubyte * (CODEGEN_FUNCPTR *_ptrc_glGetStringi)(GLenum name, GLuint index);
GEN_GL_PROC(glGetTexParameterIiv, PFNGLGETTEXPARAMETERIIVPROC, void, (GLenum target, GLenum pname, GLint * params));
GEN_GL_PROC(glGetTexParameterIuiv, PFNGLGETTEXPARAMETERIUIVPROC, void, (GLenum target, GLenum pname, GLuint * params));
GEN_GL_PROC(glGetTransformFeedbackVarying, PFNGLGETTRANSFORMFEEDBACKVARYINGPROC, void, (GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name));
GEN_GL_PROC(glGetUniformuiv, PFNGLGETUNIFORMUIVPROC, void, (GLuint program, GLint location, GLuint * params));
GEN_GL_PROC(glGetVertexAttribIiv, PFNGLGETVERTEXATTRIBIIVPROC, void, (GLuint index, GLenum pname, GLint * params));
GEN_GL_PROC(glGetVertexAttribIuiv, PFNGLGETVERTEXATTRIBIUIVPROC, void, (GLuint index, GLenum pname, GLuint * params));
GEN_GL_PROC(glIsEnabledi, PFNGLISENABLEDIPROC, GLboolean, (GLenum target, GLuint index));
GEN_GL_PROC(glIsFramebuffer, PFNGLISFRAMEBUFFERPROC, GLboolean, (GLuint framebuffer));
GEN_GL_PROC(glIsRenderbuffer, PFNGLISRENDERBUFFERPROC, GLboolean, (GLuint renderbuffer));
GEN_GL_PROC(glIsVertexArray, PFNGLISVERTEXARRAYPROC, GLboolean, (GLuint ren_array));
extern void * (CODEGEN_FUNCPTR *_ptrc_glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GEN_GL_PROC(glRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC, void, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height));
GEN_GL_PROC(glRenderbufferStorageMultisample, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height));
GEN_GL_PROC(glTexParameterIiv, PFNGLTEXPARAMETERIIVPROC, void, (GLenum target, GLenum pname, const GLint * params));
GEN_GL_PROC(glTexParameterIuiv, PFNGLTEXPARAMETERIUIVPROC, void, (GLenum target, GLenum pname, const GLuint * params));
GEN_GL_PROC(glTransformFeedbackVaryings, PFNGLTRANSFORMFEEDBACKVARYINGSPROC, void, (GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode));
GEN_GL_PROC(glUniform1ui, PFNGLUNIFORM1UIPROC, void, (GLint location, GLuint v0));
GEN_GL_PROC(glUniform1uiv, PFNGLUNIFORM1UIVPROC, void, (GLint location, GLsizei count, const GLuint * value));
GEN_GL_PROC(glUniform2ui, PFNGLUNIFORM2UIPROC, void, (GLint location, GLuint v0, GLuint v1));
GEN_GL_PROC(glUniform2uiv, PFNGLUNIFORM2UIVPROC, void, (GLint location, GLsizei count, const GLuint * value));
GEN_GL_PROC(glUniform3ui, PFNGLUNIFORM3UIPROC, void, (GLint location, GLuint v0, GLuint v1, GLuint v2));
GEN_GL_PROC(glUniform3uiv, PFNGLUNIFORM3UIVPROC, void, (GLint location, GLsizei count, const GLuint * value));
GEN_GL_PROC(glUniform4ui, PFNGLUNIFORM4UIPROC, void, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3));
GEN_GL_PROC(glUniform4uiv, PFNGLUNIFORM4UIVPROC, void, (GLint location, GLsizei count, const GLuint * value));
GEN_GL_PROC(glVertexAttribI1i, PFNGLVERTEXATTRIBI1IPROC, void, (GLuint index, GLint x));
GEN_GL_PROC(glVertexAttribI1iv, PFNGLVERTEXATTRIBI1IVPROC, void, (GLuint index, const GLint * v));
GEN_GL_PROC(glVertexAttribI1ui, PFNGLVERTEXATTRIBI1UIPROC, void, (GLuint index, GLuint x));
GEN_GL_PROC(glVertexAttribI1uiv, PFNGLVERTEXATTRIBI1UIVPROC, void, (GLuint index, const GLuint * v));
GEN_GL_PROC(glVertexAttribI2i, PFNGLVERTEXATTRIBI2IPROC, void, (GLuint index, GLint x, GLint y));
GEN_GL_PROC(glVertexAttribI2iv, PFNGLVERTEXATTRIBI2IVPROC, void, (GLuint index, const GLint * v));
GEN_GL_PROC(glVertexAttribI2ui, PFNGLVERTEXATTRIBI2UIPROC, void, (GLuint index, GLuint x, GLuint y));
GEN_GL_PROC(glVertexAttribI2uiv, PFNGLVERTEXATTRIBI2UIVPROC, void, (GLuint index, const GLuint * v));
GEN_GL_PROC(glVertexAttribI3i, PFNGLVERTEXATTRIBI3IPROC, void, (GLuint index, GLint x, GLint y, GLint z));
GEN_GL_PROC(glVertexAttribI3iv, PFNGLVERTEXATTRIBI3IVPROC, void, (GLuint index, const GLint * v));
GEN_GL_PROC(glVertexAttribI3ui, PFNGLVERTEXATTRIBI3UIPROC, void, (GLuint index, GLuint x, GLuint y, GLuint z));
GEN_GL_PROC(glVertexAttribI3uiv, PFNGLVERTEXATTRIBI3UIVPROC, void, (GLuint index, const GLuint * v));
GEN_GL_PROC(glVertexAttribI4bv, PFNGLVERTEXATTRIBI4BVPROC, void, (GLuint index, const GLbyte * v));
GEN_GL_PROC(glVertexAttribI4i, PFNGLVERTEXATTRIBI4IPROC, void, (GLuint index, GLint x, GLint y, GLint z, GLint w));
GEN_GL_PROC(glVertexAttribI4iv, PFNGLVERTEXATTRIBI4IVPROC, void, (GLuint index, const GLint * v));
GEN_GL_PROC(glVertexAttribI4sv, PFNGLVERTEXATTRIBI4SVPROC, void, (GLuint index, const GLshort * v));
GEN_GL_PROC(glVertexAttribI4ubv, PFNGLVERTEXATTRIBI4UBVPROC, void, (GLuint index, const GLubyte * v));
GEN_GL_PROC(glVertexAttribI4ui, PFNGLVERTEXATTRIBI4UIPROC, void, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w));
GEN_GL_PROC(glVertexAttribI4uiv, PFNGLVERTEXATTRIBI4UIVPROC, void, (GLuint index, const GLuint * v));
GEN_GL_PROC(glVertexAttribI4usv, PFNGLVERTEXATTRIBI4USVPROC, void, (GLuint index, const GLushort * v));
GEN_GL_PROC(glVertexAttribIPointer, PFNGLVERTEXATTRIBIPOINTERPROC, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer));
GEN_GL_PROC(glCopyBufferSubData, PFNGLCOPYBUFFERSUBDATAPROC, void, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size));
GEN_GL_PROC(glDrawArraysInstanced, PFNGLDRAWARRAYSINSTANCEDPROC, void, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount));
GEN_GL_PROC(glDrawElementsInstanced, PFNGLDRAWELEMENTSINSTANCEDPROC, void, (GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount));
GEN_GL_PROC(glGetActiveUniformBlockName, PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC, void, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName));
GEN_GL_PROC(glGetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC, void, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params));
GEN_GL_PROC(glGetActiveUniformName, PFNGLGETACTIVEUNIFORMNAMEPROC, void, (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName));
GEN_GL_PROC(glGetActiveUniformsiv, PFNGLGETACTIVEUNIFORMSIVPROC, void, (GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params));
GEN_GL_PROC(glGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC, GLuint, (GLuint program, const GLchar * uniformBlockName));
GEN_GL_PROC(glGetUniformIndices, PFNGLGETUNIFORMINDICESPROC, void, (GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices));
GEN_GL_PROC(glPrimitiveRestartIndex, PFNGLPRIMITIVERESTARTINDEXPROC, void, (GLuint index));
GEN_GL_PROC(glTexBuffer, PFNGLTEXBUFFERPROC, void, (GLenum target, GLenum internalformat, GLuint buffer));
GEN_GL_PROC(glUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC, void, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding));
GEN_GL_PROC(glClientWaitSync, PFNGLCLIENTWAITSYNCPROC, GLenum, (GLsync sync, GLbitfield flags, GLuint64 timeout));
GEN_GL_PROC(glDeleteSync, PFNGLDELETESYNCPROC, void, (GLsync sync));
GEN_GL_PROC(glDrawElementsBaseVertex, PFNGLDRAWELEMENTSBASEVERTEXPROC, void, (GLenum mode, GLsizei count, GLenum type, const void * indices, GLint basevertex));
GEN_GL_PROC(glDrawElementsInstancedBaseVertex, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC, void, (GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount, GLint basevertex));
GEN_GL_PROC(glDrawRangeElementsBaseVertex, PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC, void, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices, GLint basevertex));
GEN_GL_PROC(glFenceSync, PFNGLFENCESYNCPROC, GLsync, (GLenum condition, GLbitfield flags));
GEN_GL_PROC(glFramebufferTexture, PFNGLFRAMEBUFFERTEXTUREPROC, void, (GLenum target, GLenum attachment, GLuint texture, GLint level));
GEN_GL_PROC(glGetBufferParameteri64v, PFNGLGETBUFFERPARAMETERI64VPROC, void, (GLenum target, GLenum pname, GLint64 * params));
GEN_GL_PROC(glGetInteger64i_v, PFNGLGETINTEGER64I_VPROC, void, (GLenum target, GLuint index, GLint64 * data));
GEN_GL_PROC(glGetInteger64v, PFNGLGETINTEGER64VPROC, void, (GLenum pname, GLint64 * data));
GEN_GL_PROC(glGetMultisamplefv, PFNGLGETMULTISAMPLEFVPROC, void, (GLenum pname, GLuint index, GLfloat * val));
GEN_GL_PROC(glGetSynciv, PFNGLGETSYNCIVPROC, void, (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei * length, GLint * values));
GEN_GL_PROC(glIsSync, PFNGLISSYNCPROC, GLboolean, (GLsync sync));
GEN_GL_PROC(glMultiDrawElementsBaseVertex, PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC, void, (GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount, const GLint * basevertex));
GEN_GL_PROC(glProvokingVertex, PFNGLPROVOKINGVERTEXPROC, void, (GLenum mode));
GEN_GL_PROC(glSampleMaski, PFNGLSAMPLEMASKIPROC, void, (GLuint maskNumber, GLbitfield mask));
GEN_GL_PROC(glTexImage2DMultisample, PFNGLTEXIMAGE2DMULTISAMPLEPROC, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations));
GEN_GL_PROC(glTexImage3DMultisample, PFNGLTEXIMAGE3DMULTISAMPLEPROC, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations));
GEN_GL_PROC(glWaitSync, PFNGLWAITSYNCPROC, void, (GLsync sync, GLbitfield flags, GLuint64 timeout));
#endif // TODO
#endif
