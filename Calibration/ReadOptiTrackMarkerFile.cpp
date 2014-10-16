/***********************************************************************
ReadOptiTrackMarkerFile - Helper functions to read marker files in XML
format as written by the NaturalPoint OptiTrack rigid body tracking
toolkit.
Copyright (c) 2008-2010 Oliver Kreylos

This file is part of the Vrui calibration utility package.

The Vrui calibration utility package is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Vrui calibration utility package is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Vrui calibration utility package; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include "ReadOptiTrackMarkerFile.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <Geometry/Point.h>

namespace {

/****************
Helper functions:
****************/

int readTagParameter(std::ifstream& file,std::string& parameterName,std::string& parameterValue)
	{
	/* Skip whitespace: */
	int c=file.get();
	while(isspace(c))
		c=file.get();
	
	/* Check for closing angle bracket: */
	if(c=='>')
		return 1; // Standard tag closer
	else if(c=='/')
		{
		c=file.get();
		if(c=='>')
			return 2; // Self-closing tag closer
		else
			throw 1; // Syntax error
		}
	
	/* Parse the parameter name: */
	parameterName.clear();
	while(!isspace(c)&&c!='=')
		{
		parameterName.push_back(c);
		c=file.get();
		}
	
	/* Skip whitespace and the equal sign: */
	while(isspace(c))
		c=file.get();
	c=file.get();
	while(isspace(c))
		c=file.get();
	
	/* Parse the parameter value: */
	parameterValue.clear();
	if(c=='"'||c=='\'')
		{
		int delimiter=c;
		c=file.get();
		while(c!=delimiter)
			{
			parameterValue.push_back(c);
			c=file.get();
			}
		}
	else
		{
		while(!isspace(c)&&c!='/'&&c!='>')
			{
			parameterValue.push_back(c);
			c=file.get();
			}
		
		if(c=='/'||c=='/')
			file.putback(c);
		}
	
	return 0;
	}

}

void readOptiTrackMarkerFile(const char* fileName,const char* bodyName,double scale,bool flipZ,std::vector<Geometry::Point<double,3> >& markers)
	{
	/* Open the input file: */
	std::ifstream file;
	file.open(fileName);
	
	/* Parse the XML file: */
	bool inRigidBodyProfile=false;
	bool inRigidBody=false;
	std::string currentRigidBodyName;
	bool storeMarkers=false;
	
	if(file.is_open())
		{
		while(true)
			{
			/* Read the next tag: */
			int c;
			while(!file.eof()&&(c=file.get())!='<')
				;
			if(file.eof())
				break;
			
			/* Parse the tag type: */
			std::string tagType;
			bool closingTag=false;
			c=file.get();
			if(c=='/')
				closingTag=true;
			else
				tagType.push_back(c);
			while(!isspace((c=file.get())))
				tagType.push_back(c);
			
			/* Parse the tag: */
			if(tagType=="RigidBodyProfile")
				{
				if(!closingTag)
					{
					/* Skip all tag parameters: */
					int flag;
					std::string name,value;
					while((flag=readTagParameter(file,name,value))==0)
						;
					inRigidBodyProfile=flag==1;
					}
				else
					inRigidBodyProfile=false;
				}
			else if(inRigidBodyProfile&&tagType=="RigidBody")
				{
				if(!closingTag)
					{
					/* Parse all tag parameters: */
					int flag;
					std::string name,value;
					while((flag=readTagParameter(file,name,value))==0)
						{
						if(name=="Name")
							{
							currentRigidBodyName=value;
							storeMarkers=bodyName==0||currentRigidBodyName==bodyName;
							}
						}
					inRigidBody=flag==1;
					}
				else
					inRigidBody=false;
				}
			else if(inRigidBody&&tagType=="Marker")
				{
				if(!closingTag)
					{
					/* Parse all tag parameters: */
					int flag;
					std::string name,value;
					Geometry::Point<double,3> marker=Geometry::Point<double,3>::origin;
					while((flag=readTagParameter(file,name,value))==0)
						{
						if(name=="X")
							marker[0]=atof(value.c_str())*scale;
						else if(name=="Y")
							marker[1]=atof(value.c_str())*scale;
						else if(name=="Z")
							{
							marker[2]=atof(value.c_str())*scale;
							if(flipZ)
								marker[2]=-marker[2];
							}
						}
					if(storeMarkers)
						markers.push_back(marker);
					}
				}
			}
		}
	
	file.close();
	}
