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

#include <Vrui/Internal/LensCorrector.h>

#include <string>
#include <stdexcept>
#include <Misc/PrintInteger.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/FixedArray.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ArrayValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/Extensions/GLARBMultitexture.h>
#include <GL/Extensions/GLARBTextureRectangle.h>
#include <GL/Extensions/GLARBShaderObjects.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <GL/Extensions/GLARBVertexProgram.h>
#include <GL/Extensions/GLEXTFramebufferBlit.h>
#include <GL/Extensions/GLEXTFramebufferObject.h>
#include <GL/Extensions/GLEXTFramebufferMultisample.h>
#include <GL/Extensions/GLEXTPackedDepthStencil.h>
#include <Vrui/Vrui.h>
#include <Vrui/WindowProperties.h>
#include <Vrui/VRScreen.h>
#include <Vrui/VRWindow.h>
#include <Vrui/DisplayState.h>

namespace Vrui {

/**************************************************
Methods of class LensCorrector::DistortionEquation:
**************************************************/

LensCorrector::DistortionEquation::~DistortionEquation(void)
	{
	}

double LensCorrector::DistortionEquation::invert(double rp) const
	{
	/* Run Newton-Raphson iteration to calculate the inverse of dist(): */
	double r2=0.25;
	double maxR2=getMaxR2();
	for(int i=0;i<20;++i)
		{
		/* Calculate distortionEquation(r) and distortionEquation'(r): */
		double dr=(*this)(r2)-rp;
		if(Math::abs(dr)<1.0e-8)
			break;
		double ddr=this->d(r2);
		
		/* Do the step: */
		r2-=dr/ddr;
		if(r2<0.0)
			r2=0.0;
		if(r2>maxR2)
			r2=maxR2;
		}
	
	return r2;
	}

namespace {

/**************
Helper classes:
**************/

class PolynomialDistortionEquation:public LensCorrector::DistortionEquation // Class for polynomial equations
	{
	/* Elements: */
	private:
	unsigned int degree; // Polynomial degree
	double* c; // Array of polynomial coefficients
	
	/* Constructors and destructors: */
	public:
	PolynomialDistortionEquation(unsigned int sDegree,const double* sC =0)
		:degree(sDegree),
		 c(new double[degree+1])
		{
		if(sC!=0)
			{
			for(unsigned int i=0;i<=degree;++i)
				c[i]=sC[i];
			}
		else
			{
			for(unsigned int i=0;i<=degree;++i)
				c[i]=0.0;
			}
		}
	virtual ~PolynomialDistortionEquation(void)
		{
		delete[] c;
		}
	
	/* Methods from LensCorrector::DistortionEquation: */
	virtual double getMaxR2(void) const
		{
		return 1.0;
		}
	virtual double operator()(double r2) const
		{
		// result=((...(cn*r2+c(n-1))...+c2)*r2+c1)*r2+c0
		double result=c[degree];
		for(unsigned int i=degree;i>0;--i)
			result=result*r2+c[i-1];
		return result;
		}
	virtual double d(double r2) const
		{
		// result=(...(n*cn*r2+(n-1)*c(n-1))...+2*c2)*r2+c1
		double result=double(degree)*c[degree];
		for(unsigned int i=degree;i>1;--i)
			result=result*r2+double(i-1)*c[i-1];
		return result;
		}
	
	/* New methods: */
	void setC(unsigned int i,double newC)
		{
		c[i]=newC;
		}
	};

class ReciprocalPolynomialDistortionEquation:public LensCorrector::DistortionEquation // Class for reciprocal polynomial equations
	{
	/* Elements: */
	private:
	unsigned int degree; // Polynomial degree
	double* c; // Array of polynomial coefficients
	
	/* Constructors and destructors: */
	public:
	ReciprocalPolynomialDistortionEquation(unsigned int sDegree,const double* sC =0)
		:degree(sDegree),
		 c(new double[degree+1])
		{
		if(sC!=0)
			{
			for(unsigned int i=0;i<=degree;++i)
				c[i]=sC[i];
			}
		else
			{
			for(unsigned int i=0;i<=degree;++i)
				c[i]=0.0;
			}
		}
	virtual ~ReciprocalPolynomialDistortionEquation(void)
		{
		delete[] c;
		}
	
	/* Methods from LensCorrector::DistortionEquation: */
	virtual double getMaxR2(void) const
		{
		return 1.0;
		}
	virtual double operator()(double r2) const
		{
		// result=1.0/(((...(cn*r2+c(n-1))...+c2)*r2+c1)*r2+c0)
		double result=c[degree];
		for(unsigned int i=degree;i>0;--i)
			result=result*r2+c[i-1];
		return 1.0/result;
		}
	virtual double d(double r2) const
		{
		double result=c[degree];
		for(unsigned int i=degree;i>0;--i)
			result=result*r2+c[i-1];
		double dresult=double(degree)*c[degree];
		for(unsigned int i=degree;i>1;--i)
			dresult=dresult*r2+double(i-1)*c[i-1];
		return -dresult/(result*result);
		}
	
