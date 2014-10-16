/***********************************************************************
EarthFunctions - Helper functions to display models of Earth and other
Earth-related stuff.
Copyright (c) 2005 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Math/Math.h>
#include <Math/Constants.h>
#include <Geometry/Geoid.h>
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

/***********************************************************************
Function to calculate radius of Earth at given latitude (geoid formula):
***********************************************************************/

double calcRadius(double latitude)
	{
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	
	/* Calculate radius at latitude according to geoid formula: */
	return a*(1.0-f*Math::sqr(Math::sin(latitude)));
	}

/***********************************************************************
Function to calculate the Cartesian coordinates of a point on the
Earth's surface:
***********************************************************************/

template <class ScalarParam>
void calcSurfacePos(ScalarParam latitude,ScalarParam longitude,double scaleFactor,ScalarParam pos[3])
	{
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	
	double s0=Math::sin(double(latitude));
	double c0=Math::cos(double(latitude));
	double r=a*(1.0-f*Math::sqr(s0))*scaleFactor;
	double xy=r*c0;
	double s1=Math::sin(double(longitude));
	double c1=Math::cos(double(longitude));
	pos[0]=ScalarParam(xy*c1);
	pos[1]=ScalarParam(xy*s1);
	pos[2]=ScalarParam(r*s0);
	}

/***********************************************************************
Function to calculate the Cartesian coordinates of a point in the
Earth's interior, given a depth:
***********************************************************************/

template <class ScalarParam>
void calcDepthPos(ScalarParam latitude,ScalarParam longitude,ScalarParam depth,double scaleFactor,ScalarParam pos[3])
	{
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	
	double s0=Math::sin(double(latitude));
	double c0=Math::cos(double(latitude));
	double r=(a*(1.0-f*Math::sqr(s0))-depth)*scaleFactor;
	double xy=r*c0;
	double s1=Math::sin(double(longitude));
	double c1=Math::cos(double(longitude));
	pos[0]=ScalarParam(xy*c1);
	pos[1]=ScalarParam(xy*s1);
	pos[2]=ScalarParam(r*s0);
	}

/***********************************************************************
Function to calculate the Cartesian coordinates of a point in the
Earth's interior, given a radius:
***********************************************************************/

template <class ScalarParam>
void calcRadiusPos(ScalarParam latitude,ScalarParam longitude,ScalarParam radius,double scaleFactor,ScalarParam pos[3])
	{
	double s0=Math::sin(double(latitude));
	double c0=Math::cos(double(latitude));
	double r=radius*scaleFactor;
	double xy=r*c0;
	double s1=Math::sin(double(longitude));
	double c1=Math::cos(double(longitude));
	pos[0]=ScalarParam(xy*c1);
	pos[1]=ScalarParam(xy*s1);
	pos[2]=ScalarParam(r*s0);
	}

/***********************************************************************
Function to draw a model of Earth using texture-mapped quad strips:
***********************************************************************/

void drawEarth(int numStrips,int numQuads,double scaleFactor)
	{
	Geometry::Geoid<double> wgs84; // Standard reference ellipsoid
	double wgs84E2=(2.0-wgs84.getFlatteningFactor())*wgs84.getFlatteningFactor();
	
	float texY1=float(0)/float(numStrips);
	double lat1=(Math::Constants<double>::pi*double(0))/double(numStrips)-0.5*Math::Constants<double>::pi;
	double s1=Math::sin(lat1);
	double c1=Math::cos(lat1);
	double chi1=Math::sqrt(1.0-wgs84E2*s1*s1);
	double xy1=wgs84.getRadius()/chi1*c1*scaleFactor;
	double z1=wgs84.getRadius()*(1.0-wgs84E2)/chi1*s1*scaleFactor;
	
	/* Draw latitude quad strips: */
	for(int i=1;i<numStrips+1;++i)
		{
		float texY0=texY1;
		double s0=s1;
		double c0=c1;
		double xy0=xy1;
		double z0=z1;
		texY1=float(i)/float(numStrips);
		lat1=(Math::Constants<double>::pi*double(i))/double(numStrips)-0.5*Math::Constants<double>::pi;
		s1=Math::sin(lat1);
		c1=Math::cos(lat1);
		chi1=Math::sqrt(1.0-wgs84E2*s1*s1);
		xy1=wgs84.getRadius()/chi1*c1*scaleFactor;
		z1=wgs84.getRadius()*(1.0-wgs84E2)/chi1*s1*scaleFactor;
		
		glBegin(GL_QUAD_STRIP);
		for(int j=0;j<=numQuads;++j)
			{
			float texX=float(j)/float(numQuads)+0.5f;
			double lng=(2.0*Math::Constants<double>::pi*double(j))/double(numQuads);
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			
			glTexCoord2f(texX,texY1);
			glNormal3d(c1*cl,c1*sl,s1);
			glVertex3d(xy1*cl,xy1*sl,z1);
			
			glTexCoord2f(texX,texY0);
			glNormal3d(c0*cl,c0*sl,s0);
			glVertex3d(xy0*cl,xy0*sl,z0);
			}
		glEnd();
		}
	}

