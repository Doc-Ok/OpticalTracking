/***********************************************************************
FunctionPointerHack - Evil hack to cast a pointer returned from dlsym
into a function pointer without a warning.
Copyright (c) 2003-2011 Oliver Kreylos

This file is part of the Plugin Handling Library (Plugins).

The Plugin Handling Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Plugin Handling Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Plugin Handling Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef PLUGINS_FUNCTIONPOINTERHACK_INCLUDED
#define PLUGINS_FUNCTIONPOINTERHACK_INCLUDED

#include <stddef.h>
#include <dlfcn.h>

namespace Plugins {

namespace {

typedef void (*FunctionPointer)(void);

inline FunctionPointer nowarninghack_dlsym(void* dsoHandle,const char* functionName)
	{
	/* Use a workaround to get rid of warnings in g++'s pedantic mode: */
	ptrdiff_t intermediate=reinterpret_cast<ptrdiff_t>(dlsym(dsoHandle,functionName));
	return reinterpret_cast<FunctionPointer>(intermediate);
	}

}

}

#endif
