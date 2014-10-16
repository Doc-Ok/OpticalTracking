/***********************************************************************
GeoCoordinateSystem - Abstract base class for projected, geographic, or
geocentric coordinate systems used in geodesy.
Copyright (c) 2013 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Geometry/GeoCoordinateSystem.h>

#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <IO/ValueSource.h>
#include <Geometry/Geoid.h>
#include <Geometry/AlbersEqualAreaProjection.h>
#include <Geometry/LambertConformalProjection.h>
#include <Geometry/TransverseMercatorProjection.h>

namespace Geometry {

namespace {

/******************************************
Derived geodetic coordinate system classes:
******************************************/

class GeocentricCoordinateSystem:public GeoCoordinateSystem // Class for geocentric Cartesian coordinate systems
	{
	/* Elements: */
	private:
	Scalar meterScale; // Coordinate system's scaling factor to meters
	Scalar invMeterScale; // Inverse scaling factor
	
	/* Constructors and destructors: */
	public:
	GeocentricCoordinateSystem(Scalar sMeterScale) // Creates a coordinate system with the given scaling factor to meters
		:meterScale(sMeterScale),invMeterScale(Scalar(1)/meterScale)
		{
		}
	
	/* Methods from GeoCoordinateSystem: */
	virtual Point toCartesian(const Point& system) const;
	virtual Point fromCartesian(const Point& system) const;
	
	/* New methods: */
	Scalar getMeterScale(void) const // Returns the system's scaling factor to meters
		{
		return meterScale;
		}
	void setMeterScale(Scalar newMeterScale); // Sets the system's scaling factor to meters
	};

class GeographicCoordinateSystem:public GeoCoordinateSystem // Class for geographic (ellipsoid-based latitude/longitude) coordinate systems
	{
	/* Embedded classes: */
	public:
	typedef Geometry::Geoid<Scalar> Geoid; // Type for reference ellipsoids
	typedef Geometry::Vector<Scalar,3> Vector; // Type for vectors
	
	/* Elements: */
	private:
	Geoid geoid; // The reference ellipsoid defining the latitude/longitude grid
	Vector geoidOffset; // Offset from Earth's center of mass to center of reference ellipsoid
	int axisIndices[3]; // Index map from coordinate system's axes to (longitude, latitude, ellipsoid height)
	Scalar axisScales[3]; // Scaling factors from coordinate system's (longitude, latitude, ellipsoid height) to (radians, radians, meters)
	int invAxisIndices[3]; // Index map from (longitude, latitude, ellipsoid height) to coordinate system's axes
	Scalar invAxisScales[3]; // Inverse scaling factors for (longitude, latitude, ellipsoid height)
	
	/* Constructors and destructors: */
	public:
	GeographicCoordinateSystem(const Geoid& sGeoid); // Creates a default (longitude, latitude, ellipsoid height) coordinate system in (radians, radians, meters) for the given reference ellipsoid
	
	/* Methods from GeoCoordinateSystem: */
	virtual Point toCartesian(const Point& system) const;
	virtual Point fromCartesian(const Point& system) const;
	
	/* New methods: */
	const Geoid& getGeoid(void) const // Returns the coordinate system's reference ellipsoid
		{
		return geoid;
		}
	const Vector& getGeoidOffset(void) const // Returns the reference ellipsoid's center offset from Earth's center of mass
		{
		return geoidOffset;
		}
	const int* getAxisIndices(void) const // Returns the axis order of the coordinate system
		{
		return axisIndices;
		}
	const Scalar* getAxisScales(void) const // Returns the axis scaling factors of the coordinate system
		{
		return axisScales;
		}
	void setGeoidOffset(const Vector& newGeoidOffset); // Sets the reference ellipsoid's center offset
	void setAxisIndices(int longitudeIndex,int latitudeIndex,int ellipsoidHeightIndex); // Sets the axis order of the coordinate system
	void setAxisScales(Scalar longitudeScale,Scalar latitudeScale,Scalar ellipsoidHeightScale); // Sets the axis scaling factors of the coordinate system to (radians, radians, meters)
	};

class ProjectedCoordinateSystem:public GeographicCoordinateSystem // Abstract base class for projected coordinate systems based on geographic coordinate systems
	{
	/* Constructors and destructors: */
	public:
	ProjectedCoordinateSystem(const Geoid& sGeoid)
		:GeographicCoordinateSystem(sGeoid)
		{
		}
	
	/* New methods: */
	public:
	virtual Point toGeographic(const Point& system) const =0; // Transforms a point from this object's to the geographic coordinates defined by the base class object
	virtual Point fromGeographic(const Point& geographic) const =0; // Transforms a point from the geographic coordinates defined by the base class object to this object's coordinate system
	};

template <class ProjectionParam>
class PCS:public ProjectedCoordinateSystem // Concrete class for projected coordinate systems
	{
	/* Embedded classes: */
	private:
	typedef ProjectionParam Projection; // The type of map projection
	
	/* Elements: */
	Projection projection; // The map projection object
	
	/* Constructors and destructors: */
	public:
	PCS(const Projection& sProjection); // Creates a coordinate system from the given map projection
	
	/* Methods from GeoCoordinateSystem: */
	virtual Point toCartesian(const Point& system) const;
	virtual Point fromCartesian(const Point& system) const;
	
	/* Methods from GeographicCoordinateSystem: */
	virtual Point toGeographic(const Point& system) const;
	virtual Point fromGeographic(const Point& geographic) const;
	};