/***********************************************************************
Function to draw a model of Earth using texture-mapped quad strips:
***********************************************************************/

void drawEarth(int numStrips,int numQuads,double scaleFactor,unsigned int vertexBufferObjectId,unsigned int indexBufferObjectId)
	{
	typedef GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3> Vertex;
	
	Geometry::Geoid<double> wgs84; // Standard reference ellipsoid
	double wgs84E2=(2.0-wgs84.getFlatteningFactor())*wgs84.getFlatteningFactor();
	
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	
	/* Upload the vertex data into the vertex buffer: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,vertexBufferObjectId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,(numStrips+1)*(numQuads+1)*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	Vertex* vPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	for(int i=0;i<=numStrips;++i)
		{
		float texY=float(i)/float(numStrips);
		double lat=(double(i)/double(numStrips)-0.5)*Math::Constants<double>::pi;
		double s=Math::sin(lat);
		double c=Math::cos(lat);
		double chi=Math::sqrt(1.0-wgs84E2*s*s);
		double xy=wgs84.getRadius()/chi*c*scaleFactor;
		double z=wgs84.getRadius()*(1.0-wgs84E2)/chi*s*scaleFactor;
		for(int j=0;j<=numQuads;++j,++vPtr)
			{
			float texX=float(j)/float(numQuads)+0.5f;
			double lng=(2.0*Math::Constants<double>::pi*double(j))/double(numQuads);
			double sl=Math::sin(lng);
			double cl=Math::cos(lng);
			vPtr->texCoord[0]=texX;
			vPtr->texCoord[1]=texY;
			vPtr->normal[0]=float(c*cl);
			vPtr->normal[1]=float(c*sl);
			vPtr->normal[2]=float(s);
			vPtr->position[0]=float(xy*cl);
			vPtr->position[1]=float(xy*sl);
			vPtr->position[2]=z;
			}
		}
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	
	/* Upload the index data into the index buffer: */
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,indexBufferObjectId);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numStrips*(numQuads+1)*2*sizeof(GLuint),0,GL_STATIC_DRAW_ARB);
	GLuint* iPtr=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
	for(int i=0;i<numStrips;++i)
		{
		for(int j=0;j<=numQuads;++j,iPtr+=2)
			{
			iPtr[0]=(i+1)*(numQuads+1)+j;
			iPtr[1]=(i+0)*(numQuads+1)+j;
			}
		}
	glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
	
	/* Render the quad strips: */
	glVertexPointer(static_cast<Vertex*>(0));
	GLubyte* stripBaseIndexPtr=0;
	for(int i=0;i<numStrips;++i)
		{
		glDrawElements(GL_QUAD_STRIP,(numQuads+1)*2,GL_UNSIGNED_INT,stripBaseIndexPtr);
		stripBaseIndexPtr+=(numQuads+1)*2*sizeof(GLuint);
		}
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

/***********************************************************************
Function to draw a latitude/longitude grid on the Earth's surface:
***********************************************************************/

