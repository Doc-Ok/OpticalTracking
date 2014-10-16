/***********************************************************************
Container - Base class for GLMotif UI components that contain other
components.
Copyright (c) 2001-2014 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GLMotif/Container.h>

#include <string.h>
#include <Misc/ThrowStdErr.h>

namespace GLMotif {

/**************************
Methods of class Container:
**************************/

Container::Container(const char* sName,Container* sParent,bool sManageChild)
	:Widget(sName,sParent,false)
	{
	/* Manage me: */
	if(sManageChild)
		manageChild();
	}

Widget* Container::findChild(const char* childName)
	{
	/* Traverse all children in the container until the first name matches: */
	Widget* child;
	for(child=getFirstChild();child!=0&&strcmp(child->getName(),childName)!=0;child=getNextChild(child))
		;
	return child;
	}

namespace {

/****************
Helper functions:
****************/

inline bool strsegequal(const char* s1Begin,const char* s1End,const char* s2)
	{
	/* Find the first mismatching character in the two strings: */
	for(;s1Begin!=s1End&&*s2!='\0'&&*s1Begin==*s2;++s1Begin,++s2)
		;
	
	/* Strings are equal if both strings are over and no mismatching characters were found: */
	return s1Begin==s1End&&*s2=='\0';
	}

}

Widget* Container::findDescendant(const char* descendantPath)
	{
	/* Find the next forward slash in the descendant path: */
	const char* slashPtr;
	for(slashPtr=descendantPath;*slashPtr!='\0'&&*slashPtr!='/';++slashPtr)
		;
	
	/* Traverse all children in the container until the first name matches: */
	Widget* child;
	for(child=getFirstChild();child!=0&&!strsegequal(descendantPath,slashPtr,child->getName());child=getNextChild(child))
		;
	
	/* Check if there is another path segment: */
	if(*slashPtr=='/')
		{
		/* Check if the found child is a container: */
		Container* container=dynamic_cast<Container*>(child);
		if(container!=0)
			{
			/* Search the found container: */
			return container->findDescendant(slashPtr+1);
			}
		else
			{
			Misc::throwStdErr("GLMotif::Container::findDescendant: Path component not found");
			return 0; // Just to make compiler happy
			}
		}
	else
		return child;
	}

}