/*******************************************
Methods of class GeocentricCoordinateSystem:
*******************************************/

GeoCoordinateSystem::Point GeocentricCoordinateSystem::toCartesian(const GeoCoordinateSystem::Point& system) const
	{
	/* Scale point to meters: */
	return Point(system[0]*meterScale,system[1]*meterScale,system[2]*meterScale);
	}

GeoCoordinateSystem::Point GeocentricCoordinateSystem::fromCartesian(const GeoCoordinateSystem::Point& cartesian) const
	{
	/* Scale point from meters: */
	return Point(cartesian[0]*invMeterScale,cartesian[1]*invMeterScale,cartesian[2]*invMeterScale);
	}

void GeocentricCoordinateSystem::setMeterScale(GeoCoordinateSystem::Scalar newMeterScale)
	{
	/* Update the scaling factors: */
	meterScale=newMeterScale;
	invMeterScale=Scalar(1)/meterScale;
	}

/*******************************************
Methods of class GeographicCoordinateSystem:
*******************************************/

GeographicCoordinateSystem::GeographicCoordinateSystem(const GeographicCoordinateSystem::Geoid& sGeoid)
	:geoid(sGeoid),geoidOffset(Vector::zero)
	{
	/* Initialize the axis indices and scaling factors: */
	for(int i=0;i<3;++i)
		{
		axisIndices[i]=i;
		axisScales[i]=Scalar(1);
		invAxisIndices[i]=i;
		invAxisScales[i]=Scalar(1);
		}
	}

GeoCoordinateSystem::Point GeographicCoordinateSystem::toCartesian(const GeoCoordinateSystem::Point& system) const
	{
	/* Convert the system point to (longitude, latitude, ellipsoid height) in (radians, radians, meters): */
	Point geoPoint(system[axisIndices[0]]*axisScales[0],system[axisIndices[1]]*axisScales[1],system[axisIndices[2]]*axisScales[2]);
	
	/* Convert the geographic point to Cartesian: */
	return geoid.geodeticToCartesian(geoPoint)+geoidOffset;
	}

GeoCoordinateSystem::Point GeographicCoordinateSystem::fromCartesian(const GeoCoordinateSystem::Point& cartesian) const
	{
	/* Convert the Cartesian point to geographic: */
	Point geoPoint=geoid.cartesianToGeodetic(cartesian-geoidOffset);
	
	/* Convert the geographic point from (longitude, latitude, ellipsoid height) in (radians, radians, meters) to system: */
	return Point(geoPoint[invAxisIndices[0]]*invAxisScales[0],geoPoint[invAxisIndices[1]]*invAxisScales[1],geoPoint[invAxisIndices[2]]*invAxisScales[2]);
	}

void GeographicCoordinateSystem::setAxisIndices(int longitudeIndex,int latitudeIndex,int ellipsoidHeightIndex)
	{
	/* Set the axis indices: */
	axisIndices[0]=longitudeIndex;
	axisIndices[1]=latitudeIndex;
	axisIndices[2]=ellipsoidHeightIndex;
	
	/* Calculate the inverse axis indices and scaling factors: */
	for(int i=0;i<3;++i)
		{
		invAxisIndices[axisIndices[i]]=i;
		invAxisScales[axisIndices[i]]=Scalar(1)/axisScales[i];
		}
	}

void GeographicCoordinateSystem::setAxisScales(GeoCoordinateSystem::Scalar longitudeScale,GeoCoordinateSystem::Scalar latitudeScale,GeoCoordinateSystem::Scalar ellipsoidHeightScale)
	{
	/* Set the axis scales: */
	axisScales[0]=longitudeScale;
	axisScales[1]=latitudeScale;
	axisScales[2]=ellipsoidHeightScale;
	
	/* Calculate the inverse axis indices and scaling factors: */
	for(int i=0;i<3;++i)
		{
		invAxisIndices[axisIndices[i]]=i;
		invAxisScales[axisIndices[i]]=Scalar(1)/axisScales[i];
		}
	}

/*************************************
Methods of class PCS<ProjectionParam>:
*************************************/

template <class ProjectionParam>
inline
PCS<ProjectionParam>::PCS(
	const typename PCS<ProjectionParam>::Projection& sProjection)
	:ProjectedCoordinateSystem(sProjection),
	 projection(sProjection)
	{
	}

template <class ProjectionParam>
inline
GeoCoordinateSystem::Point
PCS<ProjectionParam>::toCartesian(
	const GeoCoordinateSystem::Point& system) const
	{
	/* Pass through to the projection object: */
	return projection.mapToCartesian(system);
	}

template <class ProjectionParam>
inline
GeoCoordinateSystem::Point
PCS<ProjectionParam>::fromCartesian(
	const GeoCoordinateSystem::Point& cartesian) const
	{
	/* Pass through to the projection object: */
	return projection.cartesianToMap(cartesian);
	}

template <class ProjectionParam>
inline
GeoCoordinateSystem::Point
PCS<ProjectionParam>::toGeographic(
	const GeoCoordinateSystem::Point& system) const
	{
	/* Pass through to the projection object: */
	typename Projection::PPoint geoPoint=projection.mapToGeodetic(typename Projection::PPoint(system[0],system[1]));
	return Point(geoPoint[0],geoPoint[1],system[2]);
	}