	/* New methods: */
	void setC(unsigned int i,double newC)
		{
		c[i]=newC;
		}
	};


class CatmullRomDistortionEquation:public LensCorrector::DistortionEquation // Class for special-purpose Catmull-Rom splines
	{
	/* Elements: */
	private:
	double r2Max; // Maximum squared radius for which the spline is defined; abscissa of controlPoints[numControlPoints-1]
	unsigned int numControlPoints; // Number of spline control points
	double* controlPoints; // Array of spline control point ordinates
	
	/* Constructors and destructors: */
	public:
	CatmullRomDistortionEquation(double sR2Max,unsigned int sNumControlPoints,const double* sControlPoints =0)
		:r2Max(sR2Max),
		 numControlPoints(sNumControlPoints),controlPoints(new double[numControlPoints])
		{
		if(sControlPoints!=0)
			for(unsigned int i=0;i<numControlPoints;++i)
				controlPoints[i]=sControlPoints[i];
		}
	virtual ~CatmullRomDistortionEquation(void)
		{
		delete[] controlPoints;
		}
	
	/* Methods from LensCorrector::DistortionEquation: */
	virtual double getMaxR2(void) const
		{
		return r2Max;
		}
	virtual double operator()(double r2) const
		{
		/* Find the segment index and segment location of the query value: */
		double segLoc=r2*double(numControlPoints-1)/r2Max;
		unsigned int seg=(unsigned int)segLoc;
		if(seg>numControlPoints-1)
			seg=numControlPoints-1;
		double loc=segLoc-double(seg);
		
		/* Find the left and right control point values and slopes of segment seg: */
		double r0,dr0,r1,dr1;
		if(seg==0)
			{
			r0=1.0; // Special case; first control point value is always 1
			dr0=controlPoints[1]-controlPoints[0]; // Special case; first control point only affects slope
			r1=controlPoints[1];
			dr1=0.5*(controlPoints[2]-controlPoints[0]);
			}
		else if(seg==numControlPoints-2)
			{
			r0=controlPoints[seg];
			dr0=0.5*(controlPoints[seg+1]-controlPoints[seg-1]);
			r1=controlPoints[seg+1];
			dr1=controlPoints[seg+1]-controlPoints[seg];
			}
		else if(seg==numControlPoints-1)
			{
			/* Extend the spline past the last control point as a straight line: */
			r0=controlPoints[seg];
			dr0=controlPoints[seg]-controlPoints[seg-1];
			r1=r0+dr0;
			dr1=dr0;
			}
		else
			{
			r0=controlPoints[seg];
			dr0=0.5*(controlPoints[seg+1]-controlPoints[seg-1]);
			r1=controlPoints[seg+1];
			dr1=0.5*(controlPoints[seg+2]-controlPoints[seg]);
			}
		
		/* Interpolate inside the segment: */
		return (r0*(1.0+2.0*loc)+dr0*loc)*(1.0-loc)*(1.0-loc)+(r1*(1.0+2.0*(1.0-loc))-dr1*(1.0-loc))*loc*loc;
		return (((2.0*r0+dr0-2.0*r1+dr1)*loc-3.0*r0-2.0*dr0+3.0*r1-dr1)*loc+dr0)*loc+r0;
		}
	virtual double d(double r2) const
		{
		/* Find the segment index and segment location of the query value: */
		double segLoc=r2*double(numControlPoints-1)/r2Max;
		unsigned int seg=(unsigned int)segLoc;
		if(seg>numControlPoints-1)
			seg=numControlPoints-1;
		double loc=segLoc-double(seg);
		
		/* Find the left and right control point values and slopes of segment seg: */
		double r0,dr0,r1,dr1;
		if(seg==0)
			{
			r0=1.0; // Special case; first control point value is always 1
			dr0=controlPoints[1]-controlPoints[0]; // Special case; first control point only affects slope
			r1=controlPoints[1];
			dr1=0.5*(controlPoints[2]-controlPoints[0]);
			}
		else if(seg==numControlPoints-2)
			{
			r0=controlPoints[seg];
			dr0=0.5*(controlPoints[seg+1]-controlPoints[seg-1]);
			r1=controlPoints[seg+1];
			dr1=controlPoints[seg+1]-controlPoints[seg];
			}
		else if(seg==numControlPoints-1)
			{
			/* Extend the spline past the last control point as a straight line: */
			r0=controlPoints[seg];
			dr0=controlPoints[seg]-controlPoints[seg-1];
			r1=r0+dr0;
			dr1=dr0;
			}
		else
			{
			r0=controlPoints[seg];
			dr0=0.5*(controlPoints[seg+1]-controlPoints[seg-1]);
			r1=controlPoints[seg+1];
			dr1=0.5*(controlPoints[seg+2]-controlPoints[seg]);
			}
		
		/* Interpolate the spline derivative inside the segment: */
		return (((6.0*r0+3.0*dr0-6.0*r1+3.0*dr1)*loc-6.0*r0-4.0*dr0+6.0*r1-2.0*dr1)*loc+dr0)*double(numControlPoints-1)/r2Max;
		}
	
