/***********************************************************************
GLLineLightingShader - Automatic shader class for Phong illumination of
lines with tangent vectors.
Copyright (c) 2012-2013 Oliver Kreylos

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

#include <GL/GLLineLightingShader.h>

#include <string>
#include <Misc/PrintInteger.h>
#include <GL/gl.h>
#include <GL/GLLightTracker.h>
#include <GL/GLClipPlaneTracker.h>
#include <GL/GLContextData.h>

/*********************************************
Static elements of class GLLineLightingShader:
*********************************************/

const char GLLineLightingShader::accumulateLightTemplate[]="\
	void accumulateLight<lightIndex>(in vec4 vertexEc,in mat3 tangentSpace,in vec3 eyeDir,in vec4 ambient,in vec4 diffuse, in vec4 specular, in float shininess,inout vec4 ambientDiffuseAccum,inout vec4 specularAccum)\n\
		{\n\
		/* Compute the light direction in eye space (works both for directional and point lights): */\n\
		vec3 lightDirEc=gl_LightSource[<lightIndex>].position.xyz*vertexEc.w-vertexEc.xyz*gl_LightSource[<lightIndex>].position.w;\n\
		\n\
		/* Accumulate the ambient light term: */\n\
		ambientDiffuseAccum+=gl_LightSource[<lightIndex>].ambient*ambient;\n\
		\n\
		/* Transform the light direction to tangent space: */\n\
		vec3 lightDir=normalize(tangentSpace*lightDirEc);\n\
		\n\
		/* Calculate the diffuse lighting coefficient: */\n\
		float acosArg=clamp(lightDir.z*inversesqrt(lightDir.y*lightDir.y+lightDir.z*lightDir.z),-1.0,1.0);\n\
		float d=(abs(lightDir.y)+(3.141592654-acos(acosArg))*lightDir.z)*0.25;\n\
		\n\
		/* Add the diffuse lighting term: */\n\
		ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].diffuse*diffuse)*d;\n\
		\n\
		/* Calculate the light half-vector: */\n\
		vec3 half=normalize(lightDir+eyeDir);\n\
		\n\
		/* Calculate the specular lighting term via numerical integration: */\n\
		float s=0.0;\n\
		for(int i=-90;i<90;i+=20)\n\
			{\n\
			/* Calculate the surface normal: */\n\
			float normalAngle=radians(float(i)+10.0);\n\
			vec3 normal=vec3(0.0,sin(normalAngle),cos(normalAngle));\n\
			\n\
			/* Evaluate the specular reflection term: */\n\
			float ln=dot(lightDir,normal);\n\
			if(ln>0.0)\n\
				{\n\
				float hn=dot(half,normal);\n\
				if(hn>0.0)\n\
					s+=pow(hn,shininess)*normal.z;\n\
				}\n\
			}\n\
		s*=sqrt(2.0*(1.0-cos(radians(20.0))))*0.5;\n\
		\n\
		/* Add the specular lighting term: */\n\
		specularAccum+=(gl_LightSource[<lightIndex>].specular*specular)*s;\n\
		}\n\
	\n";

const char GLLineLightingShader::fragmentShaderMain[]="\
	void main()\n\
		{\n\
		gl_FragColor=gl_Color;\n\
		}\n";

/*************************************
Methods of class GLLineLightingShader:
*************************************/

