/***********************************************************************
ModelTracker - Class to calculate the position and orientation of rigid
3D models based on projected images of the models using the POSIT or
SoftPOSIT algorithms.
Copyright (c) 2014 Oliver Kreylos

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

#define GEOMETRY_NONSTANDARD_TEMPLATES

#include "ModelTracker.h"

#include <iostream>
#include <Misc/SizedTypes.h>
#include <IO/Directory.h>
#include <IO/File.h>
#include <Math/Math.h>
#include <Geometry/HVector.h>
#include <Geometry/PCACalculator.h>
#include <Geometry/OutputOperators.h>

#if 0
#include "epnp.h"
#endif

#include "CameraFitter.h"
#include "LevenbergMarquardtMinimizer.h"

// DEBUGGING
// #include <Geometry/OutputOperators.h>

/*****************************
Methods of class ModelTracker:
*****************************/

ModelTracker::ModelTracker(void)
	:numModelPoints(0),modelPoints(0),mpws(0),
	 maxMatchDist2(Math::sqr(3.0))
	{
	}

ModelTracker::~ModelTracker(void)
	{
	delete[] modelPoints;
	delete[] mpws;
	}

void ModelTracker::setModel(unsigned int newNumModelPoints,const ModelTracker::Point newModelPoints[])
	{
	/* Copy the given model point array: */
	delete[] modelPoints;
	numModelPoints=newNumModelPoints;
	modelPoints=new Point[numModelPoints];
	for(unsigned int i=0;i<numModelPoints;++i)
		modelPoints[i]=newModelPoints[i];
	
	#if 0 // Not using POSIT!
	
	/* Create the homogeneous weight array: */
	mpws=new double[numModelPoints];
	
	/* Pre-calculate the model matrix for POSIT: */
	Math::Matrix modelMat(4,4,0.0);
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		{
		for(int i=0;i<3;++i)
			{
			for(int j=0;j<3;++j)
				modelMat(i,j)+=modelPoints[mpi][i]*modelPoints[mpi][j];
			modelMat(i,3)+=modelPoints[mpi][i];
			}
		for(int j=0;j<3;++j)
			modelMat(3,j)+=modelPoints[mpi][j];
		modelMat(3,3)+=1.0;
		}
	
	/* Invert the model matrix: */
	invModelMat=modelMat.inverseFullPivot();
	
	#endif
	}

void ModelTracker::loadCameraIntrinsics(const IO::Directory& directory,const char* intrinsicsFileName)
	{
	IO::FilePtr intrinsicsFile=directory.openFile(intrinsicsFileName);
	intrinsicsFile->setEndianness(Misc::LittleEndian);
	
	/* Read the raw intrinsic calibration parameters: */
	Misc::Float64 params[5];
	intrinsicsFile->read(params,5);
	std::cout<<"Camera intrinsics: "<<params[0]<<", "<<params[1]<<", "<<params[2]<<", "<<params[3]<<", "<<params[4]<<std::endl;
	
	/* Create the full camera projection matrix: */
	Projection::Matrix& pm=projection.getMatrix();
	pm=Projection::Matrix::zero;
	pm(0,0)=params[0];
	pm(0,1)=params[1];
	pm(0,2)=params[2];
	pm(1,1)=params[3];
	pm(1,2)=params[4];
	pm(2,2)=1.0;
	pm(3,2)=1.0;
	
	/* Derive the simplified projection used by POSIT: */
	f=Math::sqrt(params[0]*params[3]);
	
	/* Create the image transformation: */
	ImgTransform::Matrix& im=imgTransform.getMatrix();
	im=ImgTransform::Matrix::zero;
	im(0,0)=-params[0]/f;
	im(0,1)=-params[1]/f;
	im(0,2)=params[2];
	im(1,1)=-params[3]/f;
	im(1,2)=params[4];
	imgTransform.doInvert();
	}

void ModelTracker::setMaxMatchDist(ModelTracker::Scalar newMaxMatchDist)
	{
	/* Set the squared max match distance: */
	maxMatchDist2=Math::sqr(newMaxMatchDist);
	}