	/* New methods: */
	void setC(unsigned int i,double newC)
		{
		controlPoints[i]=newC;
		}
	};

LensCorrector::DistortionEquation* parseDistortionEquation(const Misc::ConfigurationFileSection& configFileSection)
	{
	LensCorrector::DistortionEquation* result=0;
	
	/* Determine the distortion equation type: */
	std::string eqType=configFileSection.retrieveString("./type");
	if(strcasecmp(eqType.c_str(),"Polynomial")==0)
		{
		/* Read the list of coefficients: */
		std::vector<double> coefficients=configFileSection.retrieveValue<std::vector<double> >("./coefficients");
		
		/* Adjust the first coefficient: */
		coefficients[0]+=1.0;
		
		/* Create the result equation: */
		result=new PolynomialDistortionEquation(coefficients.size()-1,&coefficients.front());
		}
	else if(strcasecmp(eqType.c_str(),"ReciprocalPolynomial")==0)
		{
		/* Read the list of coefficients: */
		std::vector<double> coefficients=configFileSection.retrieveValue<std::vector<double> >("./coefficients");
		
		/* Adjust the first coefficient: */
		coefficients[0]+=1.0;
		
		/* Create the result equation: */
		result=new ReciprocalPolynomialDistortionEquation(coefficients.size()-1,&coefficients.front());
		}
	else if(strcasecmp(eqType.c_str(),"CatmullRomSpline")==0)
		{
		/* Read the maximum squared radius: */
		double r2Max=configFileSection.retrieveValue<double>("./r2Max");
		
		/* Read the list of coefficients: */
		std::vector<double> coefficients=configFileSection.retrieveValue<std::vector<double> >("./coefficients");
		
		/* Create the result equation: */
		result=new CatmullRomDistortionEquation(r2Max,coefficients.size(),&coefficients.front());
		}
	else
		Misc::throwStdErr("Vrui::LensCorrector: Unknown distortion function type %s",eqType.c_str());
	
	return result;
	}

}

/******************************
Methods of class LensCorrector:
******************************/

LensCorrector::LensCorrector(const VRWindow& window,const WindowProperties& windowProperties,int multisamplingLevel,const GLWindow::WindowPos viewportPos[2],const Misc::ConfigurationFileSection& configFileSection)
	:predistortionMultisamplingLevel(multisamplingLevel),
	 predistortionStencilBufferSize(windowProperties.stencilBufferSize),
	 warpCubicLookup(false),correctOledResponse(false),fixContrast(true)
	{
	/* Initialize lens corrector state: */
	for(int eye=0;eye<2;++eye)
		for(int i=0;i<3;++i)
			lensConfigs[eye].distortionEquations[i]=0;
	
	/* Common lens configuration defaults: */
	double lensCenterDist=configFileSection.retrieveValue<double>("./lensCenterDist",Vrui::getInchFactor()*2.5);
	double lensScreenDist=configFileSection.retrieveValue<double>("./lensScreenDist",Math::mid(window.getVRScreen(0)->getWidth(),window.getVRScreen(1)->getWidth()));
	
	/* Left eye configuration: */
	lensConfigs[0].lensCenter=Point(window.getVRScreen(0)->getWidth()-lensCenterDist*0.5,window.getVRScreen(0)->getHeight()*0.5,lensScreenDist);
	lensConfigs[0].lensCenter=configFileSection.retrieveValue<Point>("./leftLensCenter",lensConfigs[0].lensCenter);
	
	Misc::FixedArray<std::string,3> leftFormulaNames=configFileSection.retrieveValue<Misc::FixedArray<std::string,3> >("./leftFormulaNames");
	for(int i=0;i<3;++i)
		lensConfigs[0].distortionEquations[i]=parseDistortionEquation(configFileSection.getSection(leftFormulaNames[i].c_str()));
	
	Misc::FixedArray<double,4> leftOverscan(0.0);
	leftOverscan=configFileSection.retrieveValue<Misc::FixedArray<double,4> >("./leftOverscan",leftOverscan);
	for(int i=0;i<4;++i)
		lensConfigs[0].overscan[i]=leftOverscan[i];
	
	/* Right eye configuration: */
	lensConfigs[1].lensCenter=Point(lensCenterDist*0.5,window.getVRScreen(1)->getHeight()*0.5,lensScreenDist);
	lensConfigs[1].lensCenter=configFileSection.retrieveValue<Point>("./rightLensCenter",lensConfigs[1].lensCenter);
	
	Misc::FixedArray<std::string,3> rightFormulaNames=configFileSection.retrieveValue<Misc::FixedArray<std::string,3> >("./rightFormulaNames");
	for(int i=0;i<3;++i)
		lensConfigs[1].distortionEquations[i]=parseDistortionEquation(configFileSection.getSection(rightFormulaNames[i].c_str()));
	
	Misc::FixedArray<double,4> rightOverscan(0.0);
	rightOverscan=configFileSection.retrieveValue<Misc::FixedArray<double,4> >("./rightOverscan",rightOverscan);
	for(int i=0;i<4;++i)
		lensConfigs[1].overscan[i]=rightOverscan[i];
	
	/* Ensure that both viewports have the same size: */
	if(viewportPos[0].size[0]!=viewportPos[1].size[0]||viewportPos[0].size[1]!=viewportPos[1].size[1])
		{
		Misc::throwStdErr("Vrui::LensCorrector: Left and right viewports have different sizes, %dx%d vs %dx%d",
		                  viewportPos[0].size[0],viewportPos[0].size[1],
											viewportPos[1].size[0],viewportPos[1].size[1]);
		}
	
	/* Shared configuration: */
	for(int i=0;i<2;++i)
		predistortionFrameSize[i]=viewportPos[0].size[i];
	double superSampling=configFileSection.retrieveValue<double>("./superSampling",1.0);
	finalViewport[0]=0;
	finalViewport[1]=0;
	finalViewport[2]=window.getWindowWidth();
	finalViewport[3]=window.getWindowHeight();
	
	/* Calculate derived lens configuration: */
	for(int eye=0;eye<2;++eye)
		{
		const VRScreen* screen=window.getVRScreen(eye);
		
		/* Calculate the lens distortion center in texture coordinates: */
		for(int i=0;i<2;++i)
			lensConfigs[eye].center[i]=lensConfigs[eye].lensCenter[i]/screen->getScreenSize()[i];
		
		/* Calculate scaling factors to create an isometric texture coordinate system: */
		if(screen->getWidth()>=screen->getHeight())
			{
			lensConfigs[eye].preScale[0]=2.0*double(screen->getWidth())/double(screen->getHeight());
			lensConfigs[eye].preScale[1]=2.0;
			}
		else
			{
			lensConfigs[eye].preScale[0]=2.0;
			lensConfigs[eye].preScale[1]=2.0*double(screen->getHeight())/double(screen->getWidth());
			}
		
		/* Calculate total overscan sizes: */
		for(int i=0;i<2;++i)
			lensConfigs[eye].overscanSize[i]=1.0+lensConfigs[eye].overscan[2*i+0]+lensConfigs[eye].overscan[2*i+1];
		
		/* Copy post-distortion viewport positions and sizes: */
		lensConfigs[eye].finalViewport[0]=viewportPos[eye].origin[0];
		lensConfigs[eye].finalViewport[1]=viewportPos[eye].origin[1];
		lensConfigs[eye].finalViewport[2]=viewportPos[eye].size[0];
		lensConfigs[eye].finalViewport[3]=viewportPos[eye].size[1];
		}
	
	/* Calculate average overscan size to construct identical frame buffers for left and right eye: */
	double oss[2];
	for(int i=0;i<2;++i)
		oss[i]=Math::mid(lensConfigs[0].overscanSize[i],lensConfigs[1].overscanSize[i]);
	
	/* Calculate the overscanned and super-sampled pre-distortion frame buffer size: */
	for(int i=0;i<2;++i)
		predistortionFrameSize[i]=int(Math::floor(double(predistortionFrameSize[i])*superSampling*oss[i]+0.5));
	
	/* Initialize the required OpenGL extensions: */
	GLARBMultitexture::initExtension();
	GLEXTFramebufferObject::initExtension();
	if(predistortionStencilBufferSize>0)
		GLEXTPackedDepthStencil::initExtension();
	if(predistortionMultisamplingLevel>1)
		{
		GLEXTFramebufferBlit::initExtension();
		GLEXTFramebufferMultisample::initExtension();
		}
	GLARBVertexBufferObject::initExtension();
	GLShader::initExtensions();
	
	/* Create the pre-distortion rendering frame buffer: */
	glGenFramebuffersEXT(1,&predistortionFrameBufferId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,predistortionFrameBufferId);
	
	/* Create the pre-distortion color image texture: */
	glGenTextures(2,predistortionColorBufferIds);
	for(int eye=0;eye<2;++eye)
		{
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,predistortionColorBufferIds[eye]);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MIN_FILTER,warpCubicLookup?GL_NEAREST:GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAG_FILTER,warpCubicLookup?GL_NEAREST:GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
		GLfloat texBorderColor[4]={0.0f,0.0f,0.0f,1.0f};
		glTexParameterfv(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_BORDER_COLOR,texBorderColor);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGB8,predistortionFrameSize[0],predistortionFrameSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,0);
		}
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
	
	if(predistortionMultisamplingLevel>1)
		{
		/* Create the pre-distortion multisampling color buffer: */
		glGenRenderbuffersEXT(1,&predistortionMultisamplingColorBufferId);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,predistortionMultisamplingColorBufferId);
		glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,predistortionMultisamplingLevel,GL_RGB8,predistortionFrameSize[0],predistortionFrameSize[1]);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
		
		/* Attach the pre-distortion multisampling color buffer to the frame buffer: */
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_RENDERBUFFER_EXT,predistortionMultisamplingColorBufferId);
		}
	else
		{
		/* Attach the pre-distortion color image textures to the frame buffer: */
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_RECTANGLE_ARB,predistortionColorBufferIds[0],0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_RECTANGLE_ARB,predistortionColorBufferIds[1],0);
		}
	
	/* Create the pre-distortion depth buffer: */
	if(predistortionStencilBufferSize>0)
		{
		/* Create an interleaved depth+stencil render buffer: */
		if(predistortionStencilBufferSize>8)
			Misc::throwStdErr("Vrui::LensCorrector: Lens distortion correction not supported with stencil depth %d>8",int(predistortionStencilBufferSize));
		glGenRenderbuffersEXT(1,&predistortionDepthStencilBufferId);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,predistortionDepthStencilBufferId);
		if(predistortionMultisamplingLevel>1)
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,predistortionMultisamplingLevel,GL_DEPTH24_STENCIL8_EXT,predistortionFrameSize[0],predistortionFrameSize[1]);
		else
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH24_STENCIL8_EXT,predistortionFrameSize[0],predistortionFrameSize[1]);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
		
		/* Attach the pre-distortion interleaved depth and stencil buffer to the frame buffer: */
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,predistortionDepthStencilBufferId);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_STENCIL_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,predistortionDepthStencilBufferId);
		}
	else
		{
		/* Create a depth-only render buffer: */
		glGenRenderbuffersEXT(1,&predistortionDepthStencilBufferId);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,predistortionDepthStencilBufferId);
		if(predistortionMultisamplingLevel>1)
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT,predistortionMultisamplingLevel,GL_DEPTH_COMPONENT,predistortionFrameSize[0],predistortionFrameSize[1]);
		else
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT,predistortionFrameSize[0],predistortionFrameSize[1]);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT,0);
		
		/* Attach the pre-distortion depth buffer to the frame buffer: */
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,predistortionDepthStencilBufferId);
		}
	
	/* Set up pixel sources and destinations: */
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	
	/* Check the status of the lens correction frame buffer: */
	glThrowFramebufferStatusExceptionEXT("Vrui::LensCorrector: Lens correction framebuffer incomplete due to");
	
	if(predistortionMultisamplingLevel>1)
		{
		/* Create the multisample "fixing" frame buffer: */
		glGenFramebuffersEXT(1,&multisamplingFrameBufferId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,multisamplingFrameBufferId);
		
		/* Attach the pre-distortion color image textures to the "fixing" frame buffer: */
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_RECTANGLE_ARB,predistortionColorBufferIds[0],0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_RECTANGLE_ARB,predistortionColorBufferIds[1],0);
		
		/* Check the status of the multisample "fixing" frame buffer: */
		glThrowFramebufferStatusExceptionEXT("Vrui::LensCorrector: Multisampling framebuffer incomplete due to");
		}
	
	/* Protect the created frame buffer(s): */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	
	/* Create the left and right warping meshes: */
	Misc::FixedArray<int,2> wms(64);
	wms=configFileSection.retrieveValue<Misc::FixedArray<int,2> >("./warpMeshSize",wms);
	for(int i=0;i<2;++i)
		warpMeshSize[i]=wms[i]+1;
	
	glGenBuffersARB(2,warpMeshVertexBufferIds);
	for(int eye=0;eye<2;++eye)
		{
		/* Calculate mesh creation parameters: */
		LensConfig& lc=lensConfigs[eye];
		Point2 postCenter;
		Scale2 postScale;
		for(int i=0;i<2;++i)
			{
			postCenter[i]=(lc.center[i]+lc.overscan[i*2])*double(predistortionFrameSize[i])/lc.overscanSize[i];
			postScale[i]=double(predistortionFrameSize[i])/(lc.preScale[i]*lc.overscanSize[i]);
			}
		double left=(0.0-lc.center[0])*lc.preScale[0];
		double right=(1.0-lc.center[0])*lc.preScale[0];
		double bottom=(0.0-lc.center[1])*lc.preScale[1];
		double top=(1.0-lc.center[1])*lc.preScale[1];
		
		/* Generate mesh vertex coordinates: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,warpMeshVertexBufferIds[eye]);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,warpMeshSize[1]*warpMeshSize[0]*sizeof(WarpMeshVertex),0,GL_STATIC_DRAW_ARB);
		
		/* Map the mesh vertex buffer and enter vertices in sequential order: */
		WarpMeshVertex* wmvPtr=static_cast<WarpMeshVertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		for(int y=0;y<warpMeshSize[1];++y)
			{
			for(int x=0;x<warpMeshSize[0];++x,++wmvPtr)
				{
				/* The forward distortion formula: */
				
				Point2 post; // Rectified final viewport position
				post[0]=2.0*(double(x)/double(warpMeshSize[0]-1)*double(lc.finalViewport[2])+double(lc.finalViewport[0]))/double(finalViewport[2])-1.0;
				post[1]=2.0*(double(y)/double(warpMeshSize[1]-1)*double(lc.finalViewport[3])+double(lc.finalViewport[1]))/double(finalViewport[3])-1.0;
				
				Point2 pre; // Pre-distortion viewport position
				pre[0]=(double(x)*(right-left))/double(warpMeshSize[0]-1)+left;
				pre[1]=(double(y)*(top-bottom))/double(warpMeshSize[1]-1)+bottom;
				double r2=pre.sqr();
				double r2p=(*lc.distortionEquations[0])(r2);
				Point2 gTex; // Pre-distortion pixel buffer position
				Point2 rTex,bTex; // Chroma-corrected pre-distortion pixel buffer positions
				for(int i=0;i<2;++i)
					{
					rTex[i]=postCenter[i]+pre[i]*r2p*postScale[i]*(*lc.distortionEquations[1])(r2);
					gTex[i]=postCenter[i]+pre[i]*r2p*postScale[i];
					bTex[i]=postCenter[i]+pre[i]*r2p*postScale[i]*(*lc.distortionEquations[2])(r2);
					}
				
				wmvPtr->redTex[0]=GLfloat(rTex[0]);
				wmvPtr->redTex[1]=GLfloat(rTex[1]);
				wmvPtr->greenTex[0]=GLfloat(gTex[0]);
				wmvPtr->greenTex[1]=GLfloat(gTex[1]);
				wmvPtr->blueTex[0]=GLfloat(bTex[0]);
				wmvPtr->blueTex[1]=GLfloat(bTex[1]);
				wmvPtr->pos[0]=GLfloat(post[0]);
				wmvPtr->pos[1]=GLfloat(post[1]);
				}
			}
		
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		}
	
	/* Protect the mesh vertex buffers: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		
	/* Generate mesh vertex indices: */
	glGenBuffersARB(1,&warpMeshIndexBufferId);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,warpMeshIndexBufferId);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,(warpMeshSize[1]-1)*warpMeshSize[0]*2*sizeof(GLuint),0,GL_STATIC_DRAW_ARB);
	
	/* Map the mesh index buffer and enter indices in sequential order: */
	GLuint* wmiPtr=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	for(int y=1;y<warpMeshSize[1];++y)
		{
		for(int x=0;x<warpMeshSize[0];++x,wmiPtr+=2)
			{
			wmiPtr[0]=y*warpMeshSize[0]+x;
			wmiPtr[1]=(y-1)*warpMeshSize[0]+x;
			}
		}
	
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	
	/* Protect the mesh index buffers: */
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	
	/* Check for OLED response time correction factors: */
	Misc::FixedArray<double,2> ocf(0.0);
	ocf=configFileSection.retrieveValue<Misc::FixedArray<double,2> >("./oledCorrectionFactors",ocf);
	correctOledResponse=ocf[0]!=0.0||ocf[1]!=0.0;
	if(correctOledResponse)
		{
		/* Set the correction factors: */
		for(int i=0;i<2;++i)
			oledCorrectionFactors[i]=GLfloat(ocf[i]);
		
		fixContrast=configFileSection.retrieveValue<bool>("./fixContrast",fixContrast);
		if(fixContrast)
			{
			/* Calculate the OLED contrast correction coefficients: */
			oledContrast[1]=oledCorrectionFactors[0]/(1.0f+oledCorrectionFactors[0]); // Offset
			oledContrast[0]=1.0f/(1.0f+oledCorrectionFactors[1])-oledContrast[1]; // Scale
			}
		
		/* Create the texture holding the previously rendered frame: */
		glGenTextures(1,&previousFrameTextureId);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,previousFrameTextureId);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAX_LEVEL,0);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB,0,GL_RGB8,finalViewport[2],finalViewport[3],0,GL_RGB,GL_UNSIGNED_BYTE,0);
		
		/* Protect the texture object: */
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
		}
	
	/* Compile the warping shader's vertex program: */
	static const char* warpingShaderVertexProgram="\
		attribute vec2 redTexIn;\n\
		attribute vec2 greenTexIn;\n\
		attribute vec2 blueTexIn;\n\
		\n\
		varying vec2 redTex;\n\
		varying vec2 greenTex;\n\
		varying vec2 blueTex;\n\
		\n\
		void main()\n\
			{\n\
			redTex=redTexIn;\n\
			greenTex=greenTexIn;\n\
			blueTex=blueTexIn;\n\
			gl_Position=gl_Vertex;\n\
			}\n";
	warpingShader.compileVertexShaderFromString(warpingShaderVertexProgram);
	
	/* Construct the lens distortion correction fragment shader: */
	std::string warpingShaderFragmentProgramDeclarations="\
		#extension GL_ARB_texture_rectangle : enable\n\
		\n\
		varying vec2 redTex;\n\
		varying vec2 greenTex;\n\
		varying vec2 blueTex;\n\
		\n\
		uniform sampler2DRect predistortionImageSampler;\n";
	
	std::string warpingShaderFragmentProgramMain="\
		\n\
		void main()\n\
			{\n\
			/* Get the pixel color's red, green, and blue components via their individual texture coordinates: */\n\
			float red=texture2DRect(predistortionImageSampler,redTex).r;\n\
			float green=texture2DRect(predistortionImageSampler,greenTex).g;\n\
			float blue=texture2DRect(predistortionImageSampler,blueTex).b;\n\
			vec4 newColor=vec4(red,green,blue,1.0);\n\
			\n";
	
	if(correctOledResponse)
		{
		warpingShaderFragmentProgramDeclarations.append("\
			uniform sampler2DRect previousFrameImageSampler;\n\
			uniform float overdrive[2];\n");
		
		if(fixContrast)
			{
			warpingShaderFragmentProgramDeclarations.append("\
				uniform float contrast[2];\n");
			
			warpingShaderFragmentProgramMain.append("\
				/* Reduce contrast in the pixel's color to give room for OLED response correction: */\n\
				newColor=newColor*contrast[0]+vec4(contrast[1]);\n\
				\n");
			}
		
		warpingShaderFragmentProgramMain.append("\
			/* Get the previous frame's color for the same pixel: */\n\
			vec4 previousColor=texture2DRect(previousFrameImageSampler,gl_FragCoord.xy);\n\
			if(newColor.r>=previousColor.r)\n\
				newColor.r=newColor.r+(newColor.r-previousColor.r)*overdrive[0];\n\
			else\n\
				newColor.r=newColor.r+(newColor.r-previousColor.r)*overdrive[1];\n\
			if(newColor.g>=previousColor.g)\n\
				newColor.g=newColor.g+(newColor.g-previousColor.g)*overdrive[0];\n\
			else\n\
				newColor.g=newColor.g+(newColor.g-previousColor.g)*overdrive[1];\n\
			if(newColor.b>=previousColor.b)\n\
				newColor.b=newColor.b+(newColor.b-previousColor.b)*overdrive[0];\n\
			else\n\
				newColor.b=newColor.b+(newColor.b-previousColor.b)*overdrive[1];\n\
			\n");
		}
	
	warpingShaderFragmentProgramMain.append("\
		gl_FragColor=newColor;\n\
		}\n");
	
	/* Compile the warping shader's fragment program: */
	warpingShader.compileFragmentShaderFromString((warpingShaderFragmentProgramDeclarations+warpingShaderFragmentProgramMain).c_str());
	
	/* Link the shader and query its attribute and uniform locations: */
	warpingShader.linkShader();
	warpingShaderAttributeIndices[0]=warpingShader.getAttribLocation("redTexIn");
	warpingShaderAttributeIndices[1]=warpingShader.getAttribLocation("greenTexIn");
	warpingShaderAttributeIndices[2]=warpingShader.getAttribLocation("blueTexIn");
	warpingShaderUniformIndices[0]=warpingShader.getUniformLocation("predistortionImageSampler");
	if(correctOledResponse)
		{
		warpingShaderUniformIndices[1]=warpingShader.getUniformLocation("previousFrameImageSampler");
		warpingShaderUniformIndices[2]=warpingShader.getUniformLocation("overdrive");
		if(fixContrast)
			warpingShaderUniformIndices[3]=warpingShader.getUniformLocation("contrast");
		}
	}

