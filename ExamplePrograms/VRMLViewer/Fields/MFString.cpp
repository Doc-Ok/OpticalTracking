/***********************************************************************
MFString - Class for fields containing multiple string values.
Copyright (c) 2008 Oliver Kreylos

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

#include "../VRMLParser.h"

#include "MFString.h"

/*************************
Methods of class MFString:
*************************/

std::vector<String> MFString::parse(VRMLParser& parser)
	{
	std::vector<String> result;
	
	/* Check for the opening bracket: */
	if(parser.isToken("["))
		{
		/* Skip the opening bracket: */
		parser.getNextToken();
		
		/* Parse strings until closing bracket: */
		while(!parser.isToken("]"))
			{
			/* Parse and store the current token: */
			result.push_back(String(parser.getToken()));
			
			/* Go to the next token: */
			parser.getNextToken();
			}

		/* Skip the closing bracket: */
		parser.getNextToken();
		}
	else
		{
		/* Parse and store the current token: */
		result.push_back(String(parser.getToken()));
		
		/* Go to the next token: */
		parser.getNextToken();
		}
	
	return result;
	}
