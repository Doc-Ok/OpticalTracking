/***********************************************************************
DrawEnvironment - Simple application to visualize the configuration of a
Vrui environment.
Copyright (c) 2013 Oliver Kreylos

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
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/Rotation.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/Plane.h>
#include <GL/gl.h>
#include <GL/GLColor.h>
#include <GL/GLModels.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Button.h>
#include <GLMotif/Menu.h>
#include <GLMotif/PopupMenu.h>
#include <Vrui/Vrui.h>
#include <Vrui/VRScreen.h>
#include <Vrui/Viewer.h>
#include <Vrui/Application.h>

class DrawEnvironment:public Vrui::Application
	{
	/* Elements: */
	private:
	GLMotif::PopupMenu* mainMenu; // The program's main menu
	
	/* Private methods: */
	GLMotif::PopupMenu* createMainMenu(void); // Creates the program's main menu
	void resetNavigationCallback(Misc::CallbackData* cbData);
	void drawArrow(const Vrui::Point& from,const Vrui::Point& to,Vrui::Scalar radius) const; // Draws an arrow between two points
	
	/* Constructors and destructors: */
	public:
	DrawEnvironment(int& argc,char**& argv);
	virtual ~DrawEnvironment(void);
	
	/* Methods from Vrui::Application: */
	virtual void display(GLContextData& contextData) const;
	};

/********************************
Methods of class DrawEnvironment:
********************************/

