/***********************************************************************
Test program to show the initialization sequence of Vrui with regard to
per-context state of objects derived from GLObject.
Copyright (c) 2007-2013 Oliver Kreylos

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

#include <iostream>
#include <GL/GLObject.h>
#include <GL/GLContextData.h>
#include <Vrui/Application.h>

class VruiGLTest:public Vrui::Application,public GLObject
	{
	/* Embedded classes: */
	private:
	class Renderer:public GLObject
		{
		/* Embedded classes: */
		private:
		class DataItem:public GLObject::DataItem
			{
			/* Constructors and destructors: */
			public:
			DataItem(void);
			virtual ~DataItem(void);
			};
		
		/* Constructors and destructors: */
		public:
		Renderer(void);
		virtual ~Renderer(void);
		
		/* Methods: */
		virtual void initContext(GLContextData& contextData) const;
		virtual void glRenderAction(GLContextData& contextData) const;
		};
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		private:
		
		/* Constructors and destructors: */
		public:
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	Renderer renderer1; // Embedded renderer object
	Renderer* renderer2; // Linked renderer object
	
	/* Constructors and destructors: */
	public:
	VruiGLTest(int& argc,char**& argv);
	virtual ~VruiGLTest(void);
	
	/* Methods: */
	virtual void initContext(GLContextData& contextData) const;
	virtual void frame(void);
	virtual void display(GLContextData& contextData) const;
	};

/***********************************************
Methods of class VruiGLTest::Renderer::DataItem:
***********************************************/

VruiGLTest::Renderer::DataItem::DataItem(void)
	{
	std::cout<<"Renderer::DataItem constructed at "<<this<<std::endl;
	}

VruiGLTest::Renderer::DataItem::~DataItem(void)
	{
	std::cout<<"Renderer::DataItem destructed at "<<this<<std::endl;
	}

/*************************************
Methods of class VruiGLTest::Renderer:
*************************************/

VruiGLTest::Renderer::Renderer(void)
	{
	std::cout<<"Renderer constructed at "<<this<<std::endl;
	}

VruiGLTest::Renderer::~Renderer(void)
	{
	std::cout<<"Renderer destructed at "<<this<<std::endl;
	}

void VruiGLTest::Renderer::initContext(GLContextData& contextData) const
	{
	std::cout<<"Renderer::initContext called for "<<this<<std::endl;
	
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void VruiGLTest::Renderer::glRenderAction(GLContextData& contextData) const
	{
	std::cout<<"Renderer::glRenderAction called for "<<this<<std::endl;
	}

/*************************************
Methods of class VruiGLTest::DataItem:
*************************************/

VruiGLTest::DataItem::DataItem(void)
	{
	std::cout<<"VruiGLTest::DataItem constructed at "<<this<<std::endl;
	}

VruiGLTest::DataItem::~DataItem(void)
	{
	std::cout<<"VruiGLTest::DataItem destructed at "<<this<<std::endl;
	}

/***************************
Methods of class VruiGLTest:
***************************/

VruiGLTest::VruiGLTest(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 renderer2(0)
	{
	std::cout<<"VruiGLTest constructed at "<<this<<std::endl;
	
	renderer2=new Renderer;
	}

VruiGLTest::~VruiGLTest(void)
	{
	delete renderer2;
	
	std::cout<<"VruiGLTest destructed at "<<this<<std::endl;
	}

void VruiGLTest::initContext(GLContextData& contextData) const
	{
	std::cout<<"VruiGLTest::initContext called"<<std::endl;
	
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

void VruiGLTest::frame(void)
	{
	std::cout<<"VruiGLTest::frame called"<<std::endl;
	}

void VruiGLTest::display(GLContextData& contextData) const
	{
	std::cout<<"VruiGLTest::display called"<<std::endl;
	
	renderer1.glRenderAction(contextData);
	renderer2->glRenderAction(contextData);
	}

VRUI_APPLICATION_RUN(VruiGLTest)
