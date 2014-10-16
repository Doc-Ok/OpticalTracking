/***********************************************************************
GLEXTPackedDepthStencil - OpenGL extension class for the
GL_EXT_packed_depth_stencil extension.
Copyright (c) 2013-2014 Oliver Kreylos

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

#include <GL/Extensions/GLEXTPackedDepthStencil.h>

#include <GL/gl.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>

/************************************************
Static elements of class GLEXTPackedDepthStencil:
************************************************/

GL_THREAD_LOCAL(GLEXTPackedDepthStencil*) GLEXTPackedDepthStencil::current=0;
const char* GLEXTPackedDepthStencil::name="GL_EXT_packed_depth_stencil";

/****************************************
Methods of class GLEXTPackedDepthStencil:
****************************************/

GLEXTPackedDepthStencil::GLEXTPackedDepthStencil(void)
	{
	}

GLEXTPackedDepthStencil::~GLEXTPackedDepthStencil(void)
	{
	}

const char* GLEXTPackedDepthStencil::getExtensionName(void) const
	{
	return name;
	}

void GLEXTPackedDepthStencil::activate(void)
	{
	current=this;
	}

void GLEXTPackedDepthStencil::deactivate(void)
	{
	current=0;
	}

bool GLEXTPackedDepthStencil::isSupported(void)
	{
	/* Ask the current extension manager whether the extension is supported in the current OpenGL context: */
	return GLExtensionManager::isExtensionSupported(name);
	}

void GLEXTPackedDepthStencil::initExtension(void)
	{
	/* Check if the extension is already initialized: */
	if(!GLExtensionManager::isExtensionRegistered(name))
		{
		/* Create a new extension object: */
		GLEXTPackedDepthStencil* newExtension=new GLEXTPackedDepthStencil;
		
		/* Register the extension with the current extension manager: */
		GLExtensionManager::registerExtension(newExtension);
		}
	}
