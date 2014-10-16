/***********************************************************************
GLLightTracker - Class to keep track of changes to OpenGL's light source
state to support just-in-time compilation of GLSL shaders depending on
the OpenGL context's current lighting state.
Copyright (c) 2011-2012 Oliver Kreylos

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

#include <GL/GLLightTracker.h>

#include <Misc/PrintInteger.h>
#include <GL/gl.h>
#include <GL/GLLightTemplates.h>
#include <GL/GLLight.h>
#include <GL/Extensions/GLEXTRescaleNormal.h>

/***************************************
Static elements of class GLLightTracker:
***************************************/

const char GLLightTracker::accumulateLightTemplate[]=
	"\
	void accumulateLight<lightIndex>(in vec4 vertexEc,in vec3 normalEc,in vec4 ambient,in vec4 diffuse,in vec4 specular,in float shininess,inout vec4 ambientDiffuseAccum,inout vec4 specularAccum)\n\
		{\n\
		/* Compute the light direction (works both for directional and point lights): */\n\
		vec3 lightDirEc=gl_LightSource[<lightIndex>].position.xyz*vertexEc.w-vertexEc.xyz*gl_LightSource[<lightIndex>].position.w;\n\
		lightDirEc=normalize(lightDirEc);\n\
		\n\
		/* Calculate per-source ambient light term: */\n\
		ambientDiffuseAccum+=gl_LightSource[<lightIndex>].ambient*ambient;\n\
		\n\
		/* Compute the diffuse lighting angle: */\n\
		float nl=dot(normalEc,lightDirEc);\n\
		if(nl>0.0)\n\
			{\n\
			/* Calculate per-source diffuse light term: */\n\
			ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].diffuse*diffuse)*nl;\n\
			\n\
			/* Compute the eye direction: */\n\
			vec3 eyeDirEc=normalize(-vertexEc.xyz);\n\
			\n\
			/* Compute the specular lighting angle: */\n\
			float nhv=max(dot(normalEc,normalize(eyeDirEc+lightDirEc)),0.0);\n\
			\n\
			/* Calculate per-source specular lighting term: */\n\
			specularAccum+=(gl_LightSource[<lightIndex>].specular*specular)*pow(nhv,shininess);\n\
			}\n\
		}\n\
	\n";

const char GLLightTracker::accumulateAttenuatedLightTemplate[]=
	"\
	void accumulateLight<lightIndex>(in vec4 vertexEc,in vec3 normalEc,in vec4 ambient,in vec4 diffuse,in vec4 specular,in float shininess,inout vec4 ambientDiffuseAccum,inout vec4 specularAccum)\n\
		{\n\
		/* Compute the light direction (works both for directional and point lights): */\n\
		vec3 lightDirEc=gl_LightSource[<lightIndex>].position.xyz*vertexEc.w-vertexEc.xyz*gl_LightSource[<lightIndex>].position.w;\n\
		float lightDist=length(lightDirEc);\n\
		lightDirEc=normalize(lightDirEc);\n\
		\n\
		/* Calculate the source attenuation factor: */\n\
		float att=1.0/((gl_LightSource[<lightIndex>].quadraticAttenuation*lightDist+gl_LightSource[<lightIndex>].linearAttenuation)*lightDist+gl_LightSource[<lightIndex>].constantAttenuation);\n\
		\n\
		/* Calculate per-source ambient light term: */\n\
		ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].ambient*ambient)*att;\n\
		\n\
		/* Compute the diffuse lighting angle: */\n\
		float nl=dot(normalEc,lightDirEc);\n\
		if(nl>0.0)\n\
			{\n\
			\n\
			/* Calculate per-source diffuse light term: */\n\
			ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].diffuse*diffuse)*(nl*att);\n\
			\n\
			/* Compute the eye direction: */\n\
			vec3 eyeDirEc=normalize(-vertexEc.xyz);\n\
			\n\
			/* Compute the specular lighting angle: */\n\
			float nhv=max(dot(normalEc,normalize(eyeDirEc+lightDirEc)),0.0);\n\
			\n\
			/* Calculate per-source specular lighting term: */\n\
			specularAccum+=(gl_LightSource[<lightIndex>].specular*specular)*(pow(nhv,shininess)*att);\n\
			}\n\
		}\n\
	\n";