template <class ProjectionParam>
inline
GeoCoordinateSystem::Point
PCS<ProjectionParam>::fromGeographic(
	const GeoCoordinateSystem::Point& geographic) const
	{
	/* Pass through to the projection object: */
	typename Projection::PPoint systemPoint=projection.geodeticToMap(typename Projection::PPoint(geographic[0],geographic[1]));
	return Point(systemPoint[0],systemPoint[1],geographic[2]);
	}

/**********************************************************************
Helper class to parse projection files in WKT (well-known text) format:
**********************************************************************/

class WktFileParser
	{
	/* Embedded classes: */
	public:
	enum ParseError // Enumerated type for parsing errors in WTK files
		{
		MISSING_OPENING_BRACKET,
		MISSING_CLOSING_BRACKET,
		MISSING_SEPARATOR,
		MISSING_REQUIRED_VALUE,
		UNKNOWN_KEYWORD,
		SEMANTIC_ERROR,
		UNSUPPORTED_FEATURE
		};
	
	/* Elements: */
	private:
	IO::ValueSource prjFile; // A value source to read from the projection file
	
	/* Private methods: */
	void skipOpenBracket(void)
		{
		/* Check for and skip the opening bracket: */
		if(prjFile.eof()||(prjFile.peekc()!='['&&prjFile.peekc()!='('))
			throw MISSING_OPENING_BRACKET;
		prjFile.skipString();
		}
	void skipSeparator(void)
		{
		/* Check for and skip the field separator: */
		if(prjFile.peekc()!=',')
			throw MISSING_SEPARATOR;
		prjFile.skipString();
		}
	void checkForValue(void)
		{
		if(prjFile.eof()||prjFile.peekc()==']'||prjFile.peekc()==')')
			throw MISSING_REQUIRED_VALUE;
		}
	void skipTag(const char* expectedValue)
		{
		if(prjFile.eof()||prjFile.peekc()==']'||prjFile.peekc()==')')
			throw MISSING_REQUIRED_VALUE;
		if(prjFile.readString()!=expectedValue)
			throw MISSING_REQUIRED_VALUE;
		}
	void skipCloseBracket(void)
		{
		/* Check for and skip the closing bracket: */
		if(prjFile.eof()||(prjFile.peekc()!=']'&&prjFile.peekc()!=')'))
			throw MISSING_CLOSING_BRACKET;
		prjFile.skipString();
		}
	void skipKeyword(void);
	void skipOptionalFields(void);
	Geometry::Geoid<double> parseSpheroid(void);
	Geometry::Geoid<double> parseDatum(void);
	double parsePrimeMeridian(void);
	double parseUnit(void);
	int parseAxis(void);
	GeographicCoordinateSystem* parseGeogcs(void);
	PCS<AlbersEqualAreaProjection<double> >* parseAlbersEqualAreaProjection(const GeographicCoordinateSystem* geogcs);
	PCS<LambertConformalProjection<double> >* parseLambertConformalProjection(const GeographicCoordinateSystem* geogcs);
	PCS<TransverseMercatorProjection<double> >* parseTransverseMercatorProjection(const GeographicCoordinateSystem* geogcs);
	ProjectedCoordinateSystem* parseProjcs(void);
	GeoCoordinateSystem* parseCompoundcs(void);
	GeoCoordinateSystem* parseCs(void);
	
	/* Constructors and destructors: */
	public:
	WktFileParser(IO::FilePtr file); // Creates a parser for the given file
	
	/* Methods: */
	GeoCoordinateSystem* parse(void); // Parses the projection file and returns the contained coordinate system
	};

/******************************
Methods of class WktFileParser:
******************************/

void WktFileParser::skipKeyword(void)
	{
	/* Check for opening bracket: */
	if(prjFile.peekc()=='['||prjFile.peekc()=='(')
		{
		/* Skip the opening bracket: */
		prjFile.skipString();
		
		/* Read tokens until the next matching closing bracket or end of file: */
		size_t bracketLevel=1;
		while(!prjFile.eof()&&bracketLevel>0)
			{
			/* Check if the next token is an opening or closing bracket: */
			if(prjFile.peekc()=='['||prjFile.peekc()=='(')
				{
				/* Increase the bracket level: */
				++bracketLevel;
				}
			else if(prjFile.peekc()==']'||prjFile.peekc()==')')
				{
				/* Decrease the bracket level: */
				--bracketLevel;
				}
			
			/* Skip the token: */
			prjFile.skipString();
			}
		
		/* Check if the keyword was properly closed: */
		if(bracketLevel>0)
			throw MISSING_CLOSING_BRACKET;
		}
	}

void WktFileParser::skipOptionalFields(void)
	{
	/* Read tokens until the next matching closing bracket or end of file: */
	size_t bracketLevel=1;
	while(!prjFile.eof()&&bracketLevel>0)
		{
		/* Check if the next token is an opening or closing bracket: */
		if(prjFile.peekc()=='['||prjFile.peekc()=='(')
			{
			/* Increase the bracket level: */
			++bracketLevel;
			}
		else if(prjFile.peekc()==']'||prjFile.peekc()==')')
			{
			/* Decrease the bracket level: */
			--bracketLevel;
			}
		
		/* Skip the token: */
		prjFile.skipString();
		}
	
	/* Check if the keyword was properly closed: */
	if(bracketLevel>0)
		throw MISSING_CLOSING_BRACKET;
	}

