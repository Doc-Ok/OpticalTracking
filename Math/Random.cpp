/***********************************************************************
Random - Functions to create random numbers of several probability
distributions.
Copyright (c) 2003-2005 Oliver Kreylos

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

#include <stdlib.h>
#include <math.h>

#include <Math/Random.h>

namespace Math {

double randUniformCO(void)
	{
	return double(rand())/(double(RAND_MAX)+1.0);
	}

double randUniformCC(void)
	{
	return double(rand())/double(RAND_MAX);
	}

int randUniformCO(int min,int max)
	{
	return int(floor(double(rand())*double(max-min)/(double(RAND_MAX)+1.0)))+min;
	}

double randUniformCO(double min,double max)
	{
	return double(rand())*(max-min)/(double(RAND_MAX)+1.0)+min;
	}

int randUniformCC(int min,int max)
	{
	return int(floor(double(rand())*double(max+1-min)/(double(RAND_MAX)+1.0)))+min;
	}

double randUniformCC(double min,double max)
	{
	return double(rand())*(max-min)/double(RAND_MAX)+min;
	}

double randNormal(double mean,double stddev)
	{
	/* Table containing a normal distribution's probability integral: */
	static const double normalDistributionTable[63]={
		0.0000,0.0013,0.0019,0.0026,0.0035,0.0047,0.0062,0.0082,0.0107,0.0139,
		0.0179,0.0228,0.0287,0.0359,0.0446,0.0548,0.0668,0.0808,0.0968,0.1151,
		0.1357,0.1587,0.1841,0.2119,0.2420,0.2743,0.3085,0.3446,0.3821,0.4207,
		0.4602,0.5000,0.5398,0.5793,0.6179,0.6554,0.6915,0.7257,0.7580,0.7881,
		0.8159,0.8413,0.8643,0.8849,0.9032,0.9192,0.9332,0.9452,0.9554,0.9641,
		0.9713,0.9772,0.9821,0.9861,0.9893,0.9918,0.9938,0.9953,0.9965,0.9974,
		0.9981,0.9987,1.0000};
	
	/* Create a uniformly distributed random variable in [0,1]: */
	double x=double(rand())/double(RAND_MAX);
	
	/* Find interval in normal distribution table containing the value: */
	int l=0;
	int r=62;
	while(r-l>1)
		{
		int m=(l+r)>>1;
		if(x<normalDistributionTable[m])
			r=m;
		else
			l=m;
		}
	
	/* Interpolate in normal distribution table: */
	double intWidth=normalDistributionTable[r]-normalDistributionTable[l];
	return ((double(l-31)+(x-normalDistributionTable[l])/intWidth)/10.0)*stddev+mean;
	}

}
