/***********************************************************************
GLValueCoders - Value coder classes for OpenGL abstraction classes.
Copyright (c) 2004-2013 Oliver Kreylos

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

#include <GL/GLValueCoders.h>

#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLVector.h>
#include <GL/GLBox.h>
#include <GL/GLMaterial.h>

namespace Misc {

/****************************************************
Methods of class ValueCoder<GLColor<ScalarParam,3> >:
****************************************************/

template <class ScalarParam>
std::string ValueCoder<GLColor<ScalarParam,3> >::encode(const GLColor<ScalarParam,3>& value)
	{
	/* Convert color into vector of GLdoubles: */
	GLColor<GLdouble,3> dv(value);
	
	/* Return the encoded vector: */
	return CFixedArrayValueCoder<GLdouble,3>::encode(dv.getRgba());
	}

template <class ScalarParam>
GLColor<ScalarParam,3> ValueCoder<GLColor<ScalarParam,3> >::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		/* Decode string into array of GLdoubles: */
		GLdouble components[3];
		CFixedArrayValueCoder<GLdouble,3>(components).decode(start,end,decodeEnd);
		
		/* Return result color: */
		return GLColor<ScalarParam,3>(components);
		}
	catch(std::runtime_error err)
		{
		throw DecodingError(std::string("Unable to convert ")+std::string(start,end)+std::string(" to GLColor due to ")+err.what());
		}
	}

/****************************************************
Methods of class ValueCoder<GLColor<ScalarParam,4> >:
****************************************************/

template <class ScalarParam>
std::string ValueCoder<GLColor<ScalarParam,4> >::encode(const GLColor<ScalarParam,4>& value)
	{
	/* Convert color into vector of GLdoubles: */
	GLColor<GLdouble,4> dv(value);
	
	/* Only encode three components if alpha is default value: */
	if(dv[3]==1.0)
		return CFixedArrayValueCoder<GLdouble,3>::encode(dv.getRgba());
	else
		return CFixedArrayValueCoder<GLdouble,4>::encode(dv.getRgba());
	}

template <class ScalarParam>
GLColor<ScalarParam,4> ValueCoder<GLColor<ScalarParam,4> >::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		/* Decode string into array of GLdoubles: */
		GLdouble components[4];
		DynamicArrayValueCoder<GLdouble> decoder(components,4);
		decoder.decode(start,end,decodeEnd);
		
		/* Check for correct vector size: */
		if(decoder.numElements<3||decoder.numElements>4)
			throw DecodingError("wrong number of components");
		
		/* Set default alpha value for three-component colors: */
		if(decoder.numElements==3)
			components[3]=1.0;
		
		/* Return result color: */
		return GLColor<ScalarParam,4>(components);
		}
	catch(std::runtime_error err)
		{
		throw DecodingError(std::string("Unable to convert ")+std::string(start,end)+std::string(" to GLColor due to ")+err.what());
		}
	}

/**********************************************************************
Methods of class ValueCoder<GLVector<ScalarParam,numComponentsParam> >:
**********************************************************************/

template <class ScalarParam,GLsizei numComponentsParam>
std::string ValueCoder<GLVector<ScalarParam,numComponentsParam> >::encode(const GLVector<ScalarParam,numComponentsParam>& value)
	{
	/* Return the encoded vector: */
	return CFixedArrayValueCoder<ScalarParam,numComponentsParam>::encode(value.getXyzw());
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLVector<ScalarParam,numComponentsParam> ValueCoder<GLVector<ScalarParam,numComponentsParam> >::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		GLVector<ScalarParam,numComponentsParam> result;
		CFixedArrayValueCoder<ScalarParam,numComponentsParam>(result.getXyzw()).decode(start,end,decodeEnd);
		return result;
		}
	catch(std::runtime_error err)
		{
		throw DecodingError(std::string("Unable to convert ")+std::string(start,end)+std::string(" to GLVector due to ")+err.what());
		}
	}

/*******************************************************************
Methods of class ValueCoder<GLBox<ScalarParam,numComponentsParam> >:
*******************************************************************/

template <class ScalarParam,GLsizei numComponentsParam>
std::string ValueCoder<GLBox<ScalarParam,numComponentsParam> >::encode(const GLBox<ScalarParam,numComponentsParam>& value)
	{
	std::string result;
	
	result.append(ValueCoder<typename GLBox<ScalarParam,numComponentsParam>::Vector>::encode(value.origin));
	result.push_back(',');
	result.push_back(' ');
	result.append(ValueCoder<typename GLBox<ScalarParam,numComponentsParam>::Vector>::encode(value.size));
	
	return result;
	}

template <class ScalarParam,GLsizei numComponentsParam>
GLBox<ScalarParam,numComponentsParam> ValueCoder<GLBox<ScalarParam,numComponentsParam> >::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		GLBox<ScalarParam,numComponentsParam> result;
		
		const char* cPtr=start;
		
		/* Decode box origin: */
		result.origin=ValueCoder<typename GLBox<ScalarParam,numComponentsParam>::Vector>::decode(cPtr,end,&cPtr);
		cPtr=skipWhitespace(cPtr,end);
		
		/* Check for comma separator: */
		cPtr=checkSeparator(',',cPtr,end);
		
		/* Decode box size: */
		result.size=ValueCoder<typename GLBox<ScalarParam,numComponentsParam>::Vector>::decode(cPtr,end,&cPtr);
		
		if(decodeEnd!=0)
			*decodeEnd=cPtr;
		return result;
		}
	catch(std::runtime_error err)
		{
		throw DecodingError(std::string("Unable to convert ")+std::string(start,end)+std::string(" to GLBox due to ")+err.what());
		}
	}

