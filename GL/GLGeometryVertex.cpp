/***********************************************************************
GLGeometryVertex - Class to encapsulate OpenGL vertex properties using
geometry data types.
Copyright (c) 2009-2010 Oliver Kreylos

This file is part of the OpenGL C++ Wrapper Library (GLWrappers).

The OpenGL C++ Wrapper Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The OpenGL C++ Wrapper Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the OpenGL C++ Wrapper Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <GL/GLGeometryVertex.icpp>

namespace {

/****************************************************************************************
Specialized versions of helper class for vertex types compatible with interleaved arrays:
****************************************************************************************/

template <>
class GLVertexPointerImplementation<GLfloat,4,
                                    GLfloat,4,
                                    GLfloat,
                                    GLfloat,4>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,4,
	                           GLfloat,4,
	                           GLfloat,
	                           GLfloat,4> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<4>(vertex.texCoord.getComponents());
		glColor<4>(vertex.color.getRgba());
		glNormal<3>(vertex.normal.getComponents());
		::glVertex<4>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T4F_C4F_N3F_V4F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Color|GLVertexArrayParts::Normal|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T4F_C4F_N3F_V4F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(4,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(4,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),vertices[0].normal.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(4,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<GLfloat,2,
                                    GLfloat,4,
                                    GLfloat,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,2,
	                           GLfloat,4,
	                           GLfloat,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<2>(vertex.texCoord.getComponents());
		glColor<4>(vertex.color.getRgba());
		glNormal<3>(vertex.normal.getComponents());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T2F_C4F_N3F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Color|GLVertexArrayParts::Normal|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T2F_C4F_N3F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(2,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(4,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),vertices[0].normal.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<GLfloat,2,
                                    void,0,
                                    GLfloat,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,2,
	                           void,0,
	                           GLfloat,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<2>(vertex.texCoord.getComponents());
		glNormal<3>(vertex.normal.getComponents());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T2F_N3F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Normal|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T2F_N3F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(2,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),vertices[0].normal.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<GLfloat,2,
                                    GLfloat,3,
                                    void,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,2,
	                           GLfloat,3,
	                           void,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<2>(vertex.texCoord.getComponents());
		glColor<3>(vertex.color.getRgba());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T2F_C3F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Color|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T2F_C3F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(2,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(3,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<GLfloat,2,
                                    GLubyte,4,
                                    void,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,2,
	                           GLubyte,4,
	                           void,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<2>(vertex.texCoord.getComponents());
		glColor<4>(vertex.color.getRgba());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T2F_C4UB_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Color|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T2F_C4UB_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(2,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(4,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<GLfloat,4,
                                    void,0,
                                    void,
                                    GLfloat,4>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,4,
	                           void,0,
	                           void,
	                           GLfloat,4> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<4>(vertex.texCoord.getComponents());
		::glVertex<4>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T4F_V4F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T4F_V4F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(4,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(4,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<GLfloat,2,
                                    void,0,
                                    void,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<GLfloat,2,
	                           void,0,
	                           void,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord<2>(vertex.texCoord.getComponents());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_T2F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::TexCoord|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_T2F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::TexCoord)
				glTexCoordPointer(2,sizeof(vertices[0]),vertices[0].texCoord.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    GLfloat,4,
                                    GLfloat,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           GLfloat,4,
	                           GLfloat,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor<4>(vertex.color.getRgba());
		glNormal<3>(vertex.normal.getComponents());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_C4F_N3F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Color|GLVertexArrayParts::Normal|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_C4F_N3F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(4,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),vertices[0].normal.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    void,0,
                                    GLfloat,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           void,0,
	                           GLfloat,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glNormal<3>(vertex.normal.getComponents());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_N3F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Normal|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_N3F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),vertices[0].normal.getComponents());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    GLfloat,3,
                                    void,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           GLfloat,3,
	                           void,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor<3>(vertex.color.getRgba());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_C3F_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Color|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_C3F_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(3,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    GLubyte,4,
                                    void,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           GLubyte,4,
	                           void,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor<4>(vertex.color.getRgba());
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_C4UB_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Color|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_C4UB_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(4,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    GLubyte,4,
                                    void,
                                    GLfloat,2>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           GLubyte,4,
	                           void,
	                           GLfloat,2> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor<4>(vertex.color.getRgba());
		::glVertex<2>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_C4UB_V2F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Color|GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_C4UB_V2F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(4,sizeof(vertices[0]),vertices[0].color.getRgba());
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(2,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    void,0,
                                    void,
                                    GLfloat,3>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           void,0,
	                           void,
	                           GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		::glVertex<3>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_V3F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_V3F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(3,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

template <>
class GLVertexPointerImplementation<void,0,
                                    void,0,
                                    void,
                                    GLfloat,2>
	{
	/* Embedded classes: */
	public:
	typedef GLGeometry::Vertex<void,0,
	                           void,0,
	                           void,
	                           GLfloat,2> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		::glVertex<2>(vertex.position.getComponents());
		}
	static void glVertexPointer(const Vertex* vertices)
		{
		glInterleavedArrays(GL_V2F,0,vertices);
		}
	static void glVertexPointer(int vertexPartsMask,const Vertex* vertices)
		{
		const int fullMask=GLVertexArrayParts::Position;
		if((vertexPartsMask&fullMask)==fullMask)
			glInterleavedArrays(GL_V2F,0,vertices);
		else
			{
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(2,sizeof(vertices[0]),vertices[0].position.getComponents());
			}
		}
	};

}

/******************************************************************
Force instantiation of standard glVertex/glVertexPointer templates:
******************************************************************/

namespace GLGeometry {

template class Vertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>;
template class Vertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>;
template class Vertex<GLfloat,2,void,0,GLfloat,GLfloat,3>;
template class Vertex<GLfloat,2,GLfloat,3,void,GLfloat,3>;
template class Vertex<GLfloat,2,GLubyte,4,void,GLfloat,3>;
template class Vertex<GLfloat,4,void,0,void,GLfloat,4>;
template class Vertex<GLfloat,2,void,0,void,GLfloat,3>;
template class Vertex<void,0,GLfloat,4,GLfloat,GLfloat,3>;
template class Vertex<void,0,void,0,GLfloat,GLfloat,3>;
template class Vertex<void,0,GLfloat,3,void,GLfloat,3>;
template class Vertex<void,0,GLubyte,4,void,GLfloat,3>;
template class Vertex<void,0,GLubyte,4,void,GLfloat,2>;
template class Vertex<void,0,void,0,void,GLfloat,3>;
template class Vertex<void,0,void,0,void,GLfloat,2>;

}

template void glVertex(const GLGeometry::Vertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>&);
template void glVertex(const GLGeometry::Vertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<GLfloat,2,void,0,GLfloat,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<GLfloat,2,GLfloat,3,void,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<GLfloat,2,GLubyte,4,void,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<GLfloat,4,void,0,void,GLfloat,4>&);
template void glVertex(const GLGeometry::Vertex<GLfloat,2,void,0,void,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<void,0,GLfloat,4,GLfloat,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<void,0,void,0,GLfloat,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<void,0,GLfloat,3,void,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<void,0,GLubyte,4,void,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<void,0,GLubyte,4,void,GLfloat,2>&);
template void glVertex(const GLGeometry::Vertex<void,0,void,0,void,GLfloat,3>&);
template void glVertex(const GLGeometry::Vertex<void,0,void,0,void,GLfloat,2>&);

template void glVertexPointer(const GLGeometry::Vertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>*);
template void glVertexPointer(const GLGeometry::Vertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<GLfloat,2,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<GLfloat,2,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<GLfloat,2,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<GLfloat,4,void,0,void,GLfloat,4>*);
template void glVertexPointer(const GLGeometry::Vertex<GLfloat,2,void,0,void,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,GLubyte,4,void,GLfloat,2>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,void,0,void,GLfloat,3>*);
template void glVertexPointer(const GLGeometry::Vertex<void,0,void,0,void,GLfloat,2>*);

template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,2,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,2,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,2,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,4,void,0,void,GLfloat,4>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<GLfloat,2,void,0,void,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,GLubyte,4,void,GLfloat,2>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,void,0,void,GLfloat,3>*);
template void glVertexPointer(int vertexPartsMask,const GLGeometry::Vertex<void,0,void,0,void,GLfloat,2>*);