ModelTracker::Transform ModelTracker::position(const ModelTracker::ImgPoint imagePoints[],const ModelTracker::Transform::Rotation& orientation) const
	{
	/* Build the least-squares linear system: */
	Math::Matrix ata(3,3,0.0);
	Math::Matrix atb(3,1,0.0);
	const Projection::Matrix& pm=projection.getMatrix();
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		{
		/* Transform the model point with the known orientation and projection matrix: */
		Projection::HVector pmp=projection.transform(Projection::HVector(orientation.transform(modelPoints[mpi])));
		
		/* Create the two equations for the model/image point pair: */
		double eq[2][4];
		for(int i=0;i<2;++i)
			{
			for(int j=0;j<3;++j)
				eq[i][j]=pm(i,j)-imagePoints[mpi][i]*pm(3,j);
			eq[i][3]=imagePoints[mpi][i]*pmp[3]-pmp[i];
			}
		
		/* Add the two equations to the least-squares system: */
		for(int i=0;i<3;++i)
			{
			for(int j=0;j<3;++j)
				ata(i,j)+=eq[0][i]*eq[0][j]+eq[1][i]*eq[1][j];
			atb(i)+=eq[0][i]*eq[0][3]+eq[1][i]*eq[1][3];
			}
		}
	
	/* Solve the least-squares system: */
	Math::Matrix x=atb.divideFullPivot(ata);
	
	/* Return the result transformation: */
	return Transform(Transform::Vector(x(0),x(1),x(2)),orientation);
	}

ModelTracker::Transform ModelTracker::posit(ModelTracker::ImgPoint imagePoints[],unsigned int maxNumIterations)
	{
	/* Pre-transform the image points by the image transformation: */
	for(unsigned int ipi=0;ipi<numModelPoints;++ipi)
		imagePoints[ipi]=imgTransform.transform(imagePoints[ipi]);
	
	/* Assign initial homogeneous weights to the model points: */
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		mpws[mpi]=1.0;
	
	/* Iterate until convergence: */
	Vector r1,r2,t;
	for(unsigned int iteration=0;iteration<maxNumIterations;++iteration)
		{
		/*******************************************************************
		Estimate the object's pose by approximating the perspective
		projection with a scaled orthographic projection:
		*******************************************************************/
		
		/* Build the least-squares linear system: */
		Math::Matrix atb(4,2,0.0);
		for(unsigned int pi=0;pi<numModelPoints;++pi)
			{
			/* Right-hand side matrix contains image-space point positions multiplied by estimated homogeneous weights: */
			for(int i=0;i<3;++i)
				for(int j=0;j<2;++j)
					atb(i,j)+=modelPoints[pi][i]*imagePoints[pi][j]*mpws[pi];
			for(int j=0;j<2;++j)
				atb(3,j)+=imagePoints[pi][j]*mpws[pi];
			}
		
		/* Solve the least-squares linear system: */
		Math::Matrix x=invModelMat*atb;
		
		/* Calculate the scale factor and the full rotation matrix and translation vector: */
		for(int i=0;i<3;++i)
			{
			r1[i]=x(i,0);
			r2[i]=x(i,1);
			}
		double s1=r1.mag();
		double s2=r2.mag();
		
		/* Orthogonalize the orientation vectors with minimum displacement: */
		Vector r3=Geometry::normalize(r1^r2);
		Vector mid=r1/s1+r2/s2;
		mid/=mid.mag()*Math::sqrt(2.0);
		Vector mid2=r3^mid;
		r1=mid-mid2;
		r2=mid+mid2;
		
		double s=Math::sqrt(s1*s2);
		t[0]=x(3,0)/s;
		t[1]=x(3,1)/s;
		t[2]=-f/s;
		
		/* Update the homogeneous weights of the object points: */
		for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
			mpws[mpi]=(r3*modelPoints[mpi])/t[2]+1.0;
		}
	
	/* Return the result transformation: */
	return Transform(t,Geometry::invert(Transform::Rotation::fromBaseVectors(r1,r2)));
	}

#if 0

