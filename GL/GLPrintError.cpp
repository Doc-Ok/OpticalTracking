/***********************************************************************
GLPrintError - Helper function to print a plain-text OpenGL error
message.
Copyright (c) 2010-2013 Oliver Kreylos

This file is part of the OpenGL Support Library (GLSupport).

The OpenGL Support Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL Support Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL Support Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/GLPrintError.h>

#include <GL/gl.h>

namespace {

/****************
Helper functions:
****************/

void glPrintErrorMsg(std::ostream& os,GLenum error)
	{
	switch(error)
		{
		case 0:
			os<<"Internal error in glGetError()";
			break;
		
		case GL_INVALID_ENUM:
			os<<"Invalid enum";
			break;
		
		case GL_INVALID_VALUE:
			os<<"Invalid value";
			break;
		
		case GL_INVALID_OPERATION:
			os<<"Invalid operation";
			break;
		
		case GL_STACK_OVERFLOW:
			os<<"Stack overflow";
			break;
		
		case GL_STACK_UNDERFLOW:
			os<<"Stack underflow";
			break;
		
		case GL_OUT_OF_MEMORY:
			os<<"Out of memory";
			break;
		
		case GL_TABLE_TOO_LARGE:
			os<<"Table too large";
			break;
		
		default:
			os<<"Unknown error "<<error;
		}
	}

}

void glPrintError(std::ostream& os)
	{
	GLenum error;
	while((error=glGetError())!=GL_NO_ERROR)
		{
		os<<"GL error: ";
		glPrintErrorMsg(os,error);
		os<<std::endl;
		}
	}

void glPrintError(std::ostream& os,const char* messageTag)
	{
	GLenum error;
	while((error=glGetError())!=GL_NO_ERROR)
		{
		os<<messageTag<<' ';
		glPrintErrorMsg(os,error);
		os<<std::endl;
		}
	}
