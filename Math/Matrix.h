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

#ifndef MATH_MATRIX_INCLUDED
#define MATH_MATRIX_INCLUDED

#include <utility>
#include <stdexcept>

namespace Math {

/* Forward declarations: */
struct SVD;

class Matrix
	{
	/* Embedded classes: */
	public:
	struct Error:public std::runtime_error // Generic exception class to signal errors when handling matrices
		{
		/* Constructors and destructors: */
		public:
		Error(const char* errorMsg)
			:std::runtime_error(errorMsg)
			{
			}
		};
	
	struct RankDeficientError:public Error // Exception class to signal rank-deficient matrices when solving linear systems
		{
		/* Constructors and destructors: */
		public:
		RankDeficientError(void)
			:Error("Attempt to solve linear system with rank-deficient matrix")
			{
			}
		};
	
	/* Elements: */
	private:
	unsigned int numRows,numColumns; // Size of the matrix
	double* m; // Pointer to 2D array of matrix elements, including a dangly bit at the beginning to count shared owners
	
	/* Private methods: */
	void share(double* newM); // Takes shared ownership of the given element array
	void release(void); // Releases ownership of the matrix's element array
	
	/* Constructors and destructors: */
	public:
	Matrix(void) // Creates an invalid matrix
		:numRows(0),numColumns(0),m(0)
		{
		}
	Matrix(unsigned int sNumRows,unsigned int sNumColumns,double* sElements =0); // Creates a matrix of the given size and copies the given row-major array of elements
	Matrix(unsigned int sNumRows,unsigned int sNumColumns,double sDiag); // Creates a diagonal matrix with the given diagonal element
	Matrix(const Matrix& source) // Copy constructor
		:numRows(source.numRows),numColumns(source.numColumns)
		{
		/* Share the source matrix' element array: */
		share(source.m);
		}
	Matrix& operator=(const Matrix& source) // Assignment operator
		{
		if(this!=&source)
			{
			/* Release the current element array: */
			release();
			
			/* Resize the matrix: */
			numRows=source.numRows;
			numColumns=source.numColumns;
			
			/* Share the source matrix' element array: */
			share(source.m);
			}
		return *this;
		}
	~Matrix(void) // Destroys the matrix
		{
		/* Release the element array: */
		release();
		}
	
	/* Access methods: */
	unsigned int getNumRows(void) const // Returns the matrix' number of rows
		{
		return numRows;
		}
	unsigned int getNumColumns(void) const // Returns the matrix' number of columns
		{
		return numColumns;
		}
	double operator()(unsigned int rowIndex,unsigned int columnIndex) const // Returns one matrix element
		{
		return m[rowIndex*numColumns+columnIndex];
		}
	double operator()(unsigned int index) const // Returns one matrix element from a row or column vector
		{
		return m[index];
		}
	Matrix getRow(unsigned int rowIndex) const; // Returns a matrix row as a 1 x numColumns matrix
	Matrix getColumn(unsigned int columnIndex) const; // Returns a matrix column as a numRows x 1 matrix
	
	/* Comparison operators: */
	bool operator==(const Matrix& other) const; // Returns true if the other matrix is exactly the same
	bool operator!=(const Matrix& other) const; // Returns true if the other matrix is not exactly the same
	
	/* Matrix manipulation methods: */
	void makePrivate(void); // Ensures that the matrix object is sole owner of the element array
	double& operator()(unsigned int rowIndex,unsigned int columnIndex) // Returns one matrix element as lvalue; assumes that matrix is private
		{
		return m[rowIndex*numColumns+columnIndex];
		}
	double& operator()(unsigned int index) // Returns one matrix element from a row or column vector as lvalue; assumes that matrix is private
		{
		return m[index];
		}
	void set(unsigned int rowIndex,unsigned int columnIndex,double newValue) // Sets the given matrix element to the given value
		{
		/* Ensure that the element array is private: */
		makePrivate();
		
		/* Set the matrix element: */
		m[rowIndex*numColumns+columnIndex]=newValue;
		}
	void set(unsigned int index,double newValue) // Sets the given matrix element of a row or column vector to the given value
		{
		/* Ensure that the element array is private: */
		makePrivate();
		
		/* Set the matrix element: */
		m[index]=newValue;
		}
	void setRow(unsigned int rowIndex,const Matrix& row); // Copies the given 1 x numColumns matrix into the given row
	void setColumn(unsigned int columnIndex,const Matrix& column); // Copies the given numRows x 1 matrix into the given column
	void swapRows(unsigned int rowIndex1,unsigned int rowIndex2); // Swaps two of the matrix' rows
	void swapColumns(unsigned int columnIndex1,unsigned int columnIndex2); // Swaps two of the matrix' columns
	void scaleRow(unsigned int rowIndex,double factor); // Scales the given row by the given factor
	void scaleColumn(unsigned int columnIndex,double factor); // Scales the given column by the given factor
	void combineRows(unsigned int rowIndex1,double factor,unsigned int rowIndex2); // Adds a scaled multiple of the first row to the second row
	
