/***********************************************************************
SphereRenderer - Class to efficiently render transparent spheres used
as interaction brushes.
Copyright (c) 2006 Oliver Kreylos
***********************************************************************/

#ifndef SPHERERENDERER_INCLUDED
#define SPHERERENDERER_INCLUDED

#include <GL/gl.h>
#include <GL/GLObject.h>

class SphereRenderer:public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint sphereDisplayListId; // ID of the display list containing the sphere
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	GLfloat radius; // Sphere radius
	int numStrips; // Subdivision level of the sphere model
	
	/* Constructors and destructors: */
	public:
	SphereRenderer(GLfloat sRadius,int sNumStrips);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	void glRenderAction(GLContextData& contextData) const;
	};

#endif
