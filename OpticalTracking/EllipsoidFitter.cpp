/***********************************************************************
EllipsoidFitter - Helper class to fit an ellipsoid to a set of 3D points
for sensor calibration and rectification.
Copyright (c) 2013 Oliver Kreylos

This file is part of the optical/inertial sensor fusion tracking
package.

The optical/inertial sensor fusion tracking package is free software;
you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

The optical/inertial sensor fusion tracking package is distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the optical/inertial sensor fusion tracking package; if not, write
to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA 02111-1307 USA
***********************************************************************/

#include "EllipsoidFitter.h"

#include <iostream>
#include <iomanip>
#include <Math/Math.h>
#include <Math/Matrix.h>
#include <GL/gl.h>

namespace std {

/****************************************
Helper function to pretty-print a matrix:
****************************************/

inline ostream& operator<<(ostream& os,const Math::Matrix& m)
	{
	streamsize width=os.width();
	os<<setw(1)<<'/'<<setw(width)<<m(0,0);
	for(unsigned int j=1;j<m.getNumColumns();++j)
		os<<setw(2)<<", "<<setw(width)<<m(0,j);
	os<<setw(1)<<'\\'<<endl;
	for(unsigned int i=1;i<m.getNumRows()-1;++i)
		{
		os<<setw(1)<<'|'<<setw(width)<<m(i,0);
		for(unsigned int j=1;j<m.getNumColumns();++j)
			os<<setw(2)<<", "<<setw(width)<<m(i,j);
		os<<setw(1)<<'|'<<endl;
		}
	os<<setw(1)<<'\\'<<setw(width)<<m(m.getNumRows()-1,0);
	for(unsigned int j=1;j<m.getNumColumns();++j)
		os<<setw(2)<<", "<<setw(width)<<m(m.getNumRows()-1,j);
	os<<setw(1)<<'/'<<endl;
	return os;
	}

}

/********************************
Methods of class EllipsoidFitter:
********************************/

EllipsoidFitter::EllipsoidFitter(void)
	{
	}

EllipsoidFitter::Calibration EllipsoidFitter::calculateFit(void) const
	{
	/*********************************************************************
	First step: Fit a quadric to the point set by least-squares
	minimization based on algebraic distance.
	*********************************************************************/
	
	/* Create the least-squares system: */
	Math::Matrix a(10,10,0.0);
	
	/* Process all points: */
	for(Misc::ChunkedArray<Point>::const_iterator pIt=points.begin();pIt!=points.end();++pIt)
		{
		/* Create the point's associated linear equation: */
		double eq[10];
		eq[0]=(*pIt)[0]*(*pIt)[0];
		eq[1]=2.0*(*pIt)[0]*(*pIt)[1];
		eq[2]=2.0*(*pIt)[0]*(*pIt)[2];
		eq[3]=2.0*(*pIt)[0];
		eq[4]=(*pIt)[1]*(*pIt)[1];
		eq[5]=2.0*(*pIt)[1]*(*pIt)[2];
		eq[6]=2.0*(*pIt)[1];
		eq[7]=(*pIt)[2]*(*pIt)[2];
		eq[8]=2.0*(*pIt)[2];
		eq[9]=1.0;
		
		/* Insert the equation into the least-squares system: */
		for(unsigned int i=0;i<10;++i)
			for(unsigned int j=0;j<10;++j)
				a(i,j)+=eq[i]*eq[j];
		}
	
	/* Find the least-squares system's smallest eigenvalue: */
	std::pair<Math::Matrix,Math::Matrix> qe=a.jacobiIteration();
	unsigned int minEIndex=0;
	double minE=Math::abs(qe.second(0,0));
	for(unsigned int i=1;i<10;++i)
		{
		if(minE>Math::abs(qe.second(i,0)))
			{
			minEIndex=i;
			minE=Math::abs(qe.second(i,0));
			}
		}
	
	/* Create the quadric's defining matrices: */
	Math::Matrix qa(3,3);
	qa(0,0)=qe.first(0,minEIndex);
	qa(0,1)=qe.first(1,minEIndex);
	qa(0,2)=qe.first(2,minEIndex);
	qa(1,0)=qe.first(1,minEIndex);
	qa(1,1)=qe.first(4,minEIndex);
	qa(1,2)=qe.first(5,minEIndex);
	qa(2,0)=qe.first(2,minEIndex);
	qa(2,1)=qe.first(5,minEIndex);
	qa(2,2)=qe.first(7,minEIndex);
	Math::Matrix qb(3,1);
	qb(0)=qe.first(3,minEIndex);
	qb(1)=qe.first(6,minEIndex);
	qb(2)=qe.first(8,minEIndex);
	double qc=qe.first(9,minEIndex);
	
	/* Calculate the quadric's principal axes: */
	qe=qa.jacobiIteration();
	std::cout<<std::fixed<<std::setprecision(6);
	std::cout<<std::setw(9)<<qe.first<<std::endl;
	std::cout<<std::setw(9)<<qe.second<<std::endl<<std::endl;
	std::cout.unsetf(std::ios_base::floatfield);
	
	/* "Complete the square" to calculate the quadric's centroid and radii: */
	Math::Matrix qbp=qb.divideFullPivot(qe.first);
	Math::Matrix cp(3,1);
	for(int i=0;i<3;++i)
		cp(i)=-qbp(i)/qe.second(i);
	Math::Matrix c=qe.first*cp;
	std::cout<<"Centroid: "<<c(0)<<", "<<c(1)<<", "<<c(2)<<std::endl;
	double rhs=-qc;
	for(int i=0;i<3;++i)
		rhs+=Math::sqr(qbp(i))/qe.second(i);
	double radii[3];
	for(int i=0;i<3;++i)
		radii[i]=Math::sqrt(rhs/qe.second(i));
	std::cout<<"Radii: "<<radii[0]<<", "<<radii[1]<<", "<<radii[2]<<std::endl;
	Scalar averageRadius=Math::pow(radii[0]*radii[1]*radii[2],1.0/3.0);
	std::cout<<"Average radius: "<<averageRadius<<std::endl;
		
	/* Calculate the calibration matrix: */
	Math::Matrix ellP(4,4,1.0);
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			ellP(i,j)=qe.first(i,j);
	Math::Matrix ellScale(4,4,1.0);
	for(int i=0;i<3;++i)
		ellScale(i,i)=averageRadius/radii[i];
	Math::Matrix ell=ellP;
	ell.makePrivate();
	for(int i=0;i<3;++i)
		ell(i,3)=c(i);
	Math::Matrix ellInv=ell.inverseFullPivot();
	Math::Matrix calib=ellP*ellScale*ellInv;
	Matrix result;
	for(int i=0;i<3;++i)
		for(int j=0;j<4;++j)
			result(i,j)=calib(i,j);
	return Calibration(result,averageRadius);
	}

void EllipsoidFitter::glRenderAction(GLContextData& contextData) const
	{
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	/* Draw the points inside each array chunk as a vertex array: */
	PointRenderFunctor prf;
	points.forEachChunk(prf);
	
	glPopClientAttrib();
	}