	/* Matrix ring operations: */
	Matrix operator-(void) const; // Ring additive inverse
	Matrix& operator+=(const Matrix& other); // Ring addition
	Matrix& operator-=(const Matrix& other); // Ring subtraction
	Matrix inverse(void) const; // Ring multiplicative inverse; throws exception if matrix is singular
	Matrix inverseFullPivot(void) const; // Ring multiplicative inverse calculated using full pivoting; throws exception if matrix is singular
	Matrix& operator*=(const Matrix& other); // Ring multiplication
	Matrix& operator/=(const Matrix& other); // Ring division; throws exception if other matrix is singular
	Matrix& divideFullPivot(const Matrix& other); // Ring division calculated using full pivoting; throws exception if other matrix is singular
	
	/* Matrix vector space operations: */
	Matrix& operator*=(double factor); // Scalar multiplication
	Matrix& operator/=(double divisor); // Scalar division
	
	/* Other matrix methods: */
	double mag(void) const; // Returns the matrix' Frobenius norm
	Matrix transpose(void) const; // Returns a diagonally mirrored matrix
	double determinant(void) const; // Returns the matrix' determinant
	unsigned int rank(void) const; // Returns the matrix' row rank
	Matrix kernel(void) const; // Returns a matrix whose column vectors span this matrix' null space
	std::pair<Matrix,Matrix> solveLinearSystem(const Matrix& coefficients,double zeroFudge =0.0) const; // Returns a pair of matrices defining all solutions to the linear system defined by the matrix and the coefficient column vector. The first matrix contains solution column vectors; the column vectors of the second matrix span the solution space if the system is under-determined; uses zeroFudge to check for null rows in underdetermined case
	std::pair<Matrix,Matrix> qrDecomposition(void) const; // Returns (q, r), the QR decomposition of the matrix
	std::pair<Matrix,Matrix> jacobiIteration(void) const; // Performs Jacobi iteration on a symmetric matrix; returns orthogonal matrix Q of eigenvectors and column vector E of eigenvalues
	SVD svd(bool calcU,bool calcV) const; // Performs singular value decomposition on a tall matrix (numRows >= numColumns). Calculates left-singular and right-singular vectors only if respective flags are true
	};

struct SVD // Structure to return results of singular value decomposition of an m x n matrix (where m >= n)
	{
	/* Elements: */
	public:
	Matrix u; // m x n matrix of left-singular vectors
	Matrix sigma; // n x 1 matrix of singular values
	Matrix v; // n x n matrix of right-singular vectors
	};

/*************************
External matrix operators:
*************************/

inline Matrix operator+(const Matrix& m1,const Matrix& m2) // Ring addition
	{
	Matrix result=m1;
	result+=m2;
	return result;
	}

inline Matrix operator-(const Matrix& m1,const Matrix& m2) // Ring subtraction
	{
	Matrix result=m1;
	result-=m2;
	return result;
	}

inline Matrix operator*(const Matrix& m1,const Matrix& m2) // Ring multiplication
	{
	Matrix result=m1;
	result*=m2;
	return result;
	}

inline Matrix operator/(const Matrix& m1,const Matrix& m2) // Ring division; throws exception if m2 is singular
	{
	Matrix result=m1;
	result/=m2;
	return result;
	}

inline Matrix operator*(const Matrix& m,double factor) // Scalar multiplication
	{
	Matrix result=m;
	result*=factor;
	return result;
	}

inline Matrix operator*(double factor,const Matrix& m) // Scalar multiplication from the left
	{
	Matrix result=m;
	result*=factor;
	return result;
	}

inline Matrix operator/(const Matrix& m,double divisor) // Scalar division
	{
	Matrix result=m;
	result/=divisor;
	return result;
	}

inline Matrix operator/(double dividend,const Matrix& m) // Scalar multiplication with inverse matrix
	{
	Matrix result=m.inverse();
	result*=dividend;
	return result;
	}

}

#endif
