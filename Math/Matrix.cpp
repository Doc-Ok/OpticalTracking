/***********************************************************************
Matrix - Class to represent double-valued matrices of dynamic sizes.
Copyright (c) 2000-2014 Oliver Kreylos

This file is part of the Templatized Math Library (Math).

The Templatized Math Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Math Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Math Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <Math/Matrix.h>

#include <string.h>
#include <stdexcept>
#include <Math/Math.h>
#include <Math/Constants.h>

namespace Math {

namespace {

/****************
Helper functions:
****************/

/* Perform Gaussian elimination with column pivoting on an extended matrix: */

void gaussColumnPivoting(unsigned int numRows,unsigned int numColumns,double* m)
	{
	for(unsigned int step=0;step<numRows-1;++step)
		{
		/* Find the column pivot: */
		double* pPtr=m+(step*numColumns+step);
		double pivot=Math::abs(*pPtr);
		unsigned int pivotRow=step;
		pPtr+=numColumns;
		for(unsigned int i=step+1;i<numRows;++i,pPtr+=numColumns)
			{
			double val=Math::abs(*pPtr);
			if(pivot<val)
				{
				pivot=val;
				pivotRow=i;
				}
			}
		
		/* Check for rank deficiency: */
		if(pivot==0.0)
			throw Matrix::RankDeficientError();
		
		/* Swap current and pivot rows if necessary: */
		if(pivotRow!=step)
			{
			/* Swap rows step and pivotRow: */
			double* r1Ptr=m+(step*numColumns+step);
			double* r2Ptr=m+(pivotRow*numColumns+step);
			for(unsigned int j=step;j<numColumns;++j,++r1Ptr,++r2Ptr)
				std::swap(*r1Ptr,*r2Ptr);
			}
		
		/* Combine all rows with the current row: */
		for(unsigned int i=step+1;i<numRows;++i)
			{
			/* Combine rows i and step: */
			double* r1Ptr=m+(step*numColumns+step);
			double* r2Ptr=m+(i*numColumns+step);
			double factor=-*r2Ptr/(*r1Ptr);
			++r1Ptr;
			++r2Ptr;
			for(unsigned int j=step+1;j<numColumns;++j,++r1Ptr,++r2Ptr)
				*r2Ptr+=(*r1Ptr)*factor;
			}
		}
	}

/* Perform Gaussian elimination with full pivoting on an extended matrix; returns row rank of matrix: */

unsigned int gaussFullPivoting(unsigned int numRows,unsigned int numColumns,double* m,unsigned int maxPivotColumn,unsigned int columnIndices[],int& swapSign)
	{
	/* Perform Gaussian elimination with full pivoting: */
	swapSign=1;
	unsigned int step;
	for(step=0;step<numRows;++step)
		{
		/* Find the full pivot: */
		double pivot=Math::abs(m[step*numColumns+step]);
		unsigned int pivotRow=step;
		unsigned int pivotCol=step;
		for(unsigned int i=step;i<numRows;++i)
			{
			double* mPtr=m+(i*numColumns+step);
			for(unsigned int j=step;j<maxPivotColumn;++j,++mPtr)
				{
				double val=Math::abs(*mPtr);
				if(pivot<val)
					{
					pivot=val;
					pivotRow=i;
					pivotCol=j;
					}
				}
			}
		
		/* Bail out if the rest of the matrix is all zeros: */
		if(pivot==0.0)
			break;
		
		/* Swap current and pivot rows if necessary: */
		if(pivotRow!=step)
			{
			/* Swap rows step and pivotRow: */
			double* r1Ptr=m+step*numColumns+step;
			double* r2Ptr=m+pivotRow*numColumns+step;
			for(unsigned int j=step;j<numColumns;++j,++r1Ptr,++r2Ptr)
				std::swap(*r1Ptr,*r2Ptr);
			
			swapSign=-swapSign;
			}
		
		/* Swap current and pivot columns if necessary: */
		if(pivotCol!=step)
			{
			/* Swap columns step and pivotCol: */
			double* c1Ptr=m+step;
			double* c2Ptr=m+pivotCol;
			for(unsigned int i=0;i<numRows;++i,c1Ptr+=numColumns,c2Ptr+=numColumns)
				std::swap(*c1Ptr,*c2Ptr);
			
			/* Permute the column index array: */
			std::swap(columnIndices[step],columnIndices[pivotCol]);
			
			swapSign=-swapSign;
			}
		
		/* Combine all rows with the current row: */
		for(unsigned int i=step+1;i<numRows;++i)
			{
			/* Combine rows i and step: */
			double* r1Ptr=m+(step*numColumns+step);
			double* r2Ptr=m+(i*numColumns+step);
			double factor=-*r2Ptr/(*r1Ptr);
			++r1Ptr;
			++r2Ptr;
			for(unsigned int j=step+1;j<numColumns;++j,++r1Ptr,++r2Ptr)
				*r2Ptr+=*r1Ptr*factor;
			}
		}
	
	/* Return the last step index, i.e., the matrix row rank: */
	return step;
	}

}

/***********************
Methods of class Matrix:
***********************/