Geometry::Geoid<double> WktFileParser::parseSpheroid(void)
	{
	skipOpenBracket();
	
	/* Skip the spheroid name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the semi-major axis: */
	checkForValue();
	double semimajorAxis=prjFile.readNumber();
	
	skipSeparator();
	
	/* Read the inverse flattening factor: */
	checkForValue();
	double inverseFlatteningFactor=prjFile.readNumber();
	
	/* Skip optional fields: */
	skipOptionalFields();
	
	/* Return the reference ellipsoid: */
	return Geometry::Geoid<double>(semimajorAxis,1.0/inverseFlatteningFactor);
	}

Geometry::Geoid<double> WktFileParser::parseDatum(void)
	{
	skipOpenBracket();
	
	/* Skip the datum name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the reference ellipsoid: */
	skipTag("SPHEROID");
	Geometry::Geoid<double> geoid=parseSpheroid();
	
	/* Skip optional fields: */
	skipOptionalFields();
	
	/* Return the reference ellipsoid: */
	return geoid;
	}

double WktFileParser::parsePrimeMeridian(void)
	{
	skipOpenBracket();
	
	/* Skip the prime meridian name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the prime meridian's offset: */
	checkForValue();
	double offset=prjFile.readNumber();
	
	/* Skip optional fields: */
	skipOptionalFields();
	
	return offset;
	}

double WktFileParser::parseUnit(void)
	{
	skipOpenBracket();
	
	/* Skip the linear or angular unit name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the linear or angular unit's conversion factor to meters or radians, respectively: */
	checkForValue();
	double unitFactor=prjFile.readNumber();
	
	/* Skip optional fields: */
	skipOptionalFields();
	
	return unitFactor;
	}

int WktFileParser::parseAxis(void)
	{
	skipOpenBracket();
	
	/* Skip the axis name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the axis keyword: */
	checkForValue();
	std::string axisKeyword=prjFile.readString();
	int axis=-1;
	if(axisKeyword=="NORTH")
		axis=0;
	else if(axisKeyword=="SOUTH")
		axis=1;
	else if(axisKeyword=="EAST")
		axis=2;
	else if(axisKeyword=="WEST")
		axis=3;
	else if(axisKeyword=="UP")
		axis=4;
	else if(axisKeyword=="DOWN")
		axis=5;
	else
		throw UNKNOWN_KEYWORD;
	
	skipCloseBracket();
	
	return axis;
	}

GeographicCoordinateSystem* WktFileParser::parseGeogcs(void)
	{
	skipOpenBracket();
	
	/* Skip the coordinate system name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the geographic datum: */
	skipTag("DATUM");
	Geometry::Geoid<double> geoid=parseDatum();
	
	skipSeparator();
	
	/* Read the prime meridian: */
	skipTag("PRIMEM");
	double primeMeridianOffset=parsePrimeMeridian();
	if(primeMeridianOffset!=0.0)
		throw UNSUPPORTED_FEATURE;
	
	skipSeparator();
	
	/* Read the angular unit: */
	skipTag("UNIT");
	double angularUnitFactor=parseUnit();
	
	/* Check for optional axis specifications: */
	int axisIndices[2]={0,1};
	double axisScales[2];
	axisScales[0]=axisScales[1]=angularUnitFactor;
	if(prjFile.peekc()==',')
		{
		/* Skip the field separator: */
		prjFile.skipString();
		
		/* Read the first axis specification: */
		skipTag("AXIS");
		int axis0=parseAxis();
		
		skipSeparator();
		
		/* Read the second axis specification: */
		skipTag("AXIS");
		int axis1=parseAxis();
		
		/* Set the geodetic coordinate flags: */
		if(axis0<0||axis0>3||axis1<0||axis1>3)
			throw SEMANTIC_ERROR;
		if((axis0<2&&axis1<2)||(axis0>=2&&axis1>=2))
			throw SEMANTIC_ERROR;
		if(axis0<2)
			{
			/* Coordinates are latitude-first: */
			axisIndices[0]=1;
			axisIndices[1]=0;
			}
		if(axis0==1||axis0==3)
			axisScales[0]=-axisScales[0];
		if(axis1==1||axis1==3)
			axisScales[1]=-axisScales[1];
		}
	
	/* Skip optional fields: */
	skipOptionalFields();
	
	/* Return the geographic coordinate system: */
	GeographicCoordinateSystem* result=new GeographicCoordinateSystem(geoid);
	result->setAxisIndices(axisIndices[0],axisIndices[1],2);
	result->setAxisScales(axisScales[0],axisScales[1],1.0);
	return result;
	}

