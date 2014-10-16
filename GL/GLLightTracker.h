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

#ifndef GLLIGHTTRACKER_INCLUDED
#define GLLIGHTTRACKER_INCLUDED

#include <string>
#include <GL/gl.h>

/* Forward declarations: */
struct GLLight;

class GLLightTracker
	{
	/* Embedded classes: */
	public:
	class LightState // State of an OpenGL light source
		{
		friend class GLLightTracker;
		
		/* Elements: */
		private:
		bool enabled; // Flag whether this light source is enabled
		bool attenuated; // Flag whether this light source uses non-constant attenuation
		bool spotLight; // Flag whether this light source is a spot light
		
		/* Methods: */
		public:
		bool isEnabled(void) const // Returns true if the light source is enabled
			{
			return enabled;
			}
		bool isAttenuated(void) const // Returns true if the light source uses non-constant attenuation
			{
			return attenuated;
			}
		bool isSpotLight(void) const // Returns true if the light source has an anisotropic brightness distribution
			{
			return spotLight;
			}
		};
	
	enum NormalScalingMode // Enumerated type for the three normal vector scaling modes
		{
		NormalScalingOff,
		NormalScalingRescale,
		NormalScalingNormalize
		};
	
	/* Elements: */
	private:
	static const char accumulateLightTemplate[]; // GLSL function template to illuminate with a standard light source
	static const char accumulateAttenuatedLightTemplate[]; // GLSL function template to illuminate with an attenuated light source
	static const char accumulateSpotLightTemplate[]; // GLSL function template to illuminate with a spot light source
	static const char accumulateAttenuatedSpotLightTemplate[]; // GLSL function template to illuminate with an attenuated spot light source
	unsigned int version; // Number incremented each time the tracker detects a change to OpenGL's lighting state
	bool lightingEnabled; // Flag whether lighting is currently enabled
	int maxNumLights; // Maximum number of light sources supported by this OpenGL context
	LightState* lightStates; // Array of OpenGL light source states
	bool specularColorSeparate; // Flag whether specular colors are interpolated separately from diffuse colors
	bool lightingTwoSided; // Flag whether two-sided lighting is enabled
	bool colorMaterials; // Flag whether material properties track the current color
	GLenum colorMaterialFace; // Denotes which polygon face(s) track the current color
	GLenum colorMaterialProperty; // Denotes which material property or properties track the current color
	bool haveRescaleNormal; // Flag whether the OpenGL context supports the GL_EXT_rescale_normal extension
	NormalScalingMode normalScalingMode; // Selected normal scaling mode
	
	/* Constructors and destructors: */
	public:
	GLLightTracker(void); // Creates a light source tracker for the current OpenGL context
	~GLLightTracker(void); // Destroys the light source tracker
	
	/* Methods: */
	unsigned int getVersion(void) const // Returns the version number of the current lighting state
		{
		return version;
		}
	
	/* Methods to query current lighting state: */
	bool isLightingEnabled(void) const // Returns true if lighting is currently enabled
		{
		return lightingEnabled;
		}
	int getMaxNumLights(void) const // Returns the maximum number of light sources supported by the OpenGL context
		{
		return maxNumLights;
		}
	const LightState& getLightState(int lightIndex) const // Returns the current light state structure of the given light source
		{
		return lightStates[lightIndex];
		}
	bool isSpecularColorSeparate(void) const // Returns true if specular colors are interpolated separately from diffuse colors
		{
		return specularColorSeparate;
		}
	bool isLightingTwoSided(void) const // Returns true if both front- and back-facing polygons are illuminated
		{
		return lightingTwoSided;
		}
	bool isColorMaterials(void) const // Returns true if material properties track the current color
		{
		return colorMaterials;
		}
	GLenum getColorMaterialFace(void) const // Returns which polygon face(s) track the current color
		{
		return colorMaterialFace;
		}
	GLenum getColorMaterialProperty(void) const // Returns which material property or properties track the current color
		{
		return colorMaterialProperty;
		}
	NormalScalingMode getNormalScalingMode(void) const // Returns the selected normal vector scaling mode
		{
		return normalScalingMode;
		}
	
	/* Methods to update OpenGL lighting state; return true if current lighting state changed due to provided values: */
	bool setLightingEnabled(bool newLightingEnabled); // Sets global lighting switch
	bool enableLight(int lightIndex,const GLLight& light); // Enables and sets the given light source in the current OpenGL context
	bool disableLight(int lightIndex); // Disables the given light source in the current OpenGL context
	bool setSpecularColorSeparate(bool newSpecularColorSeparate); // Sets separate specular color flag
	bool setLightingTwoSided(bool newLightingTwoSided); // Sets two-sided lighting flag
	bool setColorMaterials(bool newColorMaterials); // Sets color materials tracking flag
	bool setColorMaterialFace(GLenum newColorMaterialFace); // Sets which polygon face(s) track the current color
	bool setColorMaterialProperty(GLenum newColorMaterialProperty); // Sets which material property or properties track the current color
	bool setColorMaterial(GLenum newColorMaterialFace,GLenum newColorMaterialProperty); // Sets both color material settings at once
	bool setNormalScalingMode(NormalScalingMode newNormalScalingMode); // Sets the normal vector scaling mode
	
	bool update(void); // Updates the tracker by querying the current OpenGL context's state; returns true if state has changed since last check
	std::string createAccumulateLightFunction(int lightIndex) const; // Creates a GLSL function to accumulate the contribution of the given light source on a surfel
	
	/*********************************************************************
	The generated GLSL function has the following signature:
	
	void accumulateLight<index>(
		const vec4 vertexEc,      <- Vertex position in eye coordinates
		const vec3 normalEc,      <- Vertex normal vector in eye coordinates
		const vec4 ambient,       <- Ambient material color
		const vec4 diffuse,       <- Diffuse material color
		const vec4 specular,      <- Specular material color
		const float shininess,    <- Specular material shininess
		vec4 ambientDiffuseAccum, <- Accumulator for ambient+diffuse color
		vec4 specularAccum        <- Accumulator for specular color
		);
	
	<index> in the function name is replaced with the ASCII representation
	of the lightIndex parameter passed to createAccumulateLightFunction.
	
	The function accumulates ambient+diffuse and specular colors
	separately to work when GL_SEPARATE_SPECULAR_COLOR is enabled.
	*********************************************************************/
	};

#endif
