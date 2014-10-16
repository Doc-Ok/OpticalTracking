/***********************************************************************
LensCorrector - Helper class to render imagery into an off-screen buffer
and then warp the buffer to the final drawable to correct subsequent
lens distortion.
Copyright (c) 2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef VRUI_INTERNAL_LENSCORRECTOR_INCLUDED
#define VRUI_INTERNAL_LENSCORRECTOR_INCLUDED

#include <Geometry/ComponentArray.h>
#include <Geometry/Point.h>
#include <GL/gl.h>
#include <GL/GLShader.h>
#include <GL/GLWindow.h>
#include <Vrui/Geometry.h>

/* Forward declarations: */
namespace Misc {
class ConfigurationFileSection;
}
namespace Vrui {
class WindowProperties;
class DisplayState;
class VRWindow;
}

namespace Vrui {

class LensCorrector
	{
	/* Embedded classes: */
	public:
	class DistortionEquation // Base class for lens distortion equations
		{
		/* Constructors and destructors: */
		public:
		virtual ~DistortionEquation(void);
		
		/* Methods: */
		virtual double getMaxR2(void) const =0; // Returns the maximum value of r2 for which the equation is well-defined
		virtual double operator()(double r2) const =0; // Evaluates the distortion equation for parameter r2
		virtual double d(double r2) const =0; // Evaluates the distortion equation's derivative for parameter r2
		virtual double invert(double rp) const; // Inverts a distortion equation using Newton-Raphson iteration
		};
	
	private:
	typedef Geometry::Point<double,2> Point2; // Type for 2D points
	typedef Geometry::ComponentArray<double,2> Scale2; // Type for 2D scaling factors
	
	struct LensConfig // Structure holding configuration data for one eye<->lens<->half-screen combination
		{
		/* Elements: */
		public:
		Point lensCenter; // Center point of lens relative to screen, in physical units
		DistortionEquation* distortionEquations[3]; // Lens distortion equations for the green, red, and blue components
		Point2 center; // Radial distortion centers in viewport coordinates [0, 1] x [0, 1]
		Scale2 preScale; // Scale factors from viewport coordinates to radial coordinates
		double overscan[4]; // Overscan margins on left, right, bottom, and top edges to account for increased FOV due to the lens's magnification
		Scale2 overscanSize; // Total horizontal and vertical overscan
		GLint finalViewport[4]; // Viewport position and size of the distortion-corrected image in the final drawable
		};
	
	struct WarpMeshVertex // Structure for warping mesh vertices with three texture coordinates for red, green, and blue
		{
		/* Elements: */
		public:
		GLfloat redTex[2];
		GLfloat greenTex[2];
		GLfloat blueTex[2];
		GLfloat pos[2];
		};
	
	/* Elements: */
	private:
	LensConfig lensConfigs[2]; // Lens configurations for left and right eyes
	int predistortionFrameSize[2]; // Width and height of pre-distortion frame buffer combining final drawable size and overscan
	int predistortionMultisamplingLevel; // Multisampling level in the pre-distortion frame buffer
	int predistortionStencilBufferSize; // Bit depth of the optional pre-distortion stencil buffer
	bool warpCubicLookup; // Flag whether to use bicubic interpolation instead of bilinear for texture look-up in the warping shader
	GLint finalViewport[4]; // Viewport position and size covering the entire final drawable
	GLuint predistortionFrameBufferId; // ID of the pre-distortion frame buffer
	GLuint predistortionColorBufferIds[2]; // IDs of the left and right pre-distortion color image textures; double-buffering used for OLED response time correction
	GLuint predistortionMultisamplingColorBufferId; // ID of the shared pre-distortion multisampling color buffer
	GLuint predistortionDepthStencilBufferId; // ID of the pre-distortion depth buffer, potentially interleaved with a stencil buffer
	GLuint multisamplingFrameBufferId; // ID of a frame buffer to "fix" a multisampled image texture into a regular image texture
	
	int warpMeshSize[2]; // Number of vertices in the left and right warping quad meshes
	GLuint warpMeshVertexBufferIds[2]; // IDs of the left and right warping quad mesh vertex buffers
	GLuint warpMeshIndexBufferId; // ID of the shared warping quad mesh index buffer
	
	GLShader warpingShader; // GLSL shader to warp the pre-distortion color image buffers into the final drawable
	int warpingShaderAttributeIndices[3]; // Attribute indices of the warping shader's attribute variables
	int warpingShaderUniformIndices[4]; // Locations of the warping shader's uniform variables
	
	bool correctOledResponse; // Flag to apply inter-frame pixel brightness correction to account for asymmetric OLED pixel response curves
	GLfloat oledCorrectionFactors[2]; // Correction factors for diminishing and increasing OLED pixel brightness
	GLfloat oledContrast[2]; // Scale and offset for pre-OLED correction contrast reduction
	bool fixContrast; // Flag to reduce image contrast prior to applying OLED brightness correction
	GLuint previousFrameTextureId; // ID of texture containing the previously rendered frame, for OLED brightness correction
	
	/* Constructors and destructors: */
	public:
	LensCorrector(const VRWindow& window,const WindowProperties& windowProperties,int multisamplingLevel,const GLWindow::WindowPos viewportPos[2],const Misc::ConfigurationFileSection& configFileSection); // Creates a lens corrector by reading from the given configuration file section
	~LensCorrector(void); // Destroys the lens corrector
	
	/* Methods: */
	const int* getPredistortionFrameSize(void) const // Returns the size of the overscanned and super-sampled pre-distortion image buffer
		{
		return predistortionFrameSize;
		}
	void prepare(int eye,DisplayState& displayState) const; // Prepares for rendering of the pre-distortion image for the given eye and adjusts the given display state object
	void adjustProjection(int eye,const Point& screenEyePos,double near,double& left,double& right,double& bottom,double& top) const; // Adjusts the projection matrix to accound for overscan
	void finish(int eye) const; // Finishes up after the pre-distortion image for the given eye has been rendered
	void warp(void) const; // Warps the previously rendered left and right eye pre-distortion images into the final drawable
	};

}

#endif