ModelTracker::Transform ModelTracker::external_epnp(const ModelTracker::ImgPoint imagePoints[])
	{
	class epnp pnp;
	const Projection::Matrix& pm=projection.getMatrix();
	pnp.set_internal_parameters(pm(0,2),pm(1,2),pm(0,0),pm(1,1),pm(0,1));
	pnp.set_maximum_number_of_correspondences(numModelPoints);
	pnp.reset_correspondences();
	
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		pnp.add_correspondence(modelPoints[mpi][0],modelPoints[mpi][1],modelPoints[mpi][2],imagePoints[mpi][0],imagePoints[mpi][1]);
	
	double rotm[3][3];
	Geometry::Vector<double,3> trans;
	double err2=pnp.compute_pose(rotm,trans.getComponents());
	Geometry::Matrix<double,3,3> rotmm=Geometry::Matrix<double,3,3>::fromRowMajor(&rotm[0][0]);
	Transform::Rotation rot=Transform::Rotation::fromMatrix(rotmm);
	
	return Transform(trans,rot);
	}

#endif

#define EPNP_DEBUG 0

ModelTracker::Transform ModelTracker::epnp(const ModelTracker::ImgPoint imagePoints[])
	{
	/*********************************************************************
	Step 1: Calculate four control points enveloping the model points by
	running Principal Component Analysis on the set of model points.
	*********************************************************************/
	
	Point cps[4]; // The four control points
	Geometry::Matrix<Scalar,3,3> cpm; // The matrix to calculate barycentric control point weights for model points
	
	Geometry::PCACalculator<3> pca;
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		pca.accumulatePoint(modelPoints[mpi]);
	
	/* First control point is model point set's centroid: */
	cps[0]=Point(pca.calcCentroid());
	
	/* Next three control points are aligned with the model point set's principal axes: */
	pca.calcCovariance();
	double pcaEvals[3];
	pca.calcEigenvalues(pcaEvals);
	Vector pcaEvecs[3];
	for(int i=0;i<3;++i)
		pcaEvecs[i]=Vector(pca.calcEigenvector(pcaEvals[i]));
	if((pcaEvecs[0]^pcaEvecs[1])*pcaEvecs[2]<0.0)
		{
		// std::cout<<"World control points are left-handed!"<<std::endl;
		pcaEvecs[2]*=Scalar(-1);
		}
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			cpm(i,j)=pcaEvecs[i][j];
	Transform worldToModel(Vector(cpm*(Point::origin-cps[0])),Transform::Rotation::fromMatrix(cpm));
	for(int i=0;i<3;++i)
		{
		Scalar scale=Scalar(Math::sqrt(pcaEvals[i])); // Scale principal components to the model
		cps[1+i]=cps[0]+pcaEvecs[i]*scale; // Should add a check for zero Eigenvalue here
		
		/* Calculate the inverse control point matrix directly, as it's orthogonal: */
		for(int j=0;j<3;++j)
			cpm(i,j)/=scale;
		}
	
	#if EPNP_DEBUG
	std::cout<<"Principal components: "<<Math::sqrt(pcaEvals[0])<<", "<<Math::sqrt(pcaEvals[1])<<", "<<Math::sqrt(pcaEvals[2])<<std::endl;
	std::cout<<"Control points: "<<cps[0]<<", "<<cps[1]<<", "<<cps[2]<<", "<<cps[3]<<std::endl;
	#endif
	
	/*********************************************************************
	Step 2: Calculate the linear system M^T*M.
	*********************************************************************/
	
	Math::Matrix mtm(12,12,0.0);
	const Projection::Matrix& pm=projection.getMatrix();
	Scalar fu=pm(0,0);
	Scalar sk=pm(0,1);
	Scalar uc=pm(0,2);
	Scalar fv=pm(1,1);
	Scalar vc=pm(1,2);
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		{
		/* Calculate the model point's control point weights: */
		Vector mpc=modelPoints[mpi]-cps[0];
		Scalar alphai[4];
		for(int i=0;i<3;++i)
			alphai[1+i]=cpm(i,0)*mpc[0]+cpm(i,1)*mpc[1]+cpm(i,2)*mpc[2];
		alphai[0]=Scalar(1)-alphai[1]-alphai[2]-alphai[3];
		
		/* Calculate the coefficients of the model point / image point association's two linear equations: */
		double eqs[2][12];
		for(int i=0;i<4;++i)
			{
			/* Equation for image point's u coordinate: */
			eqs[0][i*3+0]=alphai[i]*fu;
			eqs[0][i*3+1]=alphai[i]*sk;
			eqs[0][i*3+2]=alphai[i]*(uc-imagePoints[mpi][0]);
			
			/* Equation for image point's v coordinate: */
			eqs[1][i*3+0]=0.0;
			eqs[1][i*3+1]=alphai[i]*fv;
			eqs[1][i*3+2]=alphai[i]*(vc-imagePoints[mpi][1]);
			}
		
		/* Enter the model point / image point association's two linear equations into the least-squares matrix: */
		for(unsigned int i=0;i<12;++i)
			for(unsigned int j=0;j<12;++j)
				mtm(i,j)+=eqs[0][i]*eqs[0][j]+eqs[1][i]*eqs[1][j];
		}
	
	/*********************************************************************
	Step 3: Find four potential solutions to the pose estimation problem
	by assuming that either 1, 2, 3, or 4 Eigenvalues of the least-squares
	linear system are zero or very small, and calculate a scale-preserving
	transformation for all cases. Then pick the one that minimizes
	reprojection error.
	*********************************************************************/
	
	/* Get the full set of eigenvalues and eigenvectors of the least-squares matrix: */
	std::pair<Math::Matrix,Math::Matrix> qe=mtm.jacobiIteration();
	
	/* Find the indices of the four smallest Eigenvalues: */
	unsigned int evIndices[12];
	for(unsigned int i=0;i<12;++i)
		evIndices[i]=i;
	for(unsigned int i=0;i<4;++i)
		{
		/* Find the next-smallest Eigenvalue: */
		int minI=i;
		double minE=Math::abs(qe.second(evIndices[i],0));
		for(unsigned int j=i+1;j<12;++j)
			{
			double e=Math::abs(qe.second(evIndices[j],0));
			if(minE>e)
				{
				minI=j;
				minE=e;
				}
			}
		
		/* Move the found Eigenvalue to the front: */
		int ti=evIndices[i];
		evIndices[i]=evIndices[minI];
		evIndices[minI]=ti;
		}
	
	#if EPNP_DEBUG
	std::cout<<"MTM Eigenvalues:";
	for(unsigned int i=0;i<12;++i)
		std::cout<<' '<<qe.second(evIndices[i],0);
	std::cout<<std::endl;
	#endif
	
	/* Calculate the pairwise distances between the four control points in world space: */
	Scalar cpDists[6];
	cpDists[0]=Geometry::sqrDist(cps[0],cps[1]);
	cpDists[1]=Geometry::sqrDist(cps[0],cps[2]);
	cpDists[2]=Geometry::sqrDist(cps[0],cps[3]);
	cpDists[3]=Geometry::sqrDist(cps[1],cps[2]);
	cpDists[4]=Geometry::sqrDist(cps[1],cps[3]);
	cpDists[5]=Geometry::sqrDist(cps[2],cps[3]);
	
	/*********************************************************************
	Step 3a: Calculate the solution vector for the assumed case of one
	very small Eigenvalue:
	*********************************************************************/
	
	/* Extract the positions of the four control points in camera space from the smallest Eigenvector: */
	Point cpcs[4];
	for(unsigned int cpi=0;cpi<4;++cpi)
		for(unsigned int i=0;i<3;++i)
			cpcs[cpi][i]=Scalar(qe.first(cpi*3+i,evIndices[0]));
	
	/* Calculate the pairwise distances between the four control points in camera space: */
	Scalar cpcDists[6];
	cpcDists[0]=Geometry::sqrDist(cpcs[0],cpcs[1]);
	cpcDists[1]=Geometry::sqrDist(cpcs[0],cpcs[2]);
	cpcDists[2]=Geometry::sqrDist(cpcs[0],cpcs[3]);
	cpcDists[3]=Geometry::sqrDist(cpcs[1],cpcs[2]);
	cpcDists[4]=Geometry::sqrDist(cpcs[1],cpcs[3]);
	cpcDists[5]=Geometry::sqrDist(cpcs[2],cpcs[3]);
	
	/* Calculate the scaling factor: */
	Scalar betaCounter(0);
	Scalar betaDenominator(0);
	for(int i=0;i<6;++i)
		{
		betaCounter+=Math::sqrt(cpcDists[i]*cpDists[i]);
		betaDenominator+=cpcDists[i];
		}
	Scalar beta=-betaCounter/betaDenominator;
	#if EPNP_DEBUG
	std::cout<<"Scaling factor: "<<beta<<std::endl;
	#endif
	
	/* Recalculate the camera-space control points: */
	for(int i=0;i<4;++i)
		for(int j=0;j<3;++j)
			cpcs[i][j]*=beta;
	
	/* Check if the control point order was flipped: */
	if(((cpcs[1]-cpcs[0])^(cpcs[2]-cpcs[0]))*(cpcs[3]-cpcs[0])<0.0)
		{
		// std::cout<<"Control points got done flipped"<<std::endl;
		// cpcs[3]=cpcs[0]-(cpcs[3]-cpcs[0]);
		}
	
	#if EPNP_DEBUG
	/* Print the camera-space control points: */
	for(int i=0;i<4;++i)
		std::cout<<"CCP "<<i<<": "<<cpcs[i][0]<<", "<<cpcs[i][1]<<", "<<cpcs[i][2]<<std::endl;
	std::cout<<Math::sqrt((cpcs[1]-cpcs[0])*(cpcs[1]-cpcs[0]));
	std::cout<<' '<<(cpcs[1]-cpcs[0])*(cpcs[2]-cpcs[0]);
	std::cout<<' '<<(cpcs[1]-cpcs[0])*(cpcs[3]-cpcs[0]);
	std::cout<<' '<<Math::sqrt((cpcs[2]-cpcs[0])*(cpcs[2]-cpcs[0]));
	std::cout<<' '<<(cpcs[2]-cpcs[0])*(cpcs[3]-cpcs[0]);
	std::cout<<' '<<Math::sqrt((cpcs[3]-cpcs[0])*(cpcs[3]-cpcs[0]))<<std::endl;
	
	std::cout<<Math::sqrt((cps[1]-cps[0])*(cps[1]-cps[0]));
	std::cout<<' '<<(cps[1]-cps[0])*(cps[2]-cps[0]);
	std::cout<<' '<<(cps[1]-cps[0])*(cps[3]-cps[0]);
	std::cout<<' '<<Math::sqrt((cps[2]-cps[0])*(cps[2]-cps[0]));
	std::cout<<' '<<(cps[2]-cps[0])*(cps[3]-cps[0]);
	std::cout<<' '<<Math::sqrt((cps[3]-cps[0])*(cps[3]-cps[0]))<<std::endl;
	#endif
	
	/* Calculate the transformation from camera control point space to camera space: */
	Vector cbase[3];
	for(int i=0;i<3;++i)
		cbase[i]=cpcs[1+i]-cpcs[0];
	cbase[0].normalize();
	cbase[1]-=(cbase[0]*cbase[1])*cbase[0];
	cbase[1].normalize();
	cbase[2]-=(cbase[0]*cbase[2])*cbase[0];
	cbase[2]-=(cbase[1]*cbase[2])*cbase[1];
	cbase[2].normalize();
	Geometry::Matrix<Scalar,3,3> camera;
	for(int i=0;i<3;++i)
		for(int j=0;j<3;++j)
			camera(i,j)=cbase[j][i];
	Transform modelToCamera(cpcs[0]-Point::origin,Transform::Rotation::fromMatrix(camera));
	
	return modelToCamera*worldToModel;
	}

