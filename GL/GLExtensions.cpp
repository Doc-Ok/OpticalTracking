/***********************************************************************
GLExtensions - Functions to query for availability of OpenGL extensions.
Copyright (c) 2004-2005 Oliver Kreylos

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

#include <string.h>
#include <GL/gl.h>
#ifndef GLX_GLXEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES 1
#endif
#include <GL/glx.h>

#include <GL/GLExtensions.h>

bool glHasExtension(const char* queryExtensionName)
	{
	ssize_t queryLen=strlen(queryExtensionName);
	
	/* Get the space-separated string of extension names: */
	const char* extensionNames=(const char*)glGetString(GL_EXTENSIONS);
	
	/* Compare each extension name against the query name: */
	const char* extBegin=extensionNames;
	while(*extBegin!='\0')
		{
		/* Find the next space or end-of-string character: */
		const char* extEnd;
		for(extEnd=extBegin;*extEnd!='\0'&&*extEnd!=' ';++extEnd)
			;
		
		/* Compare extension name against query name: */
		if(extEnd-extBegin==queryLen&&strncasecmp(extBegin,queryExtensionName,queryLen)==0)
			return true;
		
		/* Go to the next extension: */
		extBegin=extEnd;
		while(*extBegin==' ')
			++extBegin;
		}
	
	return false;
	}

void (*glGetFunctionPtr(const char* functionName))(void)
	{
	return glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(functionName));
	}
