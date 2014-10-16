/***********************************************************************
Small image viewer using Vrui.
Copyright (c) 2011-2013 Oliver Kreylos

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

#include <Math/Math.h>
#include <GL/gl.h>
#include <GL/GLMaterial.h>
#include <GL/GLObject.h>
#include <GL/GLContextData.h>
#include <GL/Extensions/GLARBTextureNonPowerOfTwo.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>
#include <GLMotif/Button.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupMenu.h>
#include <Vrui/Vrui.h>
#include <Vrui/Application.h>
#include <Vrui/OpenFile.h>

class ImageViewer:public Vrui::Application,public GLObject
	{
	/* Embedded classes: */
	private:
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint imageTextureId; // ID of image texture object
		GLfloat texMin[2],texMax[2]; // Texture coordinate rectangle to render the image texture
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	Images::RGBImage image; // The displayed image
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	void resetNavigationCallback(Misc::CallbackData* cbData); // Method to reset the Vrui navigation transformation to its default
	
	/* Constructors and destructors: */
	public:
	ImageViewer(int& argc,char**& argv);
	virtual ~ImageViewer(void);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	
	/* Methods from GLObject: */
	virtual void initContext(GLContextData& contextData) const;
	};

/**************************************
Methods of class ImageViewer::DataItem:
**************************************/

ImageViewer::DataItem::DataItem(void)
	:imageTextureId(0)
	{
	glGenTextures(1,&imageTextureId);
	}

ImageViewer::DataItem::~DataItem(void)
	{
	glDeleteTextures(1,&imageTextureId);
	}

/****************************
Methods of class ImageViewer:
****************************/

GLMotif::PopupMenu* ImageViewer::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Image Viewer");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create a button: */
	GLMotif::Button* resetNavigationButton=new GLMotif::Button("ResetNavigationButton",mainMenu,"Reset Navigation");
	
	/* Add a callback to the button: */
	resetNavigationButton->getSelectCallbacks().add(this,&ImageViewer::resetNavigationCallback);
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void ImageViewer::resetNavigationCallback(Misc::CallbackData* cbData)
	{
	/* Reset the Vrui navigation transformation: */
	Vrui::Scalar w=Vrui::Scalar(image.getSize(0));
	Vrui::Scalar h=Vrui::Scalar(image.getSize(1));
	Vrui::Point center(Math::div2(w),Math::div2(h),Vrui::Scalar(0));
	Vrui::Scalar size=Math::sqrt(Math::sqr(w)+Math::sqr(h));
	Vrui::setNavigationTransformation(center,size,Vrui::Vector(0,1,0));
	}

ImageViewer::ImageViewer(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 mainMenu(0)
	{
	/* Load the image: */
	image=Images::readImageFile(argv[1],Vrui::openFile(argv[1]));
	
	/* Create the user interface: */
	mainMenu=createMainMenu();
	
	/* Install the main menu: */
	Vrui::setMainMenu(mainMenu);
	
	/* Initialize the navigation transformation: */
	resetNavigationCallback(0);
	}

ImageViewer::~ImageViewer(void)
	{
	delete mainMenu;
	}

void ImageViewer::display(GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state: */
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->imageTextureId);
	
	/* Draw the image: */
	glBegin(GL_QUADS);
	glTexCoord2f(dataItem->texMin[0],dataItem->texMin[1]);
	glVertex2i(0,0);
	glTexCoord2f(dataItem->texMax[0],dataItem->texMin[1]);
	glVertex2i(image.getSize(0),0);
	glTexCoord2f(dataItem->texMax[0],dataItem->texMax[1]);
	glVertex2i(image.getSize(0),image.getSize(1));
	glTexCoord2f(dataItem->texMin[0],dataItem->texMax[1]);
	glVertex2i(0,image.getSize(1));
	glEnd();
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	
	/* Draw the image's backside: */
	glDisable(GL_TEXTURE_2D);
	glMaterial(GLMaterialEnums::FRONT,GLMaterial(GLMaterial::Color(0.7f,0.7f,0.7f)));
	
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex2i(0,0);
	glVertex2i(0,image.getSize(1));
	glVertex2i(image.getSize(0),image.getSize(1));
	glVertex2i(image.getSize(0),0);
	glEnd();
	
	/* Restore OpenGL state: */
	glPopAttrib();
	}

void ImageViewer::initContext(GLContextData& contextData) const
	{
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Check whether non-power-of-two-dimension textures are supported: */
	bool haveNpotdt=GLARBTextureNonPowerOfTwo::isSupported();
	if(haveNpotdt)
		GLARBTextureNonPowerOfTwo::initExtension();
	
	/* Calculate the texture coordinate rectangle: */
	unsigned int texSize[2];
	if(haveNpotdt)
		{
		for(int i=0;i<2;++i)
			texSize[i]=image.getSize(i);
		}
	else
		{
		for(int i=0;i<2;++i)
			for(texSize[i]=1U;texSize[i]<image.getSize(i);texSize[i]<<=1)
				;
		}
	for(int i=0;i<2;++i)
		{
		dataItem->texMin[i]=0.0f;
		dataItem->texMax[i]=GLfloat(image.getSize(i))/GLfloat(texSize[i]);
		}
	
	/* Bind the texture object: */
	glBindTexture(GL_TEXTURE_2D,dataItem->imageTextureId);
	
	/* Initialize basic texture settings: */
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_BASE_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAX_LEVEL,0);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	
	/* Upload the texture image: */
	image.glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,!haveNpotdt);
	
	/* Protect the texture object: */
	glBindTexture(GL_TEXTURE_2D,0);
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(ImageViewer)