ModelTracker::Transform ModelTracker::levenbergMarquardt(const ModelTracker::ImgPoint imagePoints[],const Transform& initialTransform,unsigned int maxNumIterations)
	{
	/* Get the camera's intrinsic parameters: */
	const Projection::Matrix& pm=projection.getMatrix();
	Scalar fu=pm(0,0);
	Scalar sk=pm(0,1);
	Scalar uc=pm(0,2);
	Scalar fv=pm(1,1);
	Scalar vc=pm(1,2);
	
	/* Create a new camera fitter: */
	CameraFitter cameraFitter(fu,sk,uc,fv,vc);
	cameraFitter.setTrackedObject(numModelPoints,modelPoints);
	cameraFitter.setTransform(initialTransform);
	for(unsigned int i=0;i<numModelPoints;++i)
		cameraFitter.setPixel(i,imagePoints[i]);
	
	/* Create a Levenberg-Marquardt optimizer: */
	LevenbergMarquardtMinimizer<CameraFitter> lmm;
	lmm.maxNumIterations=maxNumIterations;
	Scalar finalResidual=lmm.minimize(cameraFitter);
	
	/* Return the result transformation: */
	return cameraFitter.getTransform();
	}

ModelTracker::Transform ModelTracker::softPosit(unsigned int numImagePoints,ModelTracker::ImgPoint imagePoints[],const Transform& initialTransform)
	{
	typedef Transform::Vector Vector;
	
	/* Pre-transform the image points by the image transformation: */
	for(unsigned int ipi=0;ipi<numImagePoints;++ipi)
		imagePoints[ipi]=imgTransform.transform(imagePoints[ipi]);
	
	/* Assign initial homogeneous weights to the model points: */
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		mpws[mpi]=1.0;
	
	/* Create the assignment matrix: */
	Math::Matrix m(numImagePoints+1,numModelPoints+1);
	
	/* Initialize the "slack" rows and columns: */
	double gamma=1.0/double(Math::max(numImagePoints,numModelPoints)+1);
	for(unsigned int ipi=0;ipi<numImagePoints;++ipi)
		m(ipi,numModelPoints)=gamma;
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		m(numImagePoints,mpi)=gamma;
	m(numImagePoints,numModelPoints)=gamma;
	
	/* Initialize the pose vectors: */
	Transform::Rotation inverseOrientation=Geometry::invert(initialTransform.getRotation());
	Vector r1=inverseOrientation.getDirection(0);
	Vector r2=inverseOrientation.getDirection(1);
	Vector t=initialTransform.getTranslation();
	double s=-f/t[2];
	
	/* Perform the deterministic annealing loop: */
	for(double beta=0.005;beta<=0.5;beta*=1.025)
		{
		/* Create the initial assignment matrix based on squared distances between projected object points and image points: */
		for(unsigned int ipi=0;ipi<numImagePoints;++ipi)
			for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
				{
				double d2=Math::sqr((r1*modelPoints[mpi]+t[0])*s-mpws[mpi]*imagePoints[ipi][0])
				         +Math::sqr((r2*modelPoints[mpi]+t[1])*s-mpws[mpi]*imagePoints[ipi][1]);
				m(ipi,mpi)=Math::exp(-beta*(d2-maxMatchDist2));
				
				// DEBUGGING
				// std::cout<<' '<<d2;
				}
		// DEBUGGING
		// std::cout<<std::endl;
		
		/* Normalize the assignment matrix using Sinkhorn's method: */
		double rowMaxDelta,colMaxDelta;
		do
			{
			/* Normalize image point rows: */
			rowMaxDelta=0.0;
			for(unsigned int ipi=0;ipi<numImagePoints;++ipi)
				{
				/* Calculate the row sum: */
				double rowSum=0.0;
				for(unsigned int mpi=0;mpi<numModelPoints+1;++mpi)
					rowSum+=m(ipi,mpi);
				
				/* Normalize the row: */
				for(unsigned int mpi=0;mpi<numModelPoints+1;++mpi)
					{
					double oldM=m(ipi,mpi);
					m(ipi,mpi)/=rowSum;
					rowMaxDelta=Math::max(rowMaxDelta,Math::abs(m(ipi,mpi)-oldM));
					}
				}
			
			/* Normalize model point columns: */
			colMaxDelta=0.0;
			for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
				{
				/* Calculate the column sum: */
				double colSum=0.0;
				for(unsigned int ipi=0;ipi<numImagePoints+1;++ipi)
					colSum+=m(ipi,mpi);
				
				/* Normalize the column: */
				for(unsigned int ipi=0;ipi<numImagePoints+1;++ipi)
					{
					double oldM=m(ipi,mpi);
					m(ipi,mpi)/=colSum;
					colMaxDelta=Math::max(colMaxDelta,Math::abs(m(ipi,mpi)-oldM));
					}
				}
			}
		while(rowMaxDelta+colMaxDelta>1.0e-4);
		
		/* Compute the left-hand side of the pose alignment linear system: */
		Math::Matrix lhs(4,4,0.0);
		for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
			{
			const Point& mp=modelPoints[mpi];
			
			/* Calculate the linear equation weight for the model point: */
			double mpWeight=0.0;
			for(unsigned int ipi=0;ipi<numImagePoints;++ipi)
				mpWeight+=m(ipi,mpi);
			
			/* Enter the model point into the pose alignment linear system: */
			for(int i=0;i<3;++i)
				{
				for(int j=0;j<3;++j)
					lhs(i,j)+=mp[i]*mp[j]*mpWeight;
				lhs(i,3)+=mp[i]*mpWeight;
				}
			for(int j=0;j<3;++j)
				lhs(3,j)+=mp[j]*mpWeight;
			lhs(3,3)+=mpWeight;
			}
		
		/* Invert the left-hand side matrix: */
		Math::Matrix lhsInv;
		try
			{
			lhsInv=lhs.inverseFullPivot();
			}
		catch(Math::Matrix::RankDeficientError)
			{
			std::cerr<<"Left-hand side matrix is rank deficient"<<std::endl;
			for(int i=0;i<4;++i)
				{
				for(int j=0;j<4;++j)
					std::cerr<<"  "<<lhs(i,j);
				std::cerr<<std::endl;
				}
			
			std::cerr<<"Assignment matrix:"<<std::endl;
			for(unsigned int i=0;i<=numImagePoints;++i)
				{
				for(unsigned int j=0;j<=numModelPoints;++j)
					std::cerr<<"  "<<m(i,j);
				std::cerr<<std::endl;
				}
			
			return Transform::identity;
			}
		
		/* Perform a fixed number of iterations of POSIT: */
		for(unsigned int iteration=0;iteration<2U;++iteration)
			{
			/* Compute the right-hand side of the pose alignment linear system: */
			Math::Matrix rhs(4,2,0.0);
			for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
				{
				const Point& mp=modelPoints[mpi];
				
				/* Enter the model point into the pose alignment linear system: */
				double sumX=0.0;
				double sumY=0.0;
				for(unsigned int ipi=0;ipi<numImagePoints;++ipi)
					{
					sumX+=m(ipi,mpi)*imagePoints[ipi][0];
					sumY+=m(ipi,mpi)*imagePoints[ipi][1];
					}
				sumX*=mpws[mpi];
				sumY*=mpws[mpi];
				
				for(int i=0;i<3;++i)
					{
					rhs(i,0)+=sumX*mp[i];
					rhs(i,1)+=sumY*mp[i];
					}
				rhs(3,0)+=sumX;
				rhs(3,1)+=sumY;
				}
			
			/* Solve the pose alignment system: */
			Math::Matrix pose=lhsInv*rhs;
			for(int i=0;i<3;++i)
				{
				r1[i]=pose(i,0);
				r2[i]=pose(i,1);
				}
			
			/* Orthonormalize the pose vectors: */
			double s1=r1.mag();
			double s2=r2.mag();
			Vector r3=Geometry::normalize(r1^r2);
			Vector mid=r1/s1+r2/s2;
			mid/=mid.mag()*Math::sqrt(2.0);
			Vector mid2=r3^mid;
			r1=mid-mid2;
			r2=mid+mid2;
			s=Math::sqrt(s1*s2);
			t[0]=pose(3,0)/s;
			t[1]=pose(3,1)/s;
			t[2]=-f/s;
			
			/* Update the object points' homogeneous weights: */
			for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
				mpws[mpi]=(r3*modelPoints[mpi])/t[2]+1.0;
			}
		
		// DEBUGGING
		// std::cout<<"Intermediate: "<<Transform(t,Geometry::invert(Transform::Rotation::fromBaseVectors(r1,r2)))<<std::endl;
		}
	
	// DEBUGGING
	std::cerr<<"Final assignment matrix:"<<std::endl;
	for(unsigned int i=0;i<=numImagePoints;++i)
		{
		for(unsigned int j=0;j<=numModelPoints;++j)
			std::cerr<<"  "<<m(i,j);
		std::cerr<<std::endl;
		}
	
	/* Return the result transformation: */
	return Transform(t,Geometry::invert(Transform::Rotation::fromBaseVectors(r1,r2)));
	}

ModelTracker::Scalar ModelTracker::calcReprojectionError(const ModelTracker::ImgPoint imagePoints[],const ModelTracker::Transform& transform) const
	{
	Scalar error2(0);
	for(unsigned int mpi=0;mpi<numModelPoints;++mpi)
		{
		/* Project the transformed model point: */
		ImgPoint ip=project(transform.transform(modelPoints[mpi]));
		error2+=Geometry::sqrDist(ip,imagePoints[mpi]);
		}
	return error2;
	}