void drawGrid(int numStrips,int numQuads,int overSample,double scaleFactor)
	{
	Geometry::Geoid<double> wgs84; // Standard reference ellipsoid
	double wgs84E2=(2.0-wgs84.getFlatteningFactor())*wgs84.getFlatteningFactor();
	
	/* Draw parallels: */
	for(int i=1;i<numStrips;++i)
		{
		double lat=(Math::Constants<double>::pi*double(i))/double(numStrips)-0.5*Math::Constants<double>::pi;
		
		double s=Math::sin(lat);
		double c=Math::cos(lat);
		double chi=Math::sqrt(1.0-wgs84E2*s*s);
		double xy=wgs84.getRadius()/chi*c*scaleFactor;
		double z=wgs84.getRadius()*(1.0-wgs84E2)/chi*s*scaleFactor;
		
		glBegin(GL_LINE_LOOP);
		for(int j=0;j<numQuads*overSample;++j)
			{
			double lng=(2.0*Math::Constants<double>::pi*double(j))/double(numQuads*overSample);
			double cl=Math::cos(lng);
			double sl=Math::sin(lng);
			glVertex3d(xy*cl,xy*sl,z);
			}
		glEnd();
		}
	
	/* Draw meridians: */
	for(int i=0;i<numQuads;++i)
		{
		double lng=(2.0*Math::Constants<double>::pi*double(i))/double(numQuads);
		double cl=Math::cos(lng);
		double sl=Math::sin(lng);
		
		glBegin(GL_LINE_STRIP);
		glVertex3d(0.0,0.0,-wgs84.getRadius()*(1.0-wgs84.getFlatteningFactor())*scaleFactor);
		for(int j=1;j<numStrips*overSample;++j)
			{
			double lat=(Math::Constants<double>::pi*double(j))/double(numStrips*overSample)-0.5*Math::Constants<double>::pi;
			double s=Math::sin(lat);
			double c=Math::cos(lat);
			double chi=Math::sqrt(1.0-wgs84E2*s*s);
			double xy=wgs84.getRadius()/chi*c*scaleFactor;
			double z=wgs84.getRadius()*(1.0-wgs84E2)/chi*s*scaleFactor;
			glVertex3d(xy*cl,xy*sl,z);
			}
		glVertex3d(0.0,0.0,wgs84.getRadius()*(1.0-wgs84.getFlatteningFactor())*scaleFactor);
		glEnd();
		}
	}

/***********************************************************************
Function to draw a "pin" sticking out of the Earth's surface:
***********************************************************************/

void drawPin(double latitude,double longitude,double height,double scaleFactor)
	{
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	
	double lat=(latitude*Math::Constants<double>::pi)/180.0;
	double s=Math::sin(lat);
	double c=Math::cos(lat);
	double r=a*(1.0-f*s*s)*scaleFactor;
	double xy=r*c;
	double lng=(longitude*Math::Constants<double>::pi)/180.0;
	double cl=Math::cos(lng);
	double sl=Math::sin(lng);
	double x1=xy*cl;
	double y1=xy*sl;
	double z1=r*s;
	double nx=(1.0-3.0*f*s*s)*c*cl;
	double ny=(1.0-3.0*f*s*s)*c*sl;
	double nz=(1.0+3.0*f*c*c-f)*s;
	double nl=sqrt(nx*nx+ny*ny+nz*nz);
	double x2=x1+nx*height/nl;
	double y2=y1+ny*height/nl;
	double z2=z1+nz*height/nl;
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(float(x1),float(y1),float(z1));
	glVertex3f(float(x2),float(y2),float(z2));
	glEnd();
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	glVertex3f(float(x2),float(y2),float(z2));
	glEnd();
	}

/***********************************************************************
Instantiate all template function for common template parameters:
***********************************************************************/

template void calcSurfacePos<float>(float,float,double,float[3]);
template void calcSurfacePos<double>(double,double,double,double[3]);

template void calcDepthPos<float>(float,float,float,double,float[3]);
template void calcDepthPos<double>(double,double,double,double,double[3]);

template void calcRadiusPos<float>(float,float,float,double,float[3]);
template void calcRadiusPos<double>(double,double,double,double,double[3]);