const char GLLightTracker::accumulateSpotLightTemplate[]=
	"\
	void accumulateLight<lightIndex>(in vec4 vertexEc,in vec3 normalEc,in vec4 ambient,in vec4 diffuse,in vec4 specular,in float shininess,inout vec4 ambientDiffuseAccum,inout vec4 specularAccum)\n\
		{\n\
		/* Compute the light direction (works both for directional and point lights): */\n\
		vec3 lightDirEc=gl_LightSource[<lightIndex>].position.xyz*vertexEc.w-vertexEc.xyz*gl_LightSource[<lightIndex>].position.w;\n\
		lightDirEc=normalize(lightDirEc);\n\
		\n\
		/* Calculate the spot light angle: */\n\
		float sl=-dot(lightDirEc,normalize(gl_LightSource[<lightIndex>].spotDirection));\n\
		\n\
		/* Check if the point is inside the spot light's cone: */\n\
		if(sl>=gl_LightSource[<lightIndex>].spotCosCutoff)\n\
			{\n\
			/* Calculate the spot light attenuation factor: */\n\
			float att=pow(sl,gl_LightSource[<lightIndex>].spotExponent);\n\
			\n\
			/* Calculate per-source ambient light term: */\n\
			ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].ambient*ambient)*att;\n\
			\n\
			/* Compute the diffuse lighting angle: */\n\
			float nl=dot(normalEc,lightDirEc);\n\
			if(nl>0.0)\n\
				{\n\
				/* Calculate per-source diffuse light term: */\n\
				ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].diffuse*diffuse)*(nl*att);\n\
				\n\
				/* Compute the eye direction: */\n\
				vec3 eyeDirEc=normalize(-vertexEc.xyz);\n\
				\n\
				/* Compute the specular lighting angle: */\n\
				float nhv=max(dot(normalEc,normalize(eyeDirEc+lightDirEc)),0.0);\n\
				\n\
				/* Calculate per-source specular lighting term: */\n\
				specularAccum+=(gl_LightSource[<lightIndex>].specular*specular)*(pow(nhv,shininess)*att);\n\
				}\n\
			}\n\
		}\n\
	\n";

const char GLLightTracker::accumulateAttenuatedSpotLightTemplate[]=
	"\
	void accumulateLight<lightIndex>(in vec4 vertexEc,in vec3 normalEc,in vec4 ambient,in vec4 diffuse,in vec4 specular,in float shininess,inout vec4 ambientDiffuseAccum,inout vec4 specularAccum)\n\
		{\n\
		/* Compute the light direction (works both for directional and point lights): */\n\
		vec3 lightDirEc=gl_LightSource[<lightIndex>].position.xyz*vertexEc.w-vertexEc.xyz*gl_LightSource[<lightIndex>].position.w;\n\
		float lightDist=length(lightDirEc);\n\
		lightDirEc=normalize(lightDirEc);\n\
		\n\
		/* Calculate the spot light angle: */\n\
		float sl=-dot(lightDirEc,normalize(gl_LightSource[<lightIndex>].spotDirection))\n\
		\n\
		/* Check if the point is inside the spot light's cone: */\n\
		if(sl>=gl_LightSource[<lightIndex>].spotCosCutoff)\n\
			{\n\
			/* Calculate the source attenuation factor: */\n\
			float att=1.0/((gl_LightSource[<lightIndex>].quadraticAttenuation*lightDist+gl_LightSource[<lightIndex>].linearAttenuation)*lightDist+gl_LightSource[<lightIndex>].constantAttenuation);\n\
			\n\
			/* Calculate the spot light attenuation factor: */\n\
			att*=pow(sl,gl_LightSource[<lightIndex>].spotExponent);\n\
			\n\
			/* Calculate per-source ambient light term: */\n\
			ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].ambient*ambient)*att;\n\
			\n\
			/* Compute the diffuse lighting angle: */\n\
			float nl=dot(normalEc,lightDirEc);\n\
			if(nl>0.0)\n\
				{\n\
				/* Calculate per-source diffuse light term: */\n\
				ambientDiffuseAccum+=(gl_LightSource[<lightIndex>].diffuse*diffuse)*(nl*att);\n\
				\n\
				/* Compute the eye direction: */\n\
				vec3 eyeDirEc=normalize(-vertexEc.xyz);\n\
				\n\
				/* Compute the specular lighting angle: */\n\
				float nhv=max(dot(normalEc,normalize(eyeDirEc+lightDirEc)),0.0);\n\
				\n\
				/* Calculate per-source specular lighting term: */\n\
				specularAccum+=(gl_LightSource[<lightIndex>].specular*specular)*(pow(nhv,shininess)*att);\n\
				}\n\
			}\n\
		}\n\
	\n";