GLMotif::PopupMenu* DrawEnvironment::createMainMenu(void)
	{
	/* Create a popup shell to hold the main menu: */
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("Draw Environment");
	
	/* Create the main menu itself: */
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	/* Create a button: */
	GLMotif::Button* resetNavigationButton=new GLMotif::Button("ResetNavigationButton",mainMenu,"Reset Navigation");
	
	/* Add a callback to the button: */
	resetNavigationButton->getSelectCallbacks().add(this,&DrawEnvironment::resetNavigationCallback);
	
	/* Finish building the main menu: */
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

void DrawEnvironment::resetNavigationCallback(Misc::CallbackData* cbData)
	{
	/* Go back to physical space: */
	Vrui::setNavigationTransformation(Vrui::NavTransform::identity);
	}

void DrawEnvironment::drawArrow(const Vrui::Point& from,const Vrui::Point& to,Vrui::Scalar radius) const
	{
	Vrui::Scalar tipHeight=radius*Vrui::Scalar(6.0);
	Vrui::Scalar shaftLength=Geometry::dist(from,to)-tipHeight;
	
	glPushMatrix();
	glTranslate(from-Vrui::Point::origin);
	glRotate(Vrui::Rotation::rotateFromTo(Vrui::Vector(0,0,1),to-from));
	glTranslate(Vrui::Vector(0,0,Math::div2(shaftLength)));
	glDrawCylinder(radius,shaftLength,24);
	glTranslate(Vrui::Vector(0,0,Math::div2(shaftLength)+tipHeight*Vrui::Scalar(0.25)));
	glDrawCone(radius*Vrui::Scalar(2),tipHeight,24);
	glPopMatrix();
	}

DrawEnvironment::DrawEnvironment(int& argc,char**& argv)
	:Vrui::Application(argc,argv),
	 mainMenu(0)
	{
	/* Create the user interface: */
	mainMenu=createMainMenu();
	
	/* Install the main menu: */
	Vrui::setMainMenu(mainMenu);
	
	/* Set the navigation transformation: */
	resetNavigationCallback(0);
	}

DrawEnvironment::~DrawEnvironment(void)
	{
	/* Destroy the user interface: */
	delete mainMenu;
	}

void DrawEnvironment::display(GLContextData& contextData) const
	{
	glPushAttrib(GL_ENABLE_BIT|GL_LIGHTING_BIT|GL_LINE_BIT|GL_POINT_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	
	Vrui::Scalar arrowRadius=Vrui::getInchFactor()*Vrui::Scalar(0.125);
	
	/* Draw basic layout parameters: */
	
	/* Draw the floor plane: */
	Vrui::Point floorCenter=Vrui::getFloorPlane().project(Vrui::getDisplayCenter());
	Vrui::Vector floorX=Geometry::normalize(Geometry::cross(Vrui::getForwardDirection(),Vrui::getFloorPlane().getNormal()))*Vrui::getDisplaySize()*Vrui::Scalar(5);
	Vrui::Vector floorY=Geometry::normalize(Geometry::cross(Vrui::getFloorPlane().getNormal(),floorX))*Vrui::getDisplaySize()*Vrui::Scalar(5);
	glBegin(GL_QUADS);
	glColor3f(0.0f,0.5f,0.0f);
	glNormal(Vrui::getFloorPlane().getNormal());
	glVertex(floorCenter-floorX-floorY);
	glVertex(floorCenter+floorX-floorY);
	glVertex(floorCenter+floorX+floorY);
	glVertex(floorCenter-floorX+floorY);
	
	glColor3f(0.0f,0.0625f,0.0f);
	glNormal(-Vrui::getFloorPlane().getNormal());
	glVertex(floorCenter-floorX-floorY);
	glVertex(floorCenter-floorX+floorY);
	glVertex(floorCenter+floorX+floorY);
	glVertex(floorCenter+floorX-floorY);
	glEnd();
	
	/* Draw the forward direction: */
	glColor3f(1.0f,0.5f,0.5f);
	drawArrow(Vrui::getDisplayCenter(),Vrui::getDisplayCenter()+Vrui::getForwardDirection()*Math::div2(Vrui::getDisplaySize()),arrowRadius);
	
	/* Draw the up direction: */
	glColor3f(0.5f,1.0f,0.5f);
	drawArrow(Vrui::getDisplayCenter(),Vrui::getDisplayCenter()+Vrui::getUpDirection()*Math::div2(Vrui::getDisplaySize()),arrowRadius);
	
	/* Draw all screens: */
	for(int screenIndex=0;screenIndex<Vrui::getNumScreens();++screenIndex)
		{
		const Vrui::VRScreen& screen=*Vrui::getScreen(screenIndex);
		
		/* Go to the screen's coordinate system: */
		glPushMatrix();
		glMultMatrix(screen.getScreenTransformation());
		GLfloat w=GLfloat(screen.getWidth());
		GLfloat h=GLfloat(screen.getHeight());
		
		/* Draw the screen: */
		glBegin(GL_QUADS);
		glColor3f(0.5f,0.5f,0.5f);
		glNormal3f(0.0f,0.0f,1.0f);
		glVertex2f(0.0f,0.0f);
		glVertex2f(w,0.0f);
		glVertex2f(w,h);
		glVertex2f(0.0f,h);
		
		glColor3f(0.0625f,0.0625f,0.0625f);
		glNormal3f(0.0f,0.0f,-1.0f);
		glVertex2f(0.0f,0.0f);
		glVertex2f(0.0f,h);
		glVertex2f(w,h);
		glVertex2f(w,0.0f);
		glEnd();
		
		glPopMatrix();
		}
	
	/* Draw all viewers: */
	GLColor<GLubyte,3> eyeColors[16*2];
	for(int y=0;y<16;++y)
		for(int x=0;x<2;++x)
			{
			if(y<13)
				eyeColors[y*2+x]=GLColor<GLubyte,3>(255,255,255);
			else if(y<15)
				eyeColors[y*2+x]=GLColor<GLubyte,3>(64,16,8);
			else
				eyeColors[y*2+x]=GLColor<GLubyte,3>(0,0,0);
			}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,2,16,0,GL_RGB,GL_UNSIGNED_BYTE,eyeColors);
	
	for(int viewerIndex=0;viewerIndex<Vrui::getNumViewers();++viewerIndex)
		{
		const Vrui::Viewer& viewer=*Vrui::getViewer(viewerIndex);
		Vrui::Rotation eyeRot=Vrui::Rotation::rotateFromTo(Vrui::Vector(0,0,1),viewer.getViewDirection());
		Vrui::Scalar eyeRadius=Vrui::getInchFactor()*Vrui::Scalar(0.5);
		
		/* Draw the viewing direction: */
		glColor3f(0.5f,0.5f,0.5f);
		drawArrow(viewer.getEyePosition(Vrui::Viewer::MONO),viewer.getEyePosition(Vrui::Viewer::MONO)+viewer.getViewDirection()*Vrui::getInchFactor()*Vrui::Scalar(2),arrowRadius);
		
		/* Draw the viewer's eyes: */
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
		glColor3f(1.0f,1.0f,1.0f);
		
		/* Draw the left eye: */
		glPushMatrix();
		glTranslate(viewer.getEyePosition(Vrui::Viewer::LEFT)-Vrui::Point::origin);
		glRotate(eyeRot);
		glTranslate(Vrui::Vector(0,0,-eyeRadius));
		glDrawSphereMercatorWithTexture(eyeRadius,12,24);
		glPopMatrix();
		
		/* Draw the right eye: */
		glPushMatrix();
		glTranslate(viewer.getEyePosition(Vrui::Viewer::RIGHT)-Vrui::Point::origin);
		glRotate(eyeRot);
		glTranslate(Vrui::Vector(0,0,-eyeRadius));
		glDrawSphereMercatorWithTexture(eyeRadius,12,24);
		glPopMatrix();
		
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
		glDisable(GL_TEXTURE_2D);
		}
	
	/* Draw all input devices: */
	for(int deviceIndex=0;deviceIndex<Vrui::getNumInputDevices();++deviceIndex)
		{
		const Vrui::InputDevice& device=*Vrui::getInputDevice(deviceIndex);
		
		Vrui::Scalar arrowLen=Vrui::getInchFactor()*Vrui::Scalar(2);
		
		/* Draw the device's coordinate frame: */
		Vrui::Point devicePos=device.getPosition();
		glColor3f(1.0f,0.5f,0.5f);
		drawArrow(devicePos,devicePos+device.getOrientation().getDirection(0)*arrowLen,arrowRadius);
		glColor3f(0.5f,1.0f,0.5f);
		drawArrow(devicePos,devicePos+device.getOrientation().getDirection(1)*arrowLen,arrowRadius);
		glColor3f(0.5f,0.5f,1.0f);
		drawArrow(devicePos,devicePos+device.getOrientation().getDirection(2)*arrowLen,arrowRadius);
		
		/* Draw the device's ray direction: */
		glColor3f(0.5f,0.5f,0.5f);
		drawArrow(devicePos,devicePos+device.getRayDirection()*arrowLen*Vrui::Scalar(1.5),arrowRadius);
		}
	
	/* Draw the display volume: */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	
	glColor4f(0.5f,0.5f,0.0f,0.333f);
	glPushMatrix();
	glTranslate(Vrui::getDisplayCenter()-Vrui::Point::origin);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
	glCullFace(GL_FRONT);
	glDrawSphereIcosahedron(Vrui::getDisplaySize(),12);
	glCullFace(GL_BACK);
	glDrawSphereIcosahedron(Vrui::getDisplaySize(),12);
	glPopMatrix();
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	
	glPopAttrib();
	}

/* Create and execute an application object: */
VRUI_APPLICATION_RUN(DrawEnvironment)
