/***********************************************************************
GLVertex - Class to encapsulate OpenGL vertex properties.
Copyright (c) 2004-2010 Oliver Kreylos

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

#include <GL/GLVertex.icpp>

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
	typedef GLVertex<GLfloat,4,
	                 GLfloat,4,
	                 GLfloat,
	                 GLfloat,4> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		glColor(vertex.color);
		glNormal(vertex.normal);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),&vertices[0].normal);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<GLfloat,2,
	                 GLfloat,4,
	                 GLfloat,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		glColor(vertex.color);
		glNormal(vertex.normal);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),&vertices[0].normal);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<GLfloat,2,
	                 void,0,
	                 GLfloat,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		glNormal(vertex.normal);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),&vertices[0].normal);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<GLfloat,2,
	                 GLfloat,3,
	                 void,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		glColor(vertex.color);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<GLfloat,2,
	                 GLubyte,4,
	                 void,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		glColor(vertex.color);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Color)
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<GLfloat,4,
	                 void,0,
	                 void,
	                 GLfloat,4> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<GLfloat,2,
	                 void,0,
	                 void,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glTexCoord(vertex.texCoord);
		::glVertex(vertex.position);
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
				glTexCoordPointer(sizeof(vertices[0]),&vertices[0].texCoord);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 GLfloat,4,
	                 GLfloat,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor(vertex.color);
		glNormal(vertex.normal);
		::glVertex(vertex.position);
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
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Normal)
				glNormalPointer(sizeof(vertices[0]),&vertices[0].normal);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 void,0,
	                 GLfloat,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glNormal(vertex.normal);
		::glVertex(vertex.position);
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
				glNormalPointer(sizeof(vertices[0]),&vertices[0].normal);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 GLfloat,3,
	                 void,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor(vertex.color);
		::glVertex(vertex.position);
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
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 GLubyte,4,
	                 void,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor(vertex.color);
		::glVertex(vertex.position);
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
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 GLubyte,4,
	                 void,
	                 GLfloat,2> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		glColor(vertex.color);
		::glVertex(vertex.position);
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
				glColorPointer(sizeof(vertices[0]),&vertices[0].color);
			if(vertexPartsMask&GLVertexArrayParts::Position)
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 void,0,
	                 void,
	                 GLfloat,3> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		::glVertex(vertex.position);
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
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
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
	typedef GLVertex<void,0,
	                 void,0,
	                 void,
	                 GLfloat,2> Vertex;
	
	/* Methods: */
	static void glVertex(const Vertex& vertex)
		{
		::glVertex(vertex.position);
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
				::glVertexPointer(sizeof(vertices[0]),&vertices[0].position);
			}
		}
	};

}

/******************************************************************
Force instantiation of standard glVertex/glVertexPointer templates:
******************************************************************/

template class GLVertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>;
template class GLVertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>;
template class GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3>;
template class GLVertex<GLfloat,2,GLfloat,3,void,GLfloat,3>;
template class GLVertex<GLfloat,2,GLubyte,4,void,GLfloat,3>;
template class GLVertex<GLfloat,4,void,0,void,GLfloat,4>;
template class GLVertex<GLfloat,2,void,0,void,GLfloat,3>;
template class GLVertex<void,0,GLfloat,4,GLfloat,GLfloat,3>;
template class GLVertex<void,0,void,0,GLfloat,GLfloat,3>;
template class GLVertex<void,0,GLfloat,3,void,GLfloat,3>;
template class GLVertex<void,0,GLubyte,4,void,GLfloat,3>;
template class GLVertex<void,0,GLubyte,4,void,GLfloat,2>;
template class GLVertex<void,0,void,0,void,GLfloat,3>;
template class GLVertex<void,0,void,0,void,GLfloat,2>;

template void glVertex(const GLVertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>&);
template void glVertex(const GLVertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>&);
template void glVertex(const GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3>&);
template void glVertex(const GLVertex<GLfloat,2,GLfloat,3,void,GLfloat,3>&);
template void glVertex(const GLVertex<GLfloat,2,GLubyte,4,void,GLfloat,3>&);
template void glVertex(const GLVertex<GLfloat,4,void,0,void,GLfloat,4>&);
template void glVertex(const GLVertex<GLfloat,2,void,0,void,GLfloat,3>&);
template void glVertex(const GLVertex<void,0,GLfloat,4,GLfloat,GLfloat,3>&);
template void glVertex(const GLVertex<void,0,void,0,GLfloat,GLfloat,3>&);
template void glVertex(const GLVertex<void,0,GLfloat,3,void,GLfloat,3>&);
template void glVertex(const GLVertex<void,0,GLubyte,4,void,GLfloat,3>&);
template void glVertex(const GLVertex<void,0,GLubyte,4,void,GLfloat,2>&);
template void glVertex(const GLVertex<void,0,void,0,void,GLfloat,3>&);
template void glVertex(const GLVertex<void,0,void,0,void,GLfloat,2>&);

template void glVertexPointer(const GLVertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>*);
template void glVertexPointer(const GLVertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLVertex<GLfloat,2,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(const GLVertex<GLfloat,2,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(const GLVertex<GLfloat,4,void,0,void,GLfloat,4>*);
template void glVertexPointer(const GLVertex<GLfloat,2,void,0,void,GLfloat,3>*);
template void glVertexPointer(const GLVertex<void,0,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLVertex<void,0,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(const GLVertex<void,0,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(const GLVertex<void,0,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(const GLVertex<void,0,GLubyte,4,void,GLfloat,2>*);
template void glVertexPointer(const GLVertex<void,0,void,0,void,GLfloat,3>*);
template void glVertexPointer(const GLVertex<void,0,void,0,void,GLfloat,2>*);

template void glVertexPointer(int,const GLVertex<GLfloat,4,GLfloat,4,GLfloat,GLfloat,4>*);
template void glVertexPointer(int,const GLVertex<GLfloat,2,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<GLfloat,2,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<GLfloat,2,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<GLfloat,2,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<GLfloat,4,void,0,void,GLfloat,4>*);
template void glVertexPointer(int,const GLVertex<GLfloat,2,void,0,void,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<void,0,GLfloat,4,GLfloat,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<void,0,void,0,GLfloat,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<void,0,GLfloat,3,void,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<void,0,GLubyte,4,void,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<void,0,GLubyte,4,void,GLfloat,2>*);
template void glVertexPointer(int,const GLVertex<void,0,void,0,void,GLfloat,3>*);
template void glVertexPointer(int,const GLVertex<void,0,void,0,void,GLfloat,2>*);
