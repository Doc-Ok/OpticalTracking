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

#ifndef EARTHFUNCTIONS_INCLUDED
#define EARTHFUNCTIONS_INCLUDED

/***********************************************************************
Function to calculate radius of Earth at given latitude (geoid formula):
***********************************************************************/

double // Radius in meters
calcRadius
	(
	double latitude // Latitude in radians
	);

/***********************************************************************
Function to calculate the Cartesian coordinates of a point on the
Earth's surface:
***********************************************************************/

template <class ScalarParam>
void
calcSurfacePos
	(
	ScalarParam latitude, // Latitude in radians
	ScalarParam longitude, // Longitude in radians
	double scaleFactor, // Scale factor for resulting coordinates
	ScalarParam pos[3] // Output: Cartesian point coordinates in meters*scaleFactor
	);

/***********************************************************************
Function to calculate the Cartesian coordinates of a point in the
Earth's interior, given a depth:
***********************************************************************/

template <class ScalarParam>
void
calcDepthPos
	(
	ScalarParam latitude, // Latitude in radians
	ScalarParam longitude, // Longitude in radians
	ScalarParam depth, // Depth below surface in meters
	double scaleFactor, // Scale factor for resulting coordinates
	ScalarParam pos[3] // Output: Cartesian point coordinates in meters*scaleFactor
	);

/***********************************************************************
Function to calculate the Cartesian coordinates of a point in the
Earth's interior, given a radius:
***********************************************************************/

template <class ScalarParam>
void
calcRadiusPos
	(
	ScalarParam latitude, // Latitude in radians
	ScalarParam longitude, // Longitude in radians
	ScalarParam radius, // Radius in meters
	double scaleFactor, // Scale factor for resulting coordinates
	ScalarParam pos[3] // Output: Cartesian point coordinates in meters*scaleFactor
	);

/***********************************************************************
Function to draw a model of Earth using texture-mapped quad strips
(texture map must be uploaded and installed beforehand):
***********************************************************************/

void
drawEarth
	(
	int numStrips, // Number of quad strips (latitude sections)
	int numQuads, // Number of quads per strip (longitude sections)
	double scaleFactor // Scale factor for resulting coordinates
	);

/***********************************************************************
Function to draw a model of Earth using texture-mapped quad strips using
indexed primitives and buffer objects
(texture map must be uploaded and installed beforehand):
***********************************************************************/

void
drawEarth
	(
	int numStrips, // Number of quad strips (latitude sections)
	int numQuads, // Number of quads per strip (longitude sections)
	double scaleFactor, // Scale factor for resulting coordinates
	unsigned int vertexBufferObjectId, // ID of buffer object for vertices
	unsigned int indexBufferObjectId // ID of buffer object for indices
	);

/***********************************************************************
Function to draw a latitude/longitude grid on the Earth's surface:
***********************************************************************/

void
drawGrid
	(
	int numStrips, // Number of grid lines in latitude direction
	int numQuads, // Number of grid lines in longitude direction
	int overSample, // Number of intermediate points for grid lines
	double scaleFactor // Scale factor for resulting coordinates
	);

#endif
