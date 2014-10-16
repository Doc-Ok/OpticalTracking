/***********************************************************************
SFBool - Class for fields containing a single boolean value.
Copyright (c) 2006-2008 Oliver Kreylos

This file is part of the Virtual Reality VRML viewer (VRMLViewer).

The Virtual Reality VRML viewer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Virtual Reality VRML viewer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality VRML viewer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Misc/ThrowStdErr.h>

#include "../VRMLParser.h"

#include "SFBool.h"

/***********************
Methods of class SFBool:
***********************/

Bool SFBool::parse(VRMLParser& parser)
	{
	Bool result;
	
	/* Parse the current token: */
	if(parser.isToken("true"))
		result=true;
	else if(parser.isToken("false"))
		result=false;
	else
		Misc::throwStdErr("SFBool::parse: unrecognized boolean value %s",parser.getToken());
	
	/* Go to the next token: */
	parser.getNextToken();
	
	return result;
	}
