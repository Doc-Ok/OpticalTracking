/***********************************************************************
LensDistortionParameters - Class to represent functions to correct
common lens distortions; specifically, the Brown-Conrady radial/
tangential distortion formula with quartic radial and tangential terms.
Copyright (c) 2009-2022 Oliver Kreylos

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

#include "LensDistortionParameters.h"

#include <IO/File.h>
#include <IO/OpenFile.h>
#include <Math/Math.h>
#include <Geometry/Matrix.h>

/*****************************************
Methods of class LensDistortionParameters:
*****************************************/

LensDistortionParameters::LensDistortionParameters(const int imageSize[2])
	{
	for(int i=0;i<2;++i)
		center[i]=double(imageSize[i])*0.5;
	radiusScale2=1.0/Math::sqr((center[0]+center[1])*0.5);
	for(int i=0;i<2;++i)
		kappa[i]=0.0;
	for(int i=0;i<3;++i)
		rho[i]=0.0;
	}

void LensDistortionParameters::read(const char* fileName)
	{
	IO::FilePtr file=IO::openFile(fileName);
	read(*file);
	}

void LensDistortionParameters::read(IO::File& file)
	{
	file.setEndianness(Misc::LittleEndian);
	file.read(center.getComponents(),2);
	file.read<double>(radiusScale2);
	file.read(kappa,2);
	file.read(rho,3);
	}

void LensDistortionParameters::write(const char* fileName) const
	{
	IO::FilePtr file=IO::openFile(fileName,IO::File::WriteOnly);
	write(*file);
	}

void LensDistortionParameters::write(IO::File& file) const
	{
	file.setEndianness(Misc::LittleEndian);
	file.write(center.getComponents(),2);
	file.write<double>(radiusScale2);
	file.write(kappa,2);
	file.write(rho,3);
	}

const char* LensDistortionParameters::getVarName(int index)
	{
	switch(index)
		{
		case 0:
			return "Center X";
		
		case 1:
			return "Center Y";
		
		case 2:
			return "Kappa 1";
		
		case 3:
			return "Kappa 2";
		
		case 4:
			return "Rho 1";
		
		case 5:
			return "Rho 2";
		
		case 6:
			return "Rho 3";
		}
	
	/* Never reached; just to make compiler happy: */
	return 0;
	}

LensDistortionParameters::Derivative LensDistortionParameters::transformp(const LensDistortionParameters::Point& source) const
	{
	/* Not implemented because not needed */
	return Derivative(0);
	}