PCS<AlbersEqualAreaProjection<double> >* WktFileParser::parseAlbersEqualAreaProjection(const GeographicCoordinateSystem* geogcs)
	{
	/* Read all parameters for Albers equal-area projection: */
	double lng0=0.0;
	double lat0=0.0;
	double standardLat0=0.0;
	double standardLat1=0.0;
	double falseNorthing=0.0;
	double falseEasting=0.0;
	double unitFactor=1.0;
	while(prjFile.peekc()==',')
		{
		/* Skip the separator: */
		prjFile.skipString();
		
		/* Read the next keyword: */
		checkForValue();
		std::string keyword=prjFile.readString();
		if(keyword=="PARAMETER")
			{
			skipOpenBracket();
			
			/* Read the parameter name: */
			checkForValue();
			std::string parameterName=prjFile.readString();
			
			skipSeparator();
			
			/* Read the parameter value: */
			checkForValue();
			double parameterValue=prjFile.readNumber();
			
			/* Process the parameter: */
			if(parameterName=="Central_Meridian"||parameterName=="Longitude_Of_Center")
				lng0=parameterValue*geogcs->getAxisScales()[0];
			else if(parameterName=="Latitude_Of_Origin"||parameterName=="Latitude_Of_Center")
				lat0=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="Standard_Parallel_1")
				standardLat0=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="Standard_Parallel_2")
				standardLat1=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="False_Easting")
				falseEasting=parameterValue;
			else if(parameterName=="False_Northing")
				falseNorthing=parameterValue;
			else
				throw UNKNOWN_KEYWORD;
			
			skipCloseBracket();
			}
		else if(keyword=="UNIT")
			{
			/* Read the linear unit's conversion factor to meters: */
			unitFactor=parseUnit();
			}
		else if(keyword=="AXIS")
			{
			/* Read the first axis: */
			int axis0=parseAxis();
			
			skipSeparator();
		
			/* Read the second axis specification: */
			skipTag("AXIS");
			int axis1=parseAxis();
			
			/* Check for standard axis layout: */
			if(axis0!=2||axis1!=0)
				throw UNSUPPORTED_FEATURE;
			}
		else
			{
			/* Skip the keyword: */
			skipKeyword();
			}
		}
	
	skipCloseBracket();
	
	/* Create the result coordinate system: */
	AlbersEqualAreaProjection<double> albers(lng0,lat0,standardLat0,standardLat1,geogcs->getGeoid().getRadius(),geogcs->getGeoid().getFlatteningFactor());
	albers.setUnitFactor(unitFactor);
	albers.setFalseNorthing(falseNorthing);
	albers.setFalseEasting(falseEasting);
	
	return new PCS<AlbersEqualAreaProjection<double> >(albers);
	}

PCS<LambertConformalProjection<double> >* WktFileParser::parseLambertConformalProjection(const GeographicCoordinateSystem* geogcs)
	{
	/* Read all parameters for Lambert conformal projection: */
	double lng0=0.0;
	double lat0=0.0;
	double standardLat0=0.0;
	double standardLat1=0.0;
	double falseNorthing=0.0;
	double falseEasting=0.0;
	double unitFactor=1.0;
	while(prjFile.peekc()==',')
		{
		/* Skip the separator: */
		prjFile.skipString();
		
		/* Read the next keyword: */
		checkForValue();
		std::string keyword=prjFile.readString();
		if(keyword=="PARAMETER")
			{
			skipOpenBracket();
			
			/* Read the parameter name: */
			checkForValue();
			std::string parameterName=prjFile.readString();
			
			skipSeparator();
			
			/* Read the parameter value: */
			checkForValue();
			double parameterValue=prjFile.readNumber();
			
			/* Process the parameter: */
			if(parameterName=="Central_Meridian"||parameterName=="Longitude_Of_Center")
				lng0=parameterValue*geogcs->getAxisScales()[0];
			else if(parameterName=="Latitude_Of_Origin"||parameterName=="Latitude_Of_Center")
				lat0=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="Standard_Parallel_1")
				standardLat0=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="Standard_Parallel_2")
				standardLat1=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="False_Easting")
				falseEasting=parameterValue;
			else if(parameterName=="False_Northing")
				falseNorthing=parameterValue;
			else
				throw UNKNOWN_KEYWORD;
			
			skipCloseBracket();
			}
		else if(keyword=="UNIT")
			{
			/* Read the linear unit's conversion factor to meters: */
			unitFactor=parseUnit();
			}
		else if(keyword=="AXIS")
			{
			/* Read the first axis: */
			int axis0=parseAxis();
			
			skipSeparator();
		
			/* Read the second axis specification: */
			skipTag("AXIS");
			int axis1=parseAxis();
			
			/* Check for standard axis layout: */
			if(axis0!=2||axis1!=0)
				throw UNSUPPORTED_FEATURE;
			}
		else
			{
			/* Skip the keyword: */
			skipKeyword();
			}
		}
	
	skipCloseBracket();
	
	/* Create the result coordinate system: */
	LambertConformalProjection<double> lambert(lng0,lat0,standardLat0,standardLat1,geogcs->getGeoid().getRadius(),geogcs->getGeoid().getFlatteningFactor());
	lambert.setUnitFactor(unitFactor);
	lambert.setFalseNorthing(falseNorthing);
	lambert.setFalseEasting(falseEasting);
	
	return new PCS<LambertConformalProjection<double> >(lambert);
	}