LensCorrector::~LensCorrector(void)
	{
	/* Clean up lens configurations: */
	for(int eye=0;eye<2;++eye)
		for(int i=0;i<3;++i)
			delete lensConfigs[eye].distortionEquations[i];
	
	/* Release all allocated OpenGL resources: */
	glDeleteFramebuffersEXT(1,&predistortionFrameBufferId);
	glDeleteTextures(2,predistortionColorBufferIds);
	if(predistortionMultisamplingLevel>1)
		glDeleteRenderbuffersEXT(1,&predistortionMultisamplingColorBufferId);
	glDeleteRenderbuffersEXT(1,&predistortionDepthStencilBufferId);
	if(predistortionMultisamplingLevel>1)
		glDeleteFramebuffersEXT(1,&multisamplingFrameBufferId);
	glDeleteBuffersARB(2,warpMeshVertexBufferIds);
	glDeleteBuffersARB(1,&warpMeshIndexBufferId);
	if(correctOledResponse)
		glDeleteTextures(1,&previousFrameTextureId);
	}

void LensCorrector::prepare(int eye,DisplayState& displayState) const
	{
	/* Bind the pre-distortion frame buffer: */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,predistortionFrameBufferId);
	if(predistortionMultisamplingLevel>1)
		{
		/* Draw into the multisampling image buffer: */
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		}
	else
		{
		/* Draw directly into the left and right color image buffers: */
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT+eye);
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+eye);
		}
	
	/* Override the viewport position and size: */
	glViewport(0,0,predistortionFrameSize[0],predistortionFrameSize[1]);
	displayState.viewport[0]=displayState.viewport[1]=0;
	displayState.viewport[2]=predistortionFrameSize[0];
	displayState.viewport[3]=predistortionFrameSize[1];
	for(int i=0;i<2;++i)
		displayState.frameSize[i]=predistortionFrameSize[i];
	}

