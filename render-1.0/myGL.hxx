////////////////////////////////////////////////////////////////////
//
// $Id: myGL.hxx 2025/04/30 17:51:59 kanai Exp $
//
// Copyright (c) 2021 Takashi Kanai
// Released under the MIT license
//
////////////////////////////////////////////////////////////////////

#ifndef _MYGL_HXX
#define _MYGL_HXX 1

#ifdef __APPLE__
    #include <GL/glew.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GL/glut.h>
    #include <GL/glext.h>
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
    #include <GL/glext.h>
    #include <GL/wglext.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
    #include <GL/glext.h>
#endif

#endif // _MYGL_HXX
