/***********************************************************************
Influence - Class to encapsulate influence shapes and modification
actions.
***********************************************************************/

#ifndef INFLUENCE_INCLUDED
#define INFLUENCE_INCLUDED

#include <Math/Math.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthonormalTransformation.h>

/* Forward declarations: */
class GLContextData;
class AutoTriangleMesh;
class SphereRenderer;

class Influence
	{
	/* Embedded classes: */
	public:
	typedef AutoTriangleMesh Mesh; // Data type for meshes
	typedef Geometry::Point<double,3> Point;
	typedef Geometry::Vector<double,3> Vector;
	typedef Geometry::OrthonormalTransformation<double,3> ONTransform;
	
	enum ActionType
		{
		EXPLODE,DRAG,WHITTLE
		};
	
	struct VertexMotion // Class to store motion of vertices during fairing operation
		{
		/* Elements: */
		public:
		Mesh::VertexIterator vIt;
		float vec[3];

		/* Constructors and destructors: */
		VertexMotion(Mesh::VertexIterator sVIt)
			:vIt(sVIt)
			{
			};
		};
	
	/* Elements: */
	private:
	const SphereRenderer* sphereRenderer;
	
	/* Influence state: */
	ONTransform transformation; // Influence's current position and orientation
	Vector linearVelocity,angularVelocity; // Current linear and angular velocities of influence's motion
	
	/* Shape data: */
	double radius,radius2; // Influence's sphere radius and squared radius
	
	/* Action data: */
	ActionType action; // Current action performed by influence
	double pressure; // "Strength" of influence's action
	double density; // used to set the 'smallness' of triangle size (see NotSmallEnough fn) - i.e density of  generated mesh
	
	/* Private methods: */
	double pressureFunction(double r) const
		{
		/* Compute s-shaped curve dropping from pressure to 0.0 as r goes from 0.0 to 1.0: */
		if (r>=1.0)
			return 0.0;
		return (1.0-Math::sqr(r)*(3.0-2.0*r))*pressure;
		};
	
	/* Constructors and destructors: */
	public:
	Influence(const SphereRenderer* sSphereRenderer,double sRadius);  // Creates influence sphere of given radius
	
	/* Methods: */
	
	/* State management: */
	Point getPosition(void) const
		{
		return transformation.getOrigin();
		};
	const ONTransform::Rotation& getOrientation(void) const
		{
		return transformation.getRotation();
		};
	void setPositionOrientation(const ONTransform& newTransformation); // Sets position and orientation
	const Vector& getLinearVelocity(void) const
		{
		return linearVelocity;
		};
	const Vector& getAngularVelocity(void) const
		{
		return angularVelocity;
		};
	void setLinearVelocity(const Vector& newLinearVelocity) // Sets linear velocity
		{
		linearVelocity=newLinearVelocity;
		};
	void setAngularVelocity(const Vector& newAngularVelocity) // Sets angular velocity
		{
		angularVelocity=newAngularVelocity;
		};
	double getRadius(void) const // Returns influence's current radius
		{
		return radius;
		};
	void setRadius(double newRadius) // Sets influence's sphere radius
		{
		radius=newRadius;
		radius2=radius*radius;
		};
	ActionType getAction(void) const // Returns current action mode
		{
		return action;
		};
	void setAction(ActionType newAction) // Sets influence's action mode
		{
		action=newAction;
		};
	double getPressure(void) const // Returns current pressure
		{
		return pressure;
		};
	void setPressure(double newPressure) // Sets influence's pressure
		{
		pressure=newPressure;
		};
	double getDensity(void) const // Returns current density
		{
		return density;
		};
	void setDensity(double newDensity) // Sets influence's density
		{
		density=newDensity;
		};
	void glRenderAction(GLContextData& contextData) const; // Renders the influence object
	
	/* Interface with triangle mesh class: */
	void actOnMesh(Mesh& mesh) const; // Lets influence object act on mesh object
	};

#endif