void LensCorrector::adjustProjection(int eye,const Point& screenEyePos,double near,double& left,double& right,double& bottom,double& top) const
	{
	/* Calculate the displacement vector between the eye position and the lens center position in screen coordinates: */
	double dx=screenEyePos[0]-lensConfigs[eye].lensCenter[0];
	double dy=screenEyePos[1]-lensConfigs[eye].lensCenter[1];
	
	/* Adjust the projection matrix for lens correction: */
	left+=dx/screenEyePos[2]*near;
	right+=dx/screenEyePos[2]*near;
	bottom+=dy/screenEyePos[2]*near;
	top+=dy/screenEyePos[2]*near;
	
	/* Adjust the projection matrix for overscan: */
	double w=right-left;
	left-=w*double(lensConfigs[eye].overscan[0]);
	right+=w*double(lensConfigs[eye].overscan[1]);
	double h=top-bottom;
	bottom-=h*double(lensConfigs[eye].overscan[2]);
	top+=h*double(lensConfigs[eye].overscan[3]);
	}

void LensCorrector::finish(int eye) const
	{
	if(predistortionMultisamplingLevel>1)
		{
		/* Blit the multisampling color buffer containing the pre-distortion image into the "fixing" frame buffer: */
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,multisamplingFrameBufferId);
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT+eye);
		glBlitFramebufferEXT(0,0,predistortionFrameSize[0],predistortionFrameSize[1],0,0,predistortionFrameSize[0],predistortionFrameSize[1],GL_COLOR_BUFFER_BIT,GL_NEAREST);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
		}
	}