void Matrix::share(double* newM)
	{
	if(newM!=0)
		{
		/* Increase the shared element array's number of owners: */
		++reinterpret_cast<unsigned int*>(newM)[-1];
		}
	
	/* Set the shared element array: */
	m=newM;
	}

void Matrix::release(void)
	{
	if(m!=0)
		{
		/* Check if the matrix elements are shared: */
		if(reinterpret_cast<unsigned int*>(m)[-1]>1)
			{
			/* Reduce the shared element array's number of owners: */
			--reinterpret_cast<unsigned int*>(m)[-1];
			}
		else
			{
			/* Delete the element array: */
			delete[] (m-1);
			}
		}
	}

Matrix::Matrix(unsigned int sNumRows,unsigned int sNumColumns,double* sElements)
	:numRows(sNumRows),numColumns(sNumColumns),
	 m((new double[numRows*numColumns+1])+1)
	{
	/* Take ownership of the new element array: */
	reinterpret_cast<unsigned int*>(m)[-1]=1;
	
	if(sElements!=0)
		{
		/* Copy the given element array: */
		memcpy(m,sElements,numRows*numColumns*sizeof(double));
		}
	}

Matrix::Matrix(unsigned int sNumRows,unsigned int sNumColumns,double sDiag)
	:numRows(sNumRows),numColumns(sNumColumns),
	 m((new double[numRows*numColumns+1])+1)
	{
	/* Take ownership of the new element array: */
	reinterpret_cast<unsigned int*>(m)[-1]=1;
	
	/* Initialize the matrix: */
	double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr)
			*mPtr=i==j?sDiag:0.0;
	}

Matrix Matrix::getRow(unsigned int rowIndex) const
	{
	Matrix result(1,numColumns);
	const double* mPtr=m+rowIndex*numColumns;
	for(unsigned int j=0;j<numColumns;++j,++mPtr)
		result.m[j]=*mPtr;
	return result;
	}

Matrix Matrix::getColumn(unsigned int columnIndex) const
	{
	Matrix result(numRows,1);
	const double* mPtr=m+columnIndex;
	for(unsigned int i=0;i<numRows;++i,mPtr+=numColumns)
		result.m[i]=*mPtr;
	return result;
	}

bool Matrix::operator==(const Matrix& other) const
	{
	if(numRows!=other.numRows||numColumns!=other.numColumns)
		return false;
	
	const double* mPtr=m;
	const double* oPtr=other.m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr,++oPtr)
			if(*mPtr!=*oPtr)
				return false;
	
	return true;
	}

bool Matrix::operator!=(const Matrix& other) const
	{
	if(numRows!=other.numRows||numColumns!=other.numColumns)
		return true;
	
	const double* mPtr=m;
	const double* oPtr=other.m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr,++oPtr)
			if(*mPtr!=*oPtr)
				return true;
	
	return false;
	}

void Matrix::makePrivate(void)
	{
	/* Check if the matrix elements are shared: */
	if(m!=0&&reinterpret_cast<unsigned int*>(m)[-1]>1)
		{
		/* Create a private element array: */
		double* newM=(new double[numRows*numColumns+1])+1;
		reinterpret_cast<unsigned int*>(newM)[-1]=1;
		
		/* Copy the shared element array: */
		memcpy(newM,m,numRows*numColumns*sizeof(double));
		
		/* Reduce the shared element array's number of owners: */
		--reinterpret_cast<unsigned int*>(m)[-1];
		
		/* Set the private element array: */
		m=newM;
		}
	}

void Matrix::setRow(unsigned int rowIndex,const Matrix& row)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m+rowIndex*numColumns;
	for(unsigned int j=0;j<numColumns;++j,++mPtr)
		*mPtr=row.m[j];
	}

void Matrix::setColumn(unsigned int columnIndex,const Matrix& column)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m+columnIndex;
	for(unsigned int i=0;i<numRows;++i,mPtr+=numColumns)
		*mPtr=column.m[i];
	}

void Matrix::swapRows(unsigned int rowIndex1,unsigned int rowIndex2)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr1=m+rowIndex1*numColumns;
	double* mPtr2=m+rowIndex2*numColumns;
	for(unsigned int j=0;j<numColumns;++j,++mPtr1,++mPtr2)
		std::swap(*mPtr1,*mPtr2);
	}

void Matrix::swapColumns(unsigned int columnIndex1,unsigned int columnIndex2)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr1=m+columnIndex1;
	double* mPtr2=m+columnIndex2;
	for(unsigned int i=0;i<numRows;++i,mPtr1+=numColumns,mPtr2+=numColumns)
		std::swap(*mPtr1,*mPtr2);
	}

void Matrix::scaleRow(unsigned int rowIndex,double factor)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m+rowIndex*numColumns;
	for(unsigned int j=0;j<numColumns;++j,++mPtr)
		*mPtr*=factor;
	}

void Matrix::scaleColumn(unsigned int columnIndex,double factor)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m+columnIndex;
	for(unsigned int i=0;i<numRows;++i,mPtr+=numColumns)
		*mPtr*=factor;
	}