/***************************************
Methods of class ValueCoder<GLMaterial>:
***************************************/

std::string ValueCoder<GLMaterial>::encode(const GLMaterial& value)
	{
	std::string result="{ ";
	result+="Ambient = ";
	result+=ValueCoder<GLMaterial::Color>::encode(value.ambient);
	result+="; ";
	result+="Diffuse = ";
	result+=ValueCoder<GLMaterial::Color>::encode(value.diffuse);
	result+="; ";
	result+="Specular = ";
	result+=ValueCoder<GLMaterial::Color>::encode(value.specular);
	result+="; ";
	result+="Shininess = ";
	result+=ValueCoder<GLfloat>::encode(value.shininess);
	result+="; ";
	result+="Emission = ";
	result+=ValueCoder<GLMaterial::Color>::encode(value.emission);
	result+="; }";
	return result;
	}

GLMaterial ValueCoder<GLMaterial>::decode(const char* start,const char* end,const char** decodeEnd)
	{
	try
		{
		GLMaterial result;
		
		/* Check if the string starts with an opening brace: */
		const char* cPtr=start;
		if(*cPtr=='{')
			{
			/* It's the compound value notation of materials: */
			++cPtr;
			cPtr=skipWhitespace(cPtr,end);
			while(cPtr!=end&&*cPtr!='}')
				{
				/* Read the tag: */
				std::string tag=ValueCoder<std::string>::decode(cPtr,end,&cPtr);
				cPtr=skipWhitespace(cPtr,end);
				
				/* Check for equal sign: */
				cPtr=checkSeparator('=',cPtr,end);
				
				/* Read the tag value: */
				if(tag=="Ambient")
					result.ambient=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
				else if(tag=="Diffuse")
					result.diffuse=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
				else if(tag=="AmbientDiffuse")
					result.ambient=result.diffuse=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
				else if(tag=="Specular")
					result.specular=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
				else if(tag=="Shininess")
					result.shininess=ValueCoder<GLfloat>::decode(cPtr,end,&cPtr);
				else if(tag=="Emission")
					result.emission=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
				else
					throw DecodingError(std::string("unknown tag ")+tag);
				cPtr=skipWhitespace(cPtr,end);
				
				/* Check for semicolon: */
				cPtr=checkSeparator(';',cPtr,end);
				}
			
			/* Check for closing brace: */
			if(cPtr==end)
				throw DecodingError("missing closing brace");
			}
		else if(*cPtr=='(')
			{
			/* It's the old-style notation of materials: */
			++cPtr;
			cPtr=skipWhitespace(cPtr,end);
			result.ambient=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			cPtr=checkSeparator(',',cPtr,end);
			result.diffuse=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			cPtr=checkSeparator(',',cPtr,end);
			result.specular=ValueCoder<GLMaterial::Color>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			cPtr=checkSeparator(',',cPtr,end);
			result.shininess=ValueCoder<GLfloat>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			
			/* Check for closing parenthesis: */
			if(cPtr==end||*cPtr!=')')
				throw DecodingError("missing closing parenthesis");
			++cPtr;
			}
		else
			throw DecodingError("missing opening delimiter");
		
		/* Return result material: */
		if(decodeEnd!=0)
			*decodeEnd=cPtr;
		return result;
		}
	catch(std::runtime_error err)
		{
		throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to GLMaterial due to ")+err.what());
		}
	}

/**********************************************
Force instantiation of all value coder classes:
**********************************************/

template class ValueCoder<GLColor<GLbyte,3> >;
template class ValueCoder<GLColor<GLubyte,3> >;
template class ValueCoder<GLColor<GLint,3> >;
template class ValueCoder<GLColor<GLuint,3> >;
template class ValueCoder<GLColor<GLfloat,3> >;
template class ValueCoder<GLColor<GLdouble,3> >;

template class ValueCoder<GLColor<GLbyte,4> >;
template class ValueCoder<GLColor<GLubyte,4> >;
template class ValueCoder<GLColor<GLint,4> >;
template class ValueCoder<GLColor<GLuint,4> >;
template class ValueCoder<GLColor<GLfloat,4> >;
template class ValueCoder<GLColor<GLdouble,4> >;

template class ValueCoder<GLVector<GLshort,2> >;
template class ValueCoder<GLVector<GLint,2> >;
template class ValueCoder<GLVector<GLfloat,2> >;
template class ValueCoder<GLVector<GLdouble,2> >;

template class ValueCoder<GLVector<GLshort,3> >;
template class ValueCoder<GLVector<GLint,3> >;
template class ValueCoder<GLVector<GLfloat,3> >;
template class ValueCoder<GLVector<GLdouble,3> >;

template class ValueCoder<GLVector<GLshort,4> >;
template class ValueCoder<GLVector<GLint,4> >;
template class ValueCoder<GLVector<GLfloat,4> >;
template class ValueCoder<GLVector<GLdouble,4> >;

template class ValueCoder<GLBox<GLshort,2> >;
template class ValueCoder<GLBox<GLint,2> >;
template class ValueCoder<GLBox<GLfloat,2> >;
template class ValueCoder<GLBox<GLdouble,2> >;

template class ValueCoder<GLBox<GLshort,3> >;
template class ValueCoder<GLBox<GLint,3> >;
template class ValueCoder<GLBox<GLfloat,3> >;
template class ValueCoder<GLBox<GLdouble,3> >;

template class ValueCoder<GLBox<GLshort,4> >;
template class ValueCoder<GLBox<GLint,4> >;
template class ValueCoder<GLBox<GLfloat,4> >;
template class ValueCoder<GLBox<GLdouble,4> >;

}