void LensCorrector::warp(void) const
	{
	/* Bind the final drawable's frame buffer: */
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	glViewport(finalViewport[0],finalViewport[1],finalViewport[2],finalViewport[3]);
	
	/* Set up the warping mesh buffer structure: */
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,warpMeshIndexBufferId);
	for(int i=0;i<3;++i)
		glEnableVertexAttribArrayARB(warpingShaderAttributeIndices[i]);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	/* Render the right and left warping meshes: */
	warpingShader.useProgram();
	if(correctOledResponse)
		{
		/* Bind the previous frame's image texture: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,previousFrameTextureId);
		glUniformARB(warpingShaderUniformIndices[1],1);
		
		/* Set the up- and down-scaling factors: */
		glUniformARB<1>(warpingShaderUniformIndices[2],2,oledCorrectionFactors);
		
		if(fixContrast)
			{
			/* Set the contrast reduction coefficients: */
			glUniformARB<1>(warpingShaderUniformIndices[3],2,oledContrast);
			}
		}
	
	for(int eye=1;eye>=0;--eye)
		{
		/* Bind the pre-distortion color image texture: */
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,predistortionColorBufferIds[eye]);
		glUniformARB(warpingShaderUniformIndices[0],0);
		
		/* Bind the vertex buffer: */
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,warpMeshVertexBufferIds[eye]);
		WarpMeshVertex* vertices=0;
		glVertexAttribPointerARB(warpingShaderAttributeIndices[0],2,GL_FLOAT,GL_FALSE,sizeof(WarpMeshVertex),&vertices[0].redTex);
		glVertexAttribPointerARB(warpingShaderAttributeIndices[1],2,GL_FLOAT,GL_FALSE,sizeof(WarpMeshVertex),&vertices[0].greenTex);
		glVertexAttribPointerARB(warpingShaderAttributeIndices[2],2,GL_FLOAT,GL_FALSE,sizeof(WarpMeshVertex),&vertices[0].blueTex);
		glVertexPointer(2,GL_FLOAT,sizeof(WarpMeshVertex),&vertices[0].pos);
		
		/* Render the mesh as a sequence of quad strips: */
		const GLuint* indexPtr=0;
		for(int y=1;y<warpMeshSize[1];++y,indexPtr+=warpMeshSize[0]*2)
			glDrawElements(GL_QUAD_STRIP,warpMeshSize[0]*2,GL_UNSIGNED_INT,indexPtr);
		}
	
	if(correctOledResponse)
		{
		/* Protect the previous frame's image texture: */
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
		}
	
	/* Protect the color image texture: */
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
	
	/* Disable vertex arrays: */
	glDisableClientState(GL_VERTEX_ARRAY);
	for(int i=0;i<3;++i)
		glDisableVertexAttribArrayARB(warpingShaderAttributeIndices[i]);
	
	/* Protect the mesh buffers: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	
	/* Disable the warping shader: */
	GLShader::disablePrograms();
	
	if(correctOledResponse)
		{
		/* Copy the final rendered buffer into a texture for the next frame: */
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,previousFrameTextureId);
		glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB,0,0,0,finalViewport[0],finalViewport[1],finalViewport[2],finalViewport[3]);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,0);
		}
	}

}