void Matrix::combineRows(unsigned int rowIndex1,double factor,unsigned int rowIndex2)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr1=m+rowIndex1*numColumns;
	double* mPtr2=m+rowIndex2*numColumns;
	for(unsigned int j=0;j<numColumns;++j,++mPtr1,++mPtr2)
		*mPtr2+=*mPtr1*factor;
	}

Matrix Matrix::operator-(void) const
	{
	Matrix result(numRows,numColumns);
	const double* mPtr=m;
	double* rPtr=result.m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr,++rPtr)
			*rPtr=-*mPtr;
	return result;
	}

Matrix& Matrix::operator+=(const Matrix& other)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m;
	const double* oPtr=other.m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr,++oPtr)
			*mPtr+=*oPtr;
	return *this;
	}

Matrix& Matrix::operator-=(const Matrix& other)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m;
	const double* oPtr=other.m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr,++oPtr)
			*mPtr-=*oPtr;
	return *this;
	}

Matrix Matrix::inverse(void) const
	{
	/* Create the extended matrix: */
	double* ext=new double[numRows*numRows*2];
	double* extPtr=ext;
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		{
		/* Copy the matrix' elements: */
		for(unsigned int j=0;j<numRows;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		
		/* Create an identity matrix: */
		for(unsigned int j=0;j<numRows;++j,++extPtr)
			*extPtr=i==j?1.0:0.0;
		}
	
	/* Perform Gauss elimination with column pivoting on the extended matrix: */
	gaussColumnPivoting(numRows,numRows*2,ext);
	
	/* Create the result matrix by backsubstitution: */
	Matrix result(numRows,numRows);
	double* extRowPtr=ext+(numRows-1)*numRows*2;
	double* mRowPtr=result.m+(numRows-1)*numRows;
	for(unsigned int i1=numRows;i1>0;--i1,extRowPtr-=numRows*2,mRowPtr-=numRows) // Actual row index i plus one
		{
		double* bsPtr1=extRowPtr+(i1-1);
		double* bsPtr2=extRowPtr+numRows;
		for(unsigned int j=0;j<numRows;++j,++bsPtr2)
			{
			double sum=*bsPtr2;
			double* p1=bsPtr1+1;
			double* p2=bsPtr2+numRows*2;
			for(unsigned int k=i1;k<numRows;++k,++p1,p2+=numRows*2)
				sum-=(*p1)*(*p2);
			mRowPtr[j]=*bsPtr2=sum/(*bsPtr1);
			}
		}
	
	/* Clean up and return the result: */
	delete[] ext;
	return result;
	}

Matrix Matrix::inverseFullPivot(void) const
	{
	/* Create the extended matrix: */
	double* ext=new double[numRows*numRows*2];
	double* extPtr=ext;
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		{
		/* Copy the matrix' elements: */
		for(unsigned int j=0;j<numRows;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		
		/* Create an identity matrix: */
		for(unsigned int j=0;j<numRows;++j,++extPtr)
			*extPtr=i==j?1.0:0.0;
		}
	
	/* Create the column index array: */
	unsigned int* columnIndices=new unsigned int[numRows];
	for(unsigned int i=0;i<numRows;++i)
		columnIndices[i]=i;
	
	/* Perform Gauss elimination with full pivoting on the extended matrix: */
	int swapSign;
	if(gaussFullPivoting(numRows,numRows*2,ext,numRows,columnIndices,swapSign)<numRows)
		throw RankDeficientError();
	
	/* Create the result matrix by backsubstitution: */
	Matrix result(numRows,numRows);
	double* extRowPtr=ext+(numRows-1)*numRows*2;
	for(unsigned int i1=numRows;i1>0;--i1,extRowPtr-=numRows*2) // Actual row index i plus one
		{
		double* mRowPtr=result.m+columnIndices[i1-1]*numRows;
		double* bsPtr1=extRowPtr+(i1-1);
		double* bsPtr2=extRowPtr+numRows;
		for(unsigned int j=0;j<numRows;++j,++bsPtr2)
			{
			double sum=*bsPtr2;
			double* p1=bsPtr1+1;
			double* p2=bsPtr2+numRows*2;
			for(unsigned int k=i1;k<numRows;++k,++p1,p2+=numRows*2)
				sum-=(*p1)*(*p2);
			mRowPtr[j]=*bsPtr2=sum/(*bsPtr1);
			}
		}
	
	/* Clean up and return the result: */
	delete[] ext;
	delete[] columnIndices;
	return result;
	}

Matrix& Matrix::operator*=(const Matrix& other)
	{
	/* Create a new private element array: */
	double* newM=(new double[numRows*other.numColumns+1])+1;
	reinterpret_cast<unsigned int*>(newM)[-1]=1;
	
	/* Multiply the current and other matrices into the new element array: */
	double* rPtr=newM;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<other.numColumns;++j,++rPtr)
			{
			const double* mPtr=m+i*numColumns;
			const double* oPtr=other.m+j;
			*rPtr=0.0;
			for(unsigned int k=0;k<numColumns;++k,++mPtr,oPtr+=other.numColumns)
				*rPtr+=(*mPtr)*(*oPtr);
			}
	
	/* Release the old element array: */
	release();
	
	/* Set the new element array: */
	numColumns=other.numColumns;
	m=newM;
	
	return *this;
	}

Matrix& Matrix::operator/=(const Matrix& other)
	{
	/* Create the extended matrix: */
	double* ext=new double[numRows*(numRows+numColumns)];
	double* extPtr=ext;
	const double* mPtr=m;
	const double* oPtr=other.m;
	for(unsigned int i=0;i<numRows;++i)
		{
		/* Copy the other matrix' elements: */
		for(unsigned int j=0;j<numRows;++j,++extPtr,++oPtr)
			*extPtr=*oPtr;
		
		/* Create this matrix' elements: */
		for(unsigned int j=0;j<numColumns;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		}
	
	/* Perform Gauss elimination with column pivoting on the extended matrix: */
	gaussColumnPivoting(numRows,numRows+numColumns,ext);
	
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* extRowPtr=ext+(numRows-1)*(numRows+numColumns);
	double* mRowPtr=m+(numRows-1)*numColumns;
	for(unsigned int i1=numRows;i1>0;--i1,extRowPtr-=numRows+numColumns,mRowPtr-=numColumns) // Actual row index i plus one
		{
		double* bsPtr1=extRowPtr+(i1-1);
		double* bsPtr2=extRowPtr+numRows;
		for(unsigned int j=0;j<numColumns;++j,++bsPtr2)
			{
			double sum=*bsPtr2;
			double* p1=bsPtr1+1;
			double* p2=bsPtr2+(numRows+numColumns);
			for(unsigned int k=i1;k<numRows;++k,++p1,p2+=numRows+numColumns)
				sum-=(*p1)*(*p2);
			mRowPtr[j]=*bsPtr2=sum/(*bsPtr1);
			}
		}
	
	/* Clean up and return the result: */
	delete[] ext;
	return *this;
	}

Matrix& Matrix::divideFullPivot(const Matrix& other)
	{
	/* Create the extended matrix: */
	double* ext=new double[numRows*(numRows+numColumns)];
	double* extPtr=ext;
	const double* mPtr=m;
	const double* oPtr=other.m;
	for(unsigned int i=0;i<numRows;++i)
		{
		/* Copy the other matrix' elements: */
		for(unsigned int j=0;j<numRows;++j,++extPtr,++oPtr)
			*extPtr=*oPtr;
		
		/* Create this matrix' elements: */
		for(unsigned int j=0;j<numColumns;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		}
	
	/* Create the column index array: */
	unsigned int* columnIndices=new unsigned int[numRows];
	for(unsigned int i=0;i<numRows;++i)
		columnIndices[i]=i;
	
	/* Perform Gauss elimination with full pivoting on the extended matrix: */
	int swapSign;
	if(gaussFullPivoting(numRows,numRows+numColumns,ext,numRows,columnIndices,swapSign)<numRows)
		throw RankDeficientError();
	
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* extRowPtr=ext+(numRows-1)*(numRows+numColumns);
	for(unsigned int i1=numRows;i1>0;--i1,extRowPtr-=numRows+numColumns) // Actual row index i plus one
		{
		double* mRowPtr=m+columnIndices[i1-1]*numColumns;
		double* bsPtr1=extRowPtr+(i1-1);
		double* bsPtr2=extRowPtr+numRows;
		for(unsigned int j=0;j<numColumns;++j,++bsPtr2)
			{
			double sum=*bsPtr2;
			double* p1=bsPtr1+1;
			double* p2=bsPtr2+(numRows+numColumns);
			for(unsigned int k=i1;k<numRows;++k,++p1,p2+=numRows+numColumns)
				sum-=(*p1)*(*p2);
			mRowPtr[j]=*bsPtr2=sum/(*bsPtr1);
			}
		}
	
	/* Clean up and return the result: */
	delete[] ext;
	delete[] columnIndices;
	return *this;
	}

Matrix& Matrix::operator*=(double factor)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr)
			*mPtr*=factor;
	return *this;
	}

Matrix& Matrix::operator/=(double divisor)
	{
	/* Ensure that the element array is private: */
	makePrivate();
	
	double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr)
			*mPtr/=divisor;
	return *this;
	}