PCS<TransverseMercatorProjection<double> >* WktFileParser::parseTransverseMercatorProjection(const GeographicCoordinateSystem* geogcs)
	{
	/* Read all parameters for transverseMercator projection: */
	double lng0=0.0;
	double lat0=0.0;
	double stretching=1.0;
	double falseNorthing=0.0;
	double falseEasting=500000.0;
	double unitFactor=1.0;
	while(prjFile.peekc()==',')
		{
		/* Skip the separator: */
		prjFile.skipString();
		
		/* Read the next keyword: */
		checkForValue();
		std::string keyword=prjFile.readString();
		if(keyword=="PARAMETER")
			{
			skipOpenBracket();
			
			/* Read the parameter name: */
			checkForValue();
			std::string parameterName=prjFile.readString();
			
			skipSeparator();
			
			/* Read the parameter value: */
			checkForValue();
			double parameterValue=prjFile.readNumber();
			
			/* Process the parameter: */
			if(parameterName=="Central_Meridian")
				lng0=parameterValue*geogcs->getAxisScales()[0];
			else if(parameterName=="Latitude_Of_Origin")
				lat0=parameterValue*geogcs->getAxisScales()[1];
			else if(parameterName=="Scale_Factor")
				stretching=parameterValue;
			else if(parameterName=="False_Easting")
				falseEasting=parameterValue;
			else if(parameterName=="False_Northing")
				falseNorthing=parameterValue;
			else
				throw UNKNOWN_KEYWORD;
			
			skipCloseBracket();
			}
		else if(keyword=="UNIT")
			{
			/* Read the linear unit's conversion factor to meters: */
			unitFactor=parseUnit();
			if(unitFactor!=1.0)
				throw UNSUPPORTED_FEATURE;
			}
		else if(keyword=="AXIS")
			{
			/* Read the first axis: */
			int axis0=parseAxis();
			
			skipSeparator();
		
			/* Read the second axis specification: */
			skipTag("AXIS");
			int axis1=parseAxis();
			
			/* Check for standard axis layout: */
			if(axis0!=2||axis1!=0)
				throw UNSUPPORTED_FEATURE;
			}
		else
			{
			/* Skip the keyword: */
			skipKeyword();
			}
		}
	
	skipCloseBracket();
	
	/* Create the result coordinate system: */
	TransverseMercatorProjection<double> transverseMercator(lng0,lat0,geogcs->getGeoid().getRadius(),geogcs->getGeoid().getFlatteningFactor());
	transverseMercator.setStretching(stretching);
	transverseMercator.setFalseNorthing(falseNorthing);
	transverseMercator.setFalseEasting(falseEasting);
	
	return new PCS<TransverseMercatorProjection<double> >(transverseMercator);
	}

ProjectedCoordinateSystem* WktFileParser::parseProjcs(void)
	{
	skipOpenBracket();
	
	/* Skip the coordinate system name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Read the geographic projection: */
	skipTag("GEOGCS");
	Misc::SelfDestructPointer<GeographicCoordinateSystem> geogcs(parseGeogcs());
	
	skipSeparator();
	
	/* Read the projection type: */
	skipTag("PROJECTION");
	
	skipOpenBracket();
	
	/* Read the projection type name: */
	checkForValue();
	std::string projectionName=prjFile.readString();
	
	skipCloseBracket();
	
	/* Create the projected coordinate system: */
	if(projectionName=="Albers"||projectionName=="Albers_Conic_Equal_Area")
		return parseAlbersEqualAreaProjection(geogcs.getTarget());
	else if(projectionName=="Lambert"||projectionName=="Lambert_Conformal_Conic_2SP")
		return parseLambertConformalProjection(geogcs.getTarget());
	else if(projectionName=="Transverse_Mercator")
		return parseTransverseMercatorProjection(geogcs.getTarget());
	else
		throw UNSUPPORTED_FEATURE;
	}

GeoCoordinateSystem* WktFileParser::parseCompoundcs(void)
	{
	skipOpenBracket();
	
	/* Skip the coordinate system name: */
	checkForValue();
	prjFile.skipString();
	
	skipSeparator();
	
	/* Parse the head coordinate system: */
	Misc::SelfDestructPointer<GeoCoordinateSystem> head(parseCs());
	
	skipSeparator();
	
	/* Parse the tail coordinate system: */
	Misc::SelfDestructPointer<GeoCoordinateSystem> tail(parseCs());
	
	/* Assemble the result: */
	if(head.isValid()&&!tail.isValid())
		return head.releaseTarget();
	else if(!head.isValid()&&tail.isValid())
		return tail.releaseTarget();
	else if(!head.isValid()&&!tail.isValid())
		return 0;
	else
		throw UNSUPPORTED_FEATURE;
	}

GeoCoordinateSystem* WktFileParser::parseCs(void)
	{
	/* Read the coordinate system type: */
	checkForValue();
	std::string coordinateSystemType=prjFile.readString();
	
	/* Process the tag: */
	if(coordinateSystemType=="GEOGCS")
		{
		/* Parse a geographic coordinate system: */
		return parseGeogcs();
		}
	else if(coordinateSystemType=="PROJCS")
		{
		/* Parse a projected coordinate system: */
		return parseProjcs();
		}
	else if(coordinateSystemType=="COMPD_CS")
		{
		/* Parse a compound coordinate system: */
		return parseCompoundcs();
		}
	else if(coordinateSystemType=="VERT_CS")
		{
		/* Skip the vertical coordinate system for now and return a null pointer: */
		skipKeyword();
		
		return 0;
		}
	else
		throw UNSUPPORTED_FEATURE;
	}

WktFileParser::WktFileParser(IO::FilePtr file)
	:prjFile(file)
	{
	/* Prepare the projection file: */
	prjFile.setPunctuation("[](),");
	prjFile.setQuotes("\"");
	prjFile.skipWs();
	}

GeoCoordinateSystem* WktFileParser::parse(void)
	{
	/* Parse the root coordinate system: */
	GeoCoordinateSystem* result=parseCs();
	
	/* Check if the file is over: */
	if(!prjFile.eof())
		{
		delete result;
		throw SEMANTIC_ERROR;
		}
	
	return result;
	}