/*******************************
Methods of class GLLightTracker:
*******************************/

GLLightTracker::GLLightTracker(void)
	:version(0),
	 lightingEnabled(false),
	 maxNumLights(0),lightStates(0),
	 specularColorSeparate(false),
	 lightingTwoSided(false),
	 colorMaterials(false),
	 colorMaterialFace(GL_FRONT_AND_BACK),colorMaterialProperty(GL_AMBIENT_AND_DIFFUSE),
	 haveRescaleNormal(GLEXTRescaleNormal::isSupported()),normalScalingMode(NormalScalingOff)
	{
	/* Determine the maximum number of light sources supported by the local OpenGL: */
	glGetIntegerv(GL_MAX_LIGHTS,&maxNumLights);
	
	/* Initialize the light state array: */
	lightStates=new LightState[maxNumLights];
	for(int i=0;i<maxNumLights;++i)
		{
		lightStates[i].enabled=i==0;
		lightStates[i].attenuated=false;
		lightStates[i].spotLight=false;
		}
	
	/* Initialize the GL_EXT_rescale_normal extension: */
	if(haveRescaleNormal)
		GLEXTRescaleNormal::initExtension();
	
	/* Query the current lighting state: */
	update();
	
	/* Reset the version number to one, even if there was no change: */
	version=1;
	}

GLLightTracker::~GLLightTracker(void)
	{
	/* Destroy the light state array: */
	delete[] lightStates;
	}

bool GLLightTracker::setLightingEnabled(bool newLightingEnabled)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=lightingEnabled!=newLightingEnabled;
	lightingEnabled=newLightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	if(lightingEnabled)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	
	return changed;
	}

bool GLLightTracker::enableLight(int lightIndex,const GLLight& light)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=false;
	
	/* Check enabled state: */
	changed=changed||!lightStates[lightIndex].enabled;
	lightStates[lightIndex].enabled=true;
	
	/* Check attenuation state: */
	bool lightAttenuated=light.position[3]!=GLLight::Scalar(0)&&(light.constantAttenuation!=GLLight::Scalar(1)||light.linearAttenuation!=GLLight::Scalar(0)||light.quadraticAttenuation!=GLLight::Scalar(0));
	changed=changed||lightStates[lightIndex].attenuated!=lightAttenuated;
	lightStates[lightIndex].attenuated=lightAttenuated;
	
	/* Check spotlight state: */
	bool lightSpotLight=light.position[3]!=GLLight::Scalar(0)&&light.spotCutoff<=GLLight::Scalar(90);
	changed=changed||lightStates[lightIndex].spotLight!=lightSpotLight;
	lightStates[lightIndex].spotLight=lightSpotLight;
	
	/* Reset changed flag if lighting is disabled globally: */
	changed=changed&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the light source change through to OpenGL: */
	glEnableLight(lightIndex);
	glLight(lightIndex,light);
	
	return changed;
	}

bool GLLightTracker::disableLight(int lightIndex)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=false;
	
	/* Check enabled state: */
	changed=changed||lightStates[lightIndex].enabled;
	lightStates[lightIndex].enabled=false;
	
	/* Reset changed flag if lighting is disabled globally: */
	changed=changed&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the light source change through to OpenGL: */
	glDisableLight(lightIndex);
	
	return changed;
	}

bool GLLightTracker::setSpecularColorSeparate(bool newSpecularColorSeparate)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=specularColorSeparate!=newSpecularColorSeparate;
	specularColorSeparate=newSpecularColorSeparate;
	
	/* Reset changed flag if lighting is disabled globally: */
	changed=changed&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,specularColorSeparate?GL_SEPARATE_SPECULAR_COLOR:GL_SINGLE_COLOR);
	
	return changed;
	}

bool GLLightTracker::setLightingTwoSided(bool newLightingTwoSided)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=lightingTwoSided!=newLightingTwoSided;
	lightingTwoSided=newLightingTwoSided;
	
	/* Reset changed flag if lighting is disabled globally: */
	changed=changed&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,lightingTwoSided?GL_TRUE:GL_FALSE);
	
	return changed;
	}

bool GLLightTracker::setColorMaterials(bool newColorMaterials)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=colorMaterials!=newColorMaterials;
	colorMaterials=newColorMaterials;
	
	/* Reset changed flag if lighting is disabled globally: */
	changed=changed&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	if(colorMaterials)
		glEnable(GL_COLOR_MATERIAL);
	else
		glDisable(GL_COLOR_MATERIAL);
	
	return changed;
	}

