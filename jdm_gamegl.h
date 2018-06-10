/* jdm_gamegl.h : single file library for initializing OpenGL - public domain */
/*
 * To the extent possible under law, I have waived all copyright and related or
 * neighboring rights to this source code.
 *
 * For full legal text see
 *         https://creativecommons.org/publicdomain/zero/1.0/legalcode.txt
 */
/* USAGE:
 *
 * #define JDM_GAMEGL_IMPLEMENTATION
 * #include "jdm_gamegl.h"
 *
 * void game_initialize(void) {
 * ... load your data files, shaders, etc here
 * }
 *
 * void game_paint(void) {
 * ... draw your scene
 * }
 *
 * API:
 *
 * void pr_err(const char *fmt, ...);
 *   prints an error, and may show a message box that blocks.
 * void die(const char *msg)
 *   terminates the program with a message
 * void pr_info(const char *fmt, ...);
 *   prints a formatted string
 * void pr_dbg(const char *fmt, ...);
 *   prints a formmated string if not compiled with NDEBUG defined
 *
 * BUGS:
 *
 */
#ifndef JDM_GAMEGL_H
#define JDM_GAMEGL_H

#ifndef _WIN32
/* windows version uses many function pointers instead of linking */
#  define GL_GLEXT_PROTOTYPES
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

void game_initialize(void);
void game_paint(void);

#ifdef JDM_GAMEGL_IMPLEMENTATION
#  if defined(_WIN32)
#    include "wgl.c"
#  elif defined(__ANDROID__)
#    error Android support not implemented. sorry!
#  elif defined(__APPLE__) && defined(__MACH__)
#    error MacOS support not implemented. sorry!
#  elif defined(__GNU__) || defined(_POSIX_VERSION) || defined(__unix__)
#    include "glx.c"
#  else
#    error Unknown platform not supported. sorry!
#  endif
#endif /* JDM_GAMEGL_IMPLEMENTATION */

#endif