/*********************************************
Derived coordinate system reprojector classes:
*********************************************/

class IdentityReprojector:public GeoReprojector // Class for identity conversions between compatible coordinate systems
	{
	/* Methods from GeoReprojector: */
	public:
	virtual Point convert(const Point& source) const;
	virtual Box convert(const Box& source) const;
	};

class GeocentricToGeocentricReprojector:public GeoReprojector // Class to convert between geocentric coordinate systems
	{
	/* Elements: */
	private:
	Scalar unitFactor; // Scaling factor from source units to destination units
	
	/* Constructors and destructors: */
	public:
	GeocentricToGeocentricReprojector(const GeocentricCoordinateSystem& source,const GeocentricCoordinateSystem& dest);
	
	/* Methods from GeoReprojector: */
	virtual Point convert(const Point& source) const;
	virtual Box convert(const Box& source) const;
	};

class GeocentricToGeographicReprojector:public GeoReprojector // Class to convert from geocentric to geographic coordinate systems
	{
	/* Elements: */
	private:
	Scalar meterScale; // Scaling factor from source units to meters
	GeographicCoordinateSystem::Vector geoidOffset;
	GeographicCoordinateSystem::Geoid geoid;
	int invAxisIndices[3];
	Scalar invAxisScales[3];
	
	/* Constructors and destructors: */
	public:
	GeocentricToGeographicReprojector(const GeocentricCoordinateSystem& source,const GeographicCoordinateSystem& dest);
	
	/* Methods from GeoReprojector: */
	virtual Point convert(const Point& source) const;
	virtual Box convert(const Box& source) const;
	};

class GeographicToGeocentricReprojector:public GeoReprojector // Class to convert from geographic to geocentric coordinate systems
	{
	/* Elements: */
	private:
	GeographicCoordinateSystem::Geoid geoid;
	GeographicCoordinateSystem::Vector geoidOffset;
	int axisIndices[3];
	Scalar axisScales[3];
	Scalar invMeterScale; // Scaling factor from meters to destination units
	
	/* Constructors and destructors: */
	public:
	GeographicToGeocentricReprojector(const GeographicCoordinateSystem& source,const GeocentricCoordinateSystem& dest);
	
	/* Methods from GeoReprojector: */
	virtual Point convert(const Point& source) const;
	virtual Box convert(const Box& source) const;
	};

/************************************
Methods of class IdentityReprojector:
************************************/

GeoReprojector::Point IdentityReprojector::convert(const GeoReprojector::Point& source) const
	{
	return source;
	}

GeoReprojector::Box IdentityReprojector::convert(const GeoReprojector::Box& source) const
	{
	return source;
	}

/**************************************************
Methods of class GeocentricToGeocentricReprojector:
**************************************************/

GeocentricToGeocentricReprojector::GeocentricToGeocentricReprojector(const GeocentricCoordinateSystem& source,const GeocentricCoordinateSystem& dest)
	:unitFactor(source.getMeterScale()/dest.getMeterScale())
	{
	}

GeoReprojector::Point GeocentricToGeocentricReprojector::convert(const GeoReprojector::Point& source) const
	{
	return Point(source[0]*unitFactor,source[1]*unitFactor,source[2]*unitFactor);
	}

GeoReprojector::Box GeocentricToGeocentricReprojector::convert(const GeoReprojector::Box& source) const
	{
	Box result;
	for(int i=0;i<3;++i)
		{
		result.min[i]=source.min[i]*unitFactor;
		result.max[i]=source.max[i]*unitFactor;
		}
	return result;
	}

/**************************************************
Methods of class GeocentricToGeographicReprojector:
**************************************************/

GeocentricToGeographicReprojector::GeocentricToGeographicReprojector(const GeocentricCoordinateSystem& source,const GeographicCoordinateSystem& dest)
	:meterScale(source.getMeterScale()),
	 geoidOffset(dest.getGeoidOffset()),
	 geoid(dest.getGeoid())
	{
	/* Copy the inverse axis indices and inverse axis scales: */
	for(int i=0;i<3;++i)
		{
		invAxisIndices[dest.getAxisIndices()[i]]=i;
		invAxisScales[dest.getAxisIndices()[i]]=Scalar(1)/dest.getAxisScales()[i];
		}
	}

GeoReprojector::Point GeocentricToGeographicReprojector::convert(const GeoReprojector::Point& source) const
	{
	/* Convert the source point to Cartesian in meters: */
	Point cartesian(source[0]*meterScale,source[1]*meterScale,source[2]*meterScale);
	
	/* Convert the Cartesian point to geographic: */
	Point geoPoint=geoid.cartesianToGeodetic(cartesian-geoidOffset);
	
	/* Convert the geographic point from (longitude, latitude, ellipsoid height) in (radians, radians, meters) to system: */
	return Point(geoPoint[invAxisIndices[0]]*invAxisScales[0],geoPoint[invAxisIndices[1]]*invAxisScales[1],geoPoint[invAxisIndices[2]]*invAxisScales[2]);
	}