bool GLLightTracker::setColorMaterialFace(GLenum newColorMaterialFace)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=colorMaterialFace!=newColorMaterialFace;
	colorMaterialFace=newColorMaterialFace;
	
	/* Reset changed flag if color materials or lighting are disabled globally: */
	changed=changed&&colorMaterials&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	glColorMaterial(colorMaterialFace,colorMaterialProperty);
	
	return changed;
	}

bool GLLightTracker::setColorMaterialProperty(GLenum newColorMaterialProperty)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=colorMaterialProperty!=newColorMaterialProperty;
	colorMaterialProperty=newColorMaterialProperty;
	
	/* Reset changed flag if color materials or lighting are disabled globally: */
	changed=changed&&colorMaterials&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	glColorMaterial(colorMaterialFace,colorMaterialProperty);
	
	return changed;
	}

bool GLLightTracker::setColorMaterial(GLenum newColorMaterialFace,GLenum newColorMaterialProperty)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=false;
	
	/* Check color material face: */
	changed=changed||colorMaterialFace!=newColorMaterialFace;
	colorMaterialFace=newColorMaterialFace;
	
	/* Check color material property: */
	changed=changed||colorMaterialProperty!=newColorMaterialProperty;
	colorMaterialProperty=newColorMaterialProperty;
	
	/* Reset changed flag if color materials or lighting are disabled globally: */
	changed=changed&&colorMaterials&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	glColorMaterial(colorMaterialFace,colorMaterialProperty);
	
	return changed;
	}

bool GLLightTracker::setNormalScalingMode(GLLightTracker::NormalScalingMode newNormalScalingMode)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=normalScalingMode!=newNormalScalingMode;
	normalScalingMode=newNormalScalingMode;
	
	/* Reset changed flag if lighting is disabled globally: */
	changed=changed&&lightingEnabled;
	
	/* Update the lighting state version number if anything changed: */
	if(changed)
		++version;
	
	/* Pass the state change through to OpenGL: */
	switch(normalScalingMode)
		{
		case NormalScalingOff:
			glDisable(GL_NORMALIZE);
			if(haveRescaleNormal)
				glDisable(GL_RESCALE_NORMAL_EXT);
			break;
		
		case NormalScalingRescale:
			if(haveRescaleNormal)
				{
				glDisable(GL_NORMALIZE);
				glEnable(GL_RESCALE_NORMAL_EXT);
				}
			else
				{
				/* Fall back to normalize mode: */
				glEnable(GL_NORMALIZE);
				}
			break;
		
		case NormalScalingNormalize:
			glEnable(GL_NORMALIZE);
			if(haveRescaleNormal)
				glDisable(GL_RESCALE_NORMAL_EXT);
			break;
		}
	
	return changed;
	}