void GLLineLightingShader::buildShader(void)
	{
	GLLightTracker* lt=contextData.getLightTracker();
	GLClipPlaneTracker* cpt=contextData.getClipPlaneTracker();
	
	/* Reset the line lighting shader: */
	shader.reset();
	
	/* Start the vertex shader's main function: */
	std::string mainFunction="\
		void main()\n\
			{\n\
			/* Compute the vertex position in eye space: */\n\
			vec4 vertexEc=gl_ModelViewMatrix*gl_Vertex;\n\
			\n\
			/* Calculate the direction vector from the vertex to the eye in eye space: */\n\
			vec3 eyeDirEc=normalize(-vertexEc.xyz);\n\
			\n\
			/* Calculate the tangent-space coordinate frame: */\n\
			mat3 tangentSpace;\n\
			\n\
			/* Compute the tangent vector (which transforms like a vector, not like a normal) in eye space: */\n\
			tangentSpace[0]=normalize((gl_ModelViewMatrix*vec4(gl_Normal,0.0)).xyz);\n\
			tangentSpace[1]=normalize(cross(eyeDirEc,tangentSpace[0]));\n\
			tangentSpace[2]=normalize(cross(tangentSpace[0],tangentSpace[1]));\n\
			tangentSpace=transpose(tangentSpace);\n\
			\n\
			/* Compute the eye direction in tangent space: */\n\
			vec3 eyeDir=tangentSpace*eyeDirEc;\n\
			\n\
			/* Initialize the color accumulators: */\n\
			vec4 ambientDiffuseAccumulator=gl_LightModel.ambient*gl_FrontMaterial.ambient;\n\
			vec4 specularAccumulator=vec4(0.0,0.0,0.0,0.0);\n\
			\n";
	
	/* Call the appropriate light accumulation function for every enabled light source: */
	std::string lightAccumulationFunctions;
	for(int lightIndex=0;lightIndex<lt->getMaxNumLights();++lightIndex)
		if(lt->getLightState(lightIndex).isEnabled())
			{
			char liBuffer[12];
			const char* liString=Misc::print(lightIndex,liBuffer+11);
			
			/* Append a function call to the main function: */
			mainFunction+="accumulateLight";
			mainFunction.append(liString);
			mainFunction+="(vertexEc,tangentSpace,eyeDir,gl_FrontMaterial.ambient,gl_FrontMaterial.diffuse,gl_FrontMaterial.specular,gl_FrontMaterial.shininess,ambientDiffuseAccumulator,specularAccumulator);";
			mainFunction+="\n\n";
			
			/* Process the light function template: */
			const char* match="<lightIndex>";
			const char* matchStart=0;
			int matchLen=0;
			for(const char* tPtr=accumulateLightTemplate;*tPtr!='\0';++tPtr)
				{
				if(matchLen==0)
					{
					if(*tPtr=='<')
						{
						matchStart=tPtr;
						matchLen=1;
						}
					else
						lightAccumulationFunctions.push_back(*tPtr);
					}
				else if(matchLen<12)
					{
					if(*tPtr==match[matchLen])
						{
						++matchLen;
						if(matchLen==12)
							{
							lightAccumulationFunctions.append(liString);
							matchLen=0;
							}
						}
					else
						{
						for(const char* cPtr=matchStart;cPtr!=tPtr;++cPtr)
							lightAccumulationFunctions.push_back(*cPtr);
						matchLen=0;
						--tPtr;
						}
					}
				}
			}
	
	/* Continue the vertex shader's main function: */
	mainFunction+="\
			/* Assign the final accumulated vertex color: */\n\
			gl_FrontColor=ambientDiffuseAccumulator+specularAccumulator;\n\
			\n";
	
	/* Insert code to calculate the vertex' position relative to all user-specified clipping planes: */
	mainFunction+=cpt->createCalcClipDistances("vertexEc");
	
	/* Finish the vertex shader's main function: */
	mainFunction+="\
			\n\
			/* Use standard vertex position: */\n\
			gl_Position=ftransform();\n\
			}\n";
	
	/* Assemble the full vertex shader source: */
	std::string vertexShader=lightAccumulationFunctions+mainFunction;
	
	/* Compile the vertex shader: */
	shader.compileVertexShaderFromString(vertexShader.c_str());
			
	/* Compile the fragment shader: */
	shader.compileFragmentShaderFromString(fragmentShaderMain);
	
	/* Link the shader: */
	shader.linkShader();
	}

GLLineLightingShader::GLLineLightingShader(GLContextData& sContextData)
	:GLAutomaticShader(sContextData),
	 lightTrackerVersion(0),clipPlaneTrackerVersion(0)
	{
	}

GLLineLightingShader::~GLLineLightingShader(void)
	{
	}

void GLLineLightingShader::update(void)
	{
	}

void GLLineLightingShader::activate(void)
	{
	/* Check the lighting state's version number: */
	if(lightTrackerVersion!=contextData.getLightTracker()->getVersion()||clipPlaneTrackerVersion!=contextData.getClipPlaneTracker()->getVersion())
		{
		/* Rebuild the shader: */
		buildShader();
		
		/* Mark the shader as up-to-date: */
		lightTrackerVersion=contextData.getLightTracker()->getVersion();
		clipPlaneTrackerVersion=contextData.getClipPlaneTracker()->getVersion();
		}
	
	/* Activate the shader: */
	shader.useProgram();
	}

void GLLineLightingShader::deactivate(void)
	{
	/* Deactivate the shader: */
	GLShader::disablePrograms();
	}

bool GLLineLightingShader::isSupported(GLContextData& contextData)
	{
	/* Return true if GLSL shaders are supported: */
	return GLShader::isSupported();
	}