GeoReprojector::Box GeocentricToGeographicReprojector::convert(const GeoReprojector::Box& source) const
	{
	Box result=Box::empty;
	
	/* Add the transformed eight corners of the source box to the destination box: */
	for(int i=0;i<8;++i)
		{
		/* Convert the source box corner to Cartesian in meters: */
		Point sourceCorner=source.getVertex(i);
		Point cartesian(sourceCorner[0]*meterScale,sourceCorner[1]*meterScale,sourceCorner[2]*meterScale);
		
		/* Convert the Cartesian point to geographic: */
		Point geoPoint=geoid.cartesianToGeodetic(cartesian-geoidOffset);
		
		/* Convert the geographic point from (longitude, latitude, ellipsoid height) in (radians, radians, meters) to system: */
		result.addPoint(Point(geoPoint[invAxisIndices[0]]*invAxisScales[0],geoPoint[invAxisIndices[1]]*invAxisScales[1],geoPoint[invAxisIndices[2]]*invAxisScales[2]));
		}
	
	/* There are all kinds of special cases to consider, but oh well... */
	
	return result;
	}

/**************************************************
Methods of class GeographicToGeocentricReprojector:
**************************************************/

GeographicToGeocentricReprojector::GeographicToGeocentricReprojector(const GeographicCoordinateSystem& source,const GeocentricCoordinateSystem& dest)
	:geoid(source.getGeoid()),
	 geoidOffset(source.getGeoidOffset()),
	 invMeterScale(Scalar(1)/dest.getMeterScale())
	{
	/* Copy the axis indices and axis scales: */
	for(int i=0;i<3;++i)
		{
		axisIndices[i]=source.getAxisIndices()[i];
		axisScales[i]=source.getAxisScales()[i];
		}
	}

GeoReprojector::Point GeographicToGeocentricReprojector::convert(const GeoReprojector::Point& source) const
	{
	/* Convert the source point to (longitude, latitude, ellipsoid height) in (radians, radians, meters): */
	Point geoPoint(source[axisIndices[0]]*axisScales[0],source[axisIndices[1]]*axisScales[1],source[axisIndices[2]]*axisScales[2]);
	
	/* Convert the geographic point to Cartesian: */
	Point cartesian=geoid.geodeticToCartesian(geoPoint)+geoidOffset;
	
	/* Convert the Cartesian point to destination units: */
	return Point(cartesian[0]*invMeterScale,cartesian[1]*invMeterScale,cartesian[2]*invMeterScale);
	}

GeoReprojector::Box GeographicToGeocentricReprojector::convert(const GeoReprojector::Box& source) const
	{
	Box result=Box::empty;
	
	/* Add the transformed eight corners of the source box to the destination box: */
	for(int i=0;i<8;++i)
		{
		/* Convert the source box corner to (longitude, latitude, ellipsoid height) in (radians, radians, meters): */
		Point sourceCorner=source.getVertex(i);
		Point geoPoint(sourceCorner[axisIndices[0]]*axisScales[0],sourceCorner[axisIndices[1]]*axisScales[1],sourceCorner[axisIndices[2]]*axisScales[2]);
		
		/* Convert the geographic point to Cartesian: */
		Point cartesian=geoid.geodeticToCartesian(geoPoint)+geoidOffset;
		
		/* Convert the Cartesian point to destination units: */
		result.addPoint(Point(cartesian[0]*invMeterScale,cartesian[1]*invMeterScale,cartesian[2]*invMeterScale));
		}
	
	/* There are all kinds of special cases to consider, but oh well... */
	
	return result;
	}

}

GeoCoordinateSystemPtr parseProjectionFile(IO::DirectoryPtr directory,const char* projectionFileName)
	{
	/* Create a projection file parser: */
	WktFileParser parser(directory->openFile(projectionFileName));
	
	GeoCoordinateSystem* result=0;
	try
		{
		/* Parse the file: */
		result=parser.parse();
		}
	catch(WktFileParser::ParseError err)
		{
		/* Construct the projection file's fully-qualified name: */
		std::string name=directory->getPath();
		name.push_back('/');
		name.append(projectionFileName);
		
		/* Throw a more verbose exception: */
		switch(err)
			{
			case WktFileParser::MISSING_OPENING_BRACKET:
				Misc::throwStdErr("Geometry::parseProjectionFile: Missing opening bracket in projection file %s",name.c_str());
				break;
			
			case WktFileParser::MISSING_CLOSING_BRACKET:
				Misc::throwStdErr("Geometry::parseProjectionFile: Missing closing bracket in projection file %s",name.c_str());
				break;
			
			case WktFileParser::MISSING_SEPARATOR:
				Misc::throwStdErr("Geometry::parseProjectionFile: Missing comma separator in projection file %s",name.c_str());
				break;
			
			case WktFileParser::MISSING_REQUIRED_VALUE:
				Misc::throwStdErr("Geometry::parseProjectionFile: Missing required value in projection file %s",name.c_str());
				break;
			
			case WktFileParser::UNKNOWN_KEYWORD:
				Misc::throwStdErr("Geometry::parseProjectionFile: Unknown keyword in projection file %s",name.c_str());
				break;
			
			case WktFileParser::SEMANTIC_ERROR:
				Misc::throwStdErr("Geometry::parseProjectionFile: Semantic error in projection file %s",name.c_str());
				break;
			
			case WktFileParser::UNSUPPORTED_FEATURE:
				Misc::throwStdErr("Geometry::parseProjectionFile: Unsupported WKT format feature in projection file %s",name.c_str());
				break;
			}
		}
	
	return result;
	}

GeoReprojectorPtr createReprojector(GeoCoordinateSystemPtr source,GeoCoordinateSystemPtr dest)
	{
	return 0;
	}

}