double Matrix::mag(void) const
	{
	double result=0.0;
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr)
			result+=*mPtr**mPtr;
	
	return Math::sqrt(result);
	}

Matrix Matrix::transpose(void) const
	{
	Matrix result(numColumns,numRows);
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++mPtr)
			result.m[j*numRows+i]=*mPtr;
	
	return result;
	}

double Matrix::determinant(void) const
	{
	/* Create a temporary matrix: */
	double* ext=new double[numRows*numRows];
	double* extPtr=ext;
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		{
		/* Copy the matrix' elements: */
		for(unsigned int j=0;j<numRows;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		}
	
	/* Create a temporary column index array: */
	unsigned int* columnIndices=new unsigned int[numRows];
	
	/* Perform Gaussian elimination with full pivoting: */
	int swapSign;
	gaussFullPivoting(numRows,numRows,ext,numRows,columnIndices,swapSign);
	
	/* Multiply the diagonal elements: */
	double result=1.0;
	extPtr=ext;
	for(unsigned int i=0;i<numRows;++i,extPtr+=numRows+1)
		result*=*extPtr;
	if(swapSign<0)
		result=-result;
	
	/* Clean up and return the result: */
	delete[] ext;
	delete[] columnIndices;
	return result;
	}

unsigned int Matrix::rank(void) const
	{
	/* Create a temporary matrix: */
	double* ext=new double[numRows*numColumns];
	double* extPtr=ext;
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		{
		/* Copy the matrix' elements: */
		for(unsigned int j=0;j<numColumns;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		}
	
	/* Create a temporary column index array: */
	unsigned int* columnIndices=new unsigned int[numColumns];
	
	/* Perform Gaussian elimination with full pivoting: */
	int swapSign;
	unsigned int result=gaussFullPivoting(numRows,numColumns,ext,numColumns,columnIndices,swapSign);
	
	/* Clean up and return the result: */
	delete[] ext;
	delete[] columnIndices;
	return result;
	}

Matrix Matrix::kernel(void) const
	{
	/* Create a temporary matrix: */
	double* ext=new double[numRows*numColumns];
	double* extPtr=ext;
	const double* mPtr=m;
	for(unsigned int i=0;i<numRows;++i)
		for(unsigned int j=0;j<numColumns;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
	
	/* Create a column permutation array: */
	unsigned int* columnIndices=new unsigned int[numColumns];
	for(unsigned int j=0;j<numColumns;++j)
		columnIndices[j]=j;
	
	/* Perform Gaussian elimination with full pivoting: */
	int swapSign;
	unsigned int rank=gaussFullPivoting(numRows,numColumns,ext,numColumns,columnIndices,swapSign);
	
	/* Create the result matrix: */
	Matrix result(numColumns,numColumns-rank);
	
	double* vector=new double[numColumns];
	for(unsigned int zero=rank;zero<numColumns;++zero)
		{
		/* Calculate the swizzled result using backsubstitution: */
		for(unsigned int i=numColumns-1;i>zero;--i)
			vector[i]=0.0;
		vector[zero]=1.0;
		for(unsigned int i1=zero;i1>rank;--i1) // i1 is actual i+1
			vector[i1-1]=0.0;
		for(unsigned int i1=rank;i1>0;--i1) // i1 is actual i+1
			{
			vector[i1-1]=0.0;
			for(unsigned int j=i1;j<numColumns;++j)
				vector[i1-1]-=ext[(i1-1)*numColumns+j]*vector[j];
			vector[i1-1]/=ext[(i1-1)*numColumns+(i1-1)];
			}
		
		/* Unswizzle the result: */
		for(unsigned int j=0;j<numColumns;++j)
			result.m[columnIndices[j]*(numColumns-rank)+(zero-rank)]=vector[j];
		}
	
	/* Clean up and return the result: */
	delete[] ext;
	delete[] columnIndices;
	delete[] vector;
	return result;
	}

std::pair<Matrix,Matrix> Matrix::solveLinearSystem(const Matrix& coefficients,double zeroFudge) const
	{
	/* Create a temporary matrix: */
	double* ext=new double[numRows*(numColumns+coefficients.numColumns)];
	double* extPtr=ext;
	const double* mPtr=m;
	const double* cPtr=coefficients.m;
	for(unsigned int i=0;i<numRows;++i)
		{
		for(unsigned int j=0;j<numColumns;++j,++extPtr,++mPtr)
			*extPtr=*mPtr;
		for(unsigned int j=0;j<coefficients.numColumns;++j,++extPtr,++cPtr)
			*extPtr=*cPtr;
		}
	
	/* Create a column permutation array: */
	unsigned int* columnIndices=new unsigned int[numColumns];
	for(unsigned int j=0;j<numColumns;++j)
		columnIndices[j]=j;
	
	/* Perform Gaussian elimination with full pivoting: */
	int swapSign;
	unsigned int rank=gaussFullPivoting(numRows,numColumns+coefficients.numColumns,ext,numColumns,columnIndices,swapSign);
	
	/* Create the result matrices: */
	Matrix solution(numColumns,coefficients.numColumns);
	Matrix space(numColumns,numColumns-rank);
	
	double* vector=new double[numColumns];
	
	for(unsigned int columnIndex=0;columnIndex<coefficients.numColumns;++columnIndex)
		{
		/* Calculate the swizzled solution vector using backsubstitution: */
		for(unsigned int i1=numColumns;i1>rank;--i1) // i1 is actual i+1
			{
			vector[i1-1]=0.0;
			
			/* Check if the solution exists: */
			if(i1<=numRows&&Math::abs(ext[(i1-1)*(numColumns+coefficients.numColumns)+numColumns+columnIndex])>zeroFudge)
				throw RankDeficientError();
			}
		for(unsigned int i1=rank;i1>0;--i1) // i1 is actual i+1
			{
			vector[i1-1]=ext[(i1-1)*(numColumns+coefficients.numColumns)+numColumns+columnIndex];
			for(unsigned int j=i1;j<numColumns;++j)
				vector[i1-1]-=ext[(i1-1)*(numColumns+coefficients.numColumns)+j]*vector[j];
			vector[i1-1]/=ext[(i1-1)*(numColumns+coefficients.numColumns)+(i1-1)];
			}
		
		/* Unswizzle the solution vector: */
		for(unsigned int j=0;j<numColumns;++j)
			solution.m[columnIndices[j]*coefficients.numColumns+columnIndex]=vector[j];
		}
	
	for(unsigned int zero=rank;zero<numColumns;++zero)
		{
		/* Calculate the swizzled solution space vector using backsubstitution: */
		for(unsigned int i=numColumns-1;i>zero;--i)
			vector[i]=0.0;
		vector[zero]=1.0;
		for(unsigned int i1=zero;i1>rank;--i1) // i1 is actual i+1
			vector[i1-1]=0.0;
		for(unsigned int i1=rank;i1>0;--i1) // i1 is actual i+1
			{
			vector[i1-1]=0.0;
			for(unsigned int j=i1;j<numColumns;++j)
				vector[i1-1]-=ext[(i1-1)*(numColumns+coefficients.numColumns)+j]*vector[j];
			vector[i1-1]/=ext[(i1-1)*(numColumns+coefficients.numColumns)+(i1-1)];
			}
		
		/* Unswizzle the result: */
		for(unsigned int j=0;j<numColumns;++j)
			space.m[columnIndices[j]*(numColumns-rank)+(zero-rank)]=vector[j];
		}
	
	/* Clean up and return the result: */
	delete[] ext;
	delete[] columnIndices;
	delete[] vector;
	return std::make_pair(solution,space);
	}

std::pair<Matrix,Matrix> Matrix::qrDecomposition(void) const
	{
	/* Create the result matrices: */
	Matrix q(numRows,numRows);
	Matrix r(numRows,numColumns,0.0);
	
	/**********************************************************
	Decompose the matrix using Gram-Schmidt orthonormalization:
	**********************************************************/
	
	/* Calculate the first basis vector: */
	Matrix u0=getColumn(0);
	u0/=u0.mag();
	q.setColumn(0,u0);
	r(0,0)=(u0.transpose()*getColumn(0))(0);
	
	/* Calculate the rest of the basis vectors: */
	for(unsigned int col=1;col<numColumns;++col)
		{
		/* Get the next vector: */
		Matrix acol=getColumn(col);
		Matrix ucol=acol;
		
		/* Orthogonalize it against all previous basis vectors: */
		for(unsigned int j=0;j<col;++j)
			{
			Matrix ej=q.getColumn(j);
			double eu=(ej.transpose()*ucol)(0);
			r(j,col)=eu;
			ucol-=ej*eu;
			}
		
		/* Store the next basis vector: */
		ucol/=ucol.mag();
		q.setColumn(col,ucol);
		r(col,col)=(ucol.transpose()*acol)(0);
		}
	
	/* Return the result matrices: */
	return std::make_pair(q,r);
	}

namespace {

inline unsigned int findRowPivot(unsigned int i,unsigned int numColumns,const double* m)
	{
	unsigned int j=i+1;
	const double* mPtr=m+(i*numColumns+j);
	double pivot=Math::abs(*mPtr);
	unsigned int result=j;
	for(++j,++mPtr;j<numColumns;++j,++mPtr)
		{
		double v=Math::abs(*mPtr);
		if(pivot<v)
			{
			pivot=v;
			result=j;
			}
		}
	
	return result;
	}

}

std::pair<Matrix,Matrix> Matrix::jacobiIteration(void) const
	{
	/* Create the result matrices: */
	Matrix q(numRows,numRows,1.0);
	Matrix e(numRows,1);
	Matrix d=*this;
	d.makePrivate();
	
	/* Initialize the row pivot array: */
	unsigned int* rowPivots=new unsigned int[numRows-1];
	for(unsigned int i=0;i<numRows-1;++i)
		rowPivots[i]=findRowPivot(i,numColumns,d.m);
	
	/* Initialize the eigenvalue matrix: */
	for(unsigned int i=0;i<numRows;++i)
		e.m[i]=d.m[i*numColumns+i];
	
	/* Initialize eigenvalue change array: */
	bool* changed=new bool[numRows];
	for(unsigned int i=0;i<numRows;++i)
		changed[i]=true;
	unsigned int numChanged=numRows;
	
	/* Iterate until all off-diagonal elements are zero or the eigenvalues don't change anymore: */
	while(numChanged>0)
		{
		/* Find pivot: */
		unsigned int k=0;
		unsigned int l=rowPivots[k];
		double pivot=Math::abs(d(k,l));
		for(unsigned int i=1;i<numRows-1;++i)
			{
			unsigned int j=rowPivots[i];
			double v=Math::abs(d(i,j));
			if(pivot<v)
				{
				k=i;
				l=j;
				pivot=v;
				}
			}
		
		/* Check for convergence: */
		if(pivot==0.0)
			break;
		
		/* Calculate the Givens rotation coefficients: */
		double y=(e.m[l]-e.m[k])*0.5;
		double t=Math::abs(y)+Math::sqrt(Math::sqr(pivot)+Math::sqr(y));
		double s=Math::sqrt(Math::sqr(pivot)+Math::sqr(t));
		double c=t/s;
		s=d(k,l)/s;
		t=pivot*pivot/t;
		if(y<0.0)
			{
			s=-s;
			t=-t;
			}
		
		/* Nullify the pivot element: */
		d.m[k*numColumns+l]=0.0;
		
		/* Update the eigenvalues: */
		double ep;
		ep=e.m[k];
		e.m[k]-=t;
		bool newChanged=ep!=e.m[k];
		if(changed[k]!=newChanged)
			{
			changed[k]=newChanged;
			numChanged+=newChanged?1:-1;
			}
		ep=e.m[l];
		e.m[l]+=t;
		newChanged=ep!=e.m[l];
		if(changed[l]!=newChanged)
			{
			changed[l]=newChanged;
			numChanged+=newChanged?1:-1;
			}
		
		/* Rotate the main matrix: */
		double* dRowPtr=d.m;
		for(unsigned int i=0;i<k;++i,dRowPtr+=numColumns)
			{
			double dik=dRowPtr[k];
			double dil=dRowPtr[l];
			dRowPtr[k]=c*dik-s*dil;
			dRowPtr[l]=s*dik+c*dil;
			}
		double* dColPtr=dRowPtr+(numColumns+l);
		for(unsigned int j=k+1;j<l;++j,dColPtr+=numColumns)
			{
			double dkj=dRowPtr[j];
			double djl=*dColPtr;
			dRowPtr[j]=c*dkj-s*djl;
			*dColPtr=s*dkj+c*djl;
			}
		double* dRow2Ptr=d.m+l*numColumns;
		for(unsigned int j=l+1;j<numColumns;++j)
			{
			double dkj=dRowPtr[j];
			double dlj=dRow2Ptr[j];
			dRowPtr[j]=c*dkj-s*dlj;
			dRow2Ptr[j]=s*dkj+c*dlj;
			}
		
		/* Rotate the eigenvector matrix: */
		double* qRowPtr=q.m;
		for(unsigned int i=0;i<numRows;++i,qRowPtr+=numRows)
			{
			double qik=qRowPtr[k];
			double qil=qRowPtr[l];
			qRowPtr[k]=c*qik-s*qil;
			qRowPtr[l]=s*qik+c*qil;
			}
		
		/* Find new row pivots for the changed rows: */
		rowPivots[k]=findRowPivot(k,numColumns,d.m);
		if(l<numRows-1)
			rowPivots[l]=findRowPivot(l,numColumns,d.m);
		}
	
	/* Clean up and return the result matrices: */
	delete[] rowPivots;
	return std::make_pair(q,e);
	}

SVD Matrix::svd(bool calcU,bool calcV) const
	{
	/* Initialize the result: */
	SVD result;
	
	/* Copy this matrix into the U matrix: */
	result.u=*this;
	result.u.makePrivate();
	
	/* Initialize the sigma matrix: */
	result.sigma=Matrix(numColumns,1);
	
	/*********************************************************************
	Reduce the matrix to bidiagonal form using Householder's method:
	*********************************************************************/
	
	double* e=new double[numColumns];
	double g=0.0;
	double x=0.0;
	double tol=Math::Constants<double>::smallest/Math::Constants<double>::epsilon;
	for(unsigned int i=0;i<numColumns;++i)
		{
		e[i]=g;
		double s=0.0;
		for(unsigned int j=i;j<numRows;++j)
			s+=sqr(result.u(j,i));
		if(s<tol)
			g=0.0;
		else
			{
			double f=result.u(i,i);
			g=copysign(sqrt(s),-f);
			double h=f*g-s;
			result.u(i,i)=f-g;
			for(unsigned int j=i+1;j<numColumns;++j)
				{
				s=0.0;
				for(unsigned int k=i;k<numRows;++k)
					s+=result.u(k,j)*result.u(k,i);
				f=s/h;
				for(unsigned int k=i;k<numRows;++k)
					result.u(k,j)+=f*result.u(k,i);
				}
			}
		
		result.sigma(i)=g;
		s=0.0;
		for(unsigned int j=i+1;j<numColumns;++j)
			s+=sqr(result.u(i,j));
		if(s<tol)
			g=0.0;
		else
			{
			double f=result.u(i,i+1);
			g=copysign(sqrt(s),-f);
			double h=f*g-s;
			result.u(i,i+1)=f-g;
			for(unsigned int j=i+1;j<numColumns;++j)
				e[j]=result.u(i,j)/h;
			for(unsigned int j=i+1;j<numRows;++j)
				{
				s=0.0;
				for(unsigned int k=i+1;k<numColumns;++k)
					s+=result.u(j,k)*result.u(i,k);
				for(unsigned int k=i+1;k<numColumns;++k)
					result.u(j,k)+=s*e[k];
				}
			}
		
		x=max(x,abs(result.sigma(i))+abs(e[i]));
		}
	
	if(calcV)
		{
		/*******************************************************************
		Calculate the matrix of right-singular vectors:
		*******************************************************************/
		
		/* Initialize the right-singular matrix: */
		result.v=Matrix(numColumns,numColumns);
		
		/* Calculate the right-singular vectors: */
		for(unsigned int l=numColumns;l>0;--l)
			{
			unsigned int i=l-1;
			if(g!=0.0) // On first iteration, this g is the last g computed in the Householder reduction
				{
				double h=result.u(i,l)*g;
				for(unsigned int j=l;j<numColumns;++j)
					result.v(j,i)=result.u(i,j)/h;
				for(unsigned int j=l;j<numColumns;++j)
					{
					double s=0.0;
					for(unsigned int k=l;k<numColumns;++k)
						s+=result.v(k,j)*result.u(i,k);
					for(unsigned int k=l;k<numColumns;++k)
						result.v(k,j)+=s*result.v(k,i);
					}
				}
			
			for(unsigned int j=l;j<numColumns;++j)
				result.v(i,j)=result.v(j,i)=0.0;
			result.v(i,i)=1.0;
			g=e[i];
			}
		}
	
	if(calcU)
		{
		/*******************************************************************
		Calculate the matrix of left-singular vectors:
		*******************************************************************/
		
		for(unsigned int l=numColumns;l>0;--l)
			{
			unsigned int i=l-1;
			g=result.sigma(i);
			for(unsigned int j=l;j<numColumns;++j)
				result.u(i,j)=0.0;
			if(g!=0.0)
				{
				double h=result.u(i,i)*g;
				for(unsigned int j=l;j<numColumns;++j)
					{
					double s=0.0;
					for(unsigned int k=l;k<numRows;++k)
						s+=result.u(k,j)*result.u(k,i);
					double f=s/h;
					for(unsigned int k=l;k<numRows;++k)
						result.u(k,j)+=f*result.u(k,i);
					}
				
				for(unsigned int j=i;j<numRows;++j)
					result.u(j,i)/=g;
				}
			else
				{
				for(unsigned int j=i;j<numRows;++j)
					result.u(j,i)=0.0;
				}
			
			result.u(i,i)+=1.0;
			}
		}
	
	/*********************************************************************
	Diagonalize the bidiagonal form:
	*********************************************************************/
	
	double eps=Math::Constants<double>::epsilon*x;
	for(unsigned int k1=numColumns;k1>0;--k1)
		{
		unsigned int k=k1-1;
		unsigned int l,l1;
		double c,s;
		double f,g,h,x,y,z;
		
		testForSplitting:
		for(unsigned int l1=k1;l1>0;--l1)
			{
			l=l1-1;
			if(abs(e[l])<=eps)
				goto testForConvergence;
			if(abs(result.sigma(l-1))<=eps) // Never executed when l==0, because e[0] is always 0.0
				goto cancellation;
			}
		
		/* Cancellation of e[l] if l>0: */
		cancellation:
		c=0.0;
		s=1.0;
		l1=l-1;
		for(unsigned int i=l;i<=k;++i)
			{
			double f=s*e[i];
			e[i]=c*e[i];
			if(abs(f)<=eps)
				goto testForConvergence;
			g=result.sigma(i);
			double h=sqrt(f*f+g*g);
			result.sigma(i)=h;
			c=g/h;
			s=-f/h;
			if(calcU)
				for(unsigned int j=0;j<numRows;++j)
					{
					double y=result.u(j,l1);
					double z=result.u(j,i);
					result.u(j,l1)=y*c+z*s;
					result.u(j,i)=-y*s+z*c;
					}
			}
		
		testForConvergence:
		z=result.sigma(k);
		if(l==k)
			goto convergence;
		
		/* Shift from bottom 2x2 minor: */
		x=result.sigma(l);
		y=result.sigma(k-1);
		g=e[k-1];
		h=e[k];
		f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
		g=sqrt(f*f+1.0);
		f=((x-z)*(x+z)+h*(y/(f<0.0?f-g:f+g)-h))/x;
		
		/* Next QR transformation: */
		c=1.0;
		s=1.0;
		for(unsigned int i=l+1;i<=k;++i)
			{
			double g=e[i];
			double y=result.sigma(i);
			double h=s*g;
			g=c*g;
			double z=sqrt(f*f+h*h);
			e[i-1]=z;
			c=f/z;
			s=h/z;
			f=x*c+g*s;
			g=-x*s+g*c;
			h=y*s;
			y*=c;
			if(calcV)
				for(unsigned int j=0;j<numColumns;++j)
					{
					double x=result.v(j,i-1);
					double z=result.v(j,i);
					result.v(j,i-1)=x*c+z*s;
					result.v(j,i)=-x*s+z*c;
					}
			
			z=sqrt(f*f+h*h);
			result.sigma(i-1)=z;
			c=f/z;
			s=h/z;
			f=c*g+s*y;
			x=-s*g+c*y;
			if(calcU)
				for(unsigned int j=0;j<numRows;++j)
					{
					double y=result.u(j,i-1);
					double z=result.u(j,i);
					result.u(j,i-1)=y*c+z*s;
					result.u(j,i)=-y*s+z*c;
					}
			}
		
		e[l]=0.0;
		e[k]=f;
		result.sigma(k)=x;
		goto testForSplitting;
		
		convergence:
		if(z<0.0)
			{
			/* Make result.sigma[k] non-negative: */
			result.sigma(k)=-z;
			if(calcV)
				for(unsigned int j=0;j<numColumns;++j)
					result.v(j,k)=-result.v(j,k);
			}
		}
	
	/* Clean up and return the result: */
	delete[] e;
	return result;
	}

}