bool GLLightTracker::update(void)
	{
	/* Let the tracked lighting state reflect the new setting and remember whether lighting state changed: */
	bool changed=false;
	
	/* Check the lighting master switch: */
	bool newLightingEnabled=glIsEnabled(GL_LIGHTING);
	changed=changed||lightingEnabled!=newLightingEnabled;
	lightingEnabled=newLightingEnabled;
	
	if(lightingEnabled)
		{
		/* Check all light source states: */
		for(int lightIndex=0;lightIndex<maxNumLights;++lightIndex)
			{
			GLenum light=GL_LIGHT0+lightIndex;
			
			/* Get the light's enabled flag: */
			bool enabled=glIsEnabled(light);
			changed=changed||lightStates[lightIndex].enabled!=enabled;
			lightStates[lightIndex].enabled=enabled;
			
			if(enabled)
				{
				/* Determine the light's attenuation and spot light state: */
				bool attenuated=false;
				bool spotLight=false;
				
				/* Get the light's position: */
				GLfloat pos[4];
				glGetLightfv(light,GL_POSITION,pos);
				if(pos[3]!=0.0f)
					{
					/* Get the light's attenuation coefficients: */
					GLfloat att[3];
					glGetLightfv(light,GL_CONSTANT_ATTENUATION,&att[0]);
					glGetLightfv(light,GL_LINEAR_ATTENUATION,&att[1]);
					glGetLightfv(light,GL_QUADRATIC_ATTENUATION,&att[2]);
					
					/* Determine whether the light is attenuated: */
					attenuated=att[0]!=1.0f||att[1]!=0.0f||att[2]!=0.0f;
					
					/* Get the light's spot light cutoff angle: */
					GLfloat spotLightCutoff;
					glGetLightfv(light,GL_SPOT_CUTOFF,&spotLightCutoff);
					spotLight=spotLightCutoff<=90.0f;
					}
				
				changed=changed||lightStates[lightIndex].attenuated!=attenuated||lightStates[lightIndex].spotLight!=spotLight;
				lightStates[lightIndex].attenuated=attenuated;
				lightStates[lightIndex].spotLight=spotLight;
				}
			}
		
		/* Check for separate specular color: */
		GLint lightModelColorControl;
		glGetIntegerv(GL_LIGHT_MODEL_COLOR_CONTROL,&lightModelColorControl);
		bool newSpecularColorSeparate=lightModelColorControl==GL_SEPARATE_SPECULAR_COLOR;
		changed=changed||specularColorSeparate!=newSpecularColorSeparate;
		specularColorSeparate=newSpecularColorSeparate;
		
		/* Check the two-sided lighting flag: */
		GLint lightModelTwoSide;
		glGetIntegerv(GL_LIGHT_MODEL_TWO_SIDE,&lightModelTwoSide);
		bool newLightingTwoSided=lightModelTwoSide!=0;
		changed=changed||lightingTwoSided!=newLightingTwoSided;
		lightingTwoSided=newLightingTwoSided;
		
		/* Check the color material flag: */
		bool newColorMaterials=glIsEnabled(GL_COLOR_MATERIAL);
		changed=changed||colorMaterials!=newColorMaterials;
		colorMaterials=newColorMaterials;
		
		if(colorMaterials)
			{
			/* Get the color material face: */
			GLint newColorMaterialFace;
			glGetIntegerv(GL_COLOR_MATERIAL_FACE,&newColorMaterialFace);
			changed=changed||colorMaterialFace!=GLenum(newColorMaterialFace);
			colorMaterialFace=GLenum(newColorMaterialFace);
			
			/* Get the color material property: */
			GLint newColorMaterialProperty;
			glGetIntegerv(GL_COLOR_MATERIAL_PARAMETER,&newColorMaterialProperty);
			changed=changed||colorMaterialProperty!=GLenum(newColorMaterialProperty);
			colorMaterialProperty=GLenum(newColorMaterialProperty);
			}
		
		/* Check the normal rescaling mode: */
		NormalScalingMode newNormalScalingMode;
		if(glIsEnabled(GL_NORMALIZE))
			newNormalScalingMode=NormalScalingNormalize;
		else if(haveRescaleNormal&&glIsEnabled(GL_RESCALE_NORMAL_EXT))
			newNormalScalingMode=NormalScalingRescale;
		else
			newNormalScalingMode=NormalScalingOff;
		changed=changed||normalScalingMode!=newNormalScalingMode;
		normalScalingMode=newNormalScalingMode;
		}
	
	/* Update the version number if there was a change: */
	if(changed)
		++version;
	
	return changed;
	}

std::string GLLightTracker::createAccumulateLightFunction(int lightIndex) const
	{
	std::string result;
	
	/* Create the light index string: */
	char liBuffer[12];
	char* liString=Misc::print(lightIndex,liBuffer+11);
	
	/* Find the appropriate function template string: */
	const char* functionTemplate;
	if(lightStates[lightIndex].attenuated&&lightStates[lightIndex].spotLight)
		functionTemplate=accumulateAttenuatedSpotLightTemplate;
	else if(lightStates[lightIndex].spotLight)
		functionTemplate=accumulateSpotLightTemplate;
	else if(lightStates[lightIndex].attenuated)
		functionTemplate=accumulateAttenuatedLightTemplate;
	else
		functionTemplate=accumulateLightTemplate;
	
	/* Replace all occurrences of <lightIndex> in the template string with the light index string: */
	const char* match="<lightIndex>";
	const char* matchStart=0;
	int matchLen=0;
	for(const char* tPtr=functionTemplate;*tPtr!='\0';++tPtr)
		{
		if(matchLen==0)
			{
			if(*tPtr=='<')
				{
				matchStart=tPtr;
				matchLen=1;
				}
			else
				result.push_back(*tPtr);
			}
		else if(matchLen<12)
			{
			if(*tPtr==match[matchLen])
				{
				++matchLen;
				if(matchLen==12)
					{
					result.append(liString);
					matchLen=0;
					}
				}
			else
				{
				for(const char* cPtr=matchStart;cPtr!=tPtr;++cPtr)
					result.push_back(*cPtr);
				matchLen=0;
				--tPtr;
				}
			}
		}
	
	return result;
	}
