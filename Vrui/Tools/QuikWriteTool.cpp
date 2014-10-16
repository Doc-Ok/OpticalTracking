/***********************************************************************
QuikWriteTool - Class for tools to enter text using the stroke-based
QuikWrite user interface, developed by Ken Perlin.
Copyright (c) 2010-2014 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <Vrui/Tools/QuikWriteTool.h>

#include <Misc/ThrowStdErr.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Math/Math.h>
#include <Geometry/Ray.h>
#include <Geometry/SolidHitResult.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <GL/GLFont.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/TextEvent.h>
#include <GLMotif/TextControlEvent.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/ToolManager.h>

namespace Vrui {

/****************************************************
Methods of class QuikWriteToolFactory::Configuration:
****************************************************/

QuikWriteToolFactory::Configuration::Configuration(void)
	:useDevice(false),
	 squareSize(getUiFont()->getTextHeight()*Scalar(10)),
	 initialSquareDist(getInchFactor()*Scalar(3)),
	 backgroundColor(getBackgroundColor()),
	 drawPoint(false),pointColor(foregroundColor),pointSize(3.0f)
	{
	/* Initialize the foreground color: */
	for(int i=0;i<3;++i)
		foregroundColor[i]=Color::Scalar(1)-backgroundColor[i];
	foregroundColor[3]=Color::Scalar(1);
	}

void QuikWriteToolFactory::Configuration::read(const Misc::ConfigurationFileSection& cfs)
	{
	if(cfs.hasTag("./deviceName"))
		{
		useDevice=true;
		deviceName=cfs.retrieveString("./deviceName");
		}
	squareSize=cfs.retrieveValue<Scalar>("./squareSize",squareSize);
	initialSquareDist=cfs.retrieveValue<Scalar>("./initialSquareDist",initialSquareDist);
	backgroundColor=cfs.retrieveValue<Color>("./backgroundColor",backgroundColor);
	foregroundColor=cfs.retrieveValue<Color>("./foregroundColor",foregroundColor);
	drawPoint=cfs.retrieveValue<bool>("./drawPoint",drawPoint);
	pointColor=cfs.retrieveValue<Color>("./pointColor",pointColor);
	pointSize=cfs.retrieveValue<GLfloat>("./pointSize",pointSize);
	}

void QuikWriteToolFactory::Configuration::write(Misc::ConfigurationFileSection& cfs) const
	{
	if(useDevice)
		cfs.storeString("./deviceName",deviceName);
	cfs.storeValue<Scalar>("./squareSize",squareSize);
	cfs.storeValue<Scalar>("./initialSquareDist",initialSquareDist);
	cfs.storeValue<Color>("./backgroundColor",backgroundColor);
	cfs.storeValue<Color>("./foregroundColor",foregroundColor);
	cfs.storeValue<bool>("./drawPoint",drawPoint);
	cfs.storeValue<Color>("./pointColor",pointColor);
	cfs.storeValue<GLfloat>("./pointSize",pointSize);
	}

/*************************************
Methods of class QuikWriteToolFactory:
*************************************/

QuikWriteToolFactory::QuikWriteToolFactory(ToolManager& toolManager)
	:ToolFactory("QuikWriteTool",toolManager)
	{
	/* Initialize tool layout: */
	layout.setNumButtons(1);
	
	/* Insert class into class hierarchy: */
	ToolFactory* toolFactory=toolManager.loadClass("UserInterfaceTool");
	toolFactory->addChildClass(this);
	addParentClass(toolFactory);
	
	/* Load class settings: */
	config.read(toolManager.getToolClassSection(getClassName()));
	
	/* Set tool class' factory pointer: */
	QuikWriteTool::factory=this;
	}

QuikWriteToolFactory::~QuikWriteToolFactory(void)
	{
	/* Reset tool class' factory pointer: */
	QuikWriteTool::factory=0;
	}

const char* QuikWriteToolFactory::getName(void) const
	{
	return "QuikWrite";
	}

const char* QuikWriteToolFactory::getButtonFunction(int) const
	{
	return "Start / Stop";
	}

Tool* QuikWriteToolFactory::createTool(const ToolInputAssignment& inputAssignment) const
	{
	return new QuikWriteTool(this,inputAssignment);
	}

void QuikWriteToolFactory::destroyTool(Tool* tool) const
	{
	delete tool;
	}

extern "C" void resolveQuikWriteToolDependencies(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Load base classes: */
	manager.loadClass("UserInterfaceTool");
	}

extern "C" ToolFactory* createQuikWriteToolFactory(Plugins::FactoryManager<ToolFactory>& manager)
	{
	/* Get pointer to tool manager: */
	ToolManager* toolManager=static_cast<ToolManager*>(&manager);
	
	/* Create factory object and insert it into class hierarchy: */
	QuikWriteToolFactory* quikWriteToolFactory=new QuikWriteToolFactory(*toolManager);
	
	/* Return factory object: */
	return quikWriteToolFactory;
	}

extern "C" void destroyQuikWriteToolFactory(ToolFactory* factory)
	{
	delete factory;
	}

/****************************************
Methods of class QuikWriteTool::DataItem:
****************************************/

QuikWriteTool::DataItem::DataItem(void)
	:squareListId(glGenLists(1))
	{
	}

QuikWriteTool::DataItem::~DataItem(void)
	{
	glDeleteLists(squareListId,1);
	}

/**************************************
Static elements of class QuikWriteTool:
**************************************/

QuikWriteToolFactory* QuikWriteTool::factory=0;

const char QuikWriteTool::characters[4][9][9]=
	{
	{ // Lowercase alphabet
	{'a','s','k','m',0,0,'q',0,0},
	{' ',' ',' ',0,0,0,0,0,0},
	{'p','f','n',0,0,'l',0,0,'x'},
	{'h',0,0,'e',0,0,'c',0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,'u',0,0,'t',0,0,'y'},
	{'v',0,0,'w',0,0,'o','g','z'},
	{0,0,0,0,0,0,' ',' ',' '},
	{0,0,'j',0,0,'r','b','d','i'}
	},
	{ // Uppercase alphabet
	{'A','S','K','M',0,0,'Q',0,0},
	{' ',' ',' ',0,0,0,0,0,0},
	{'P','F','N',0,0,'L',0,0,'X'},
	{'H',0,0,'E',0,0,'C',0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,'U',0,0,'T',0,0,'Y'},
	{'V',0,0,'W',0,0,'O','G','Z'},
	{0,0,0,0,0,0,' ',' ',' '},
	{0,0,'J',0,0,'R','B','D','I'}
	},
	{ // Punctuation alphabet
	{'?','{','`','/',0,0,'\\',0,0},
	{' ',' ',' ',0,0,0,0,0,0},
	{'^','}','!',0,0,'*',0,0,'%'},
	{'\'',0,0,';',0,0,'_',0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,'"',0,0,':',0,0,'&'},
	{'|',0,0,'-',0,0,',','[','@'},
	{0,0,0,0,0,0,' ',' ',' '},
	{0,0,'$',0,0,'+','#',']','.'}
	},
	{ // Numeric alphabet
	{'1','{','(','/',0,0,'=',0,0},
	{' ',' ',' ',0,0,0,0,0,0},
	{')','}','2',0,0,'*',0,0,'%'},
	{'3',0,0,'5',0,0,'7',0,0},
	{0,0,0,0,0,0,0,0,0},
	{0,0,'4',0,0,'6',0,0,'8'},
	{'<',0,0,'9',0,0,',','[','-'},
	{0,0,0,0,0,0,' ',' ',' '},
	{0,0,'>',0,0,'0','+',']','.'}
	}
	};

/******************************
Methods of class QuikWriteTool:
******************************/

int QuikWriteTool::getZone(bool inZone5) const
	{
	/* Intersect the ray with the QuikWrite square: */
	Plane::HitResult hr=squarePlane.intersectRay(ray);
	if(hr.isValid())
		{
		/* Transform the intersection point to square space: */
		Point p=squareTransform.inverseTransform(ray(hr.getParameter()));
		
		/* Calculate the index of the zone containing the intersection point: */
		Scalar d=Math::sqrt(Math::sqr(p[0])+Math::sqr(p[1]));
		Scalar zone5Size=config.squareSize/Scalar(4);
		if(inZone5)
			zone5Size+=config.squareSize*Scalar(0.025);
		else
			zone5Size-=config.squareSize*Scalar(0.025);
		if(d<=zone5Size)
			return 5;
		else
			{
			/* Distinguish between zones 4, 1, 2, 3 and 6, 9, 8, 7: */
			if(p[0]*Math::sin(Math::rad(Scalar(-22.5)))+p[1]*Math::cos(Math::rad(Scalar(-22.5)))>=Scalar(0))
				{
				/* Distinguish between zones 4, 1 and 2, 3: */
				if(p[0]*Math::sin(Math::rad(Scalar(-112.5)))+p[1]*Math::cos(Math::rad(Scalar(-112.5)))>=Scalar(0))
					{
					/* Distinguish between zones 4 and 1: */
					if(p[0]*Math::sin(Math::rad(Scalar(-157.5)))+p[1]*Math::cos(Math::rad(Scalar(-157.5)))>=Scalar(0))
						return 4;
					else
						return 1;
					}
				else
					{
					/* Distinguish between zones 2 and 3: */
					if(p[0]*Math::sin(Math::rad(Scalar(-67.5)))+p[1]*Math::cos(Math::rad(Scalar(-67.5)))>=Scalar(0))
						return 2;
					else
						return 3;
					}
				}
			else
				{
				/* Distinguish between zones 6, 9 and 8, 7: */
				if(p[0]*Math::sin(Math::rad(Scalar(67.5)))+p[1]*Math::cos(Math::rad(Scalar(67.5)))>=Scalar(0))
					{
					/* Distinguish between zones 6 and 9: */
					if(p[0]*Math::sin(Math::rad(Scalar(22.5)))+p[1]*Math::cos(Math::rad(Scalar(22.5)))>=Scalar(0))
						return 6;
					else
						return 9;
					}
				else
					{
					/* Distinguish between zones 8 and 7: */
					if(p[0]*Math::sin(Math::rad(Scalar(112.5)))+p[1]*Math::cos(Math::rad(Scalar(112.5)))>=Scalar(0))
						return 8;
					else
						return 7;
					}
				}
			}
		}
	else
		return 0;
	}

void QuikWriteTool::setAlphabet(QuikWriteTool::Alphabet newAlphabet)
	{
	/* Bail out if no-op: */
	if(alphabet==newAlphabet)
		return;
	
	/* Set the alphabet: */
	alphabet=newAlphabet;
	
	/* Reinitialize the petal labels: */
	static const int petalMajors[32]={0,0,0,1,1,1,2,2,2,2,2,5,5,5,8,8,8,8,8,7,7,7,6,6,6,6,6,3,3,3,0,0};
	static const int petalMinors[32]={0,1,2,0,1,2,0,1,2,5,8,2,5,8,2,5,8,7,6,8,7,6,8,7,6,3,0,6,3,0,6,3};
	for(int i=0;i<32;++i)
		{
		char string[2];
		string[0]=characters[alphabet-LOWERCASE][petalMajors[i]][petalMinors[i]];
		string[1]='\0';
		petals[i].setString(string);
		GLLabel::Box::Vector labelSize=petals[i].getLabelSize();
		petals[i].setOrigin(GLLabel::Box::Vector(float(petalPos[i][0])-labelSize[0]*0.5f,float(petalPos[i][1])-labelSize[1]*0.5f,0.0f));
		}
	}

void QuikWriteTool::switchAlphabet(QuikWriteTool::Alphabet newAlphabet)
	{
	if(newAlphabet==alphabet)
		{
		if(alphabetLocked)
			{
			/* Go back to lowercase: */
			setAlphabet(LOWERCASE);
			}
		else
			{
			/* Lock the already selected alphabet: */
			alphabetLocked=true;
			}
		}
	else
		{
		/* Go to the newly-selected alphabet: */
		setAlphabet(newAlphabet);
		alphabetLocked=false;
		}
	}

void QuikWriteTool::drawRegion(int region) const
	{
	/* Calculate the square's layout: */
	Scalar squareSize=config.squareSize/Scalar(2);
	Scalar restSize=config.squareSize/Scalar(4);
	Scalar x1=Math::sin(Math::rad(22.5))*restSize;
	Scalar y1=Math::cos(Math::rad(22.5))*restSize;
	Scalar x2=Math::tan(Math::rad(22.5))*squareSize;
	Scalar y2=squareSize;
	
	glBegin(GL_POLYGON);
	switch(region)
		{
		case 1:
			glVertex(-y2,y2);
			glVertex(-y2,x2);
			glVertex(-y1,x1);
			glVertex(-x1,y1);
			glVertex(-x2,y2);
			break;
		
		case 2:
			glVertex(x2,y2);
			glVertex(-x2,y2);
			glVertex(-x1,y1);
			glVertex(x1,y1);
			break;
		
		case 3:
			glVertex(y2,y2);
			glVertex(x2,y2);
			glVertex(x1,y1);
			glVertex(y1,x1);
			glVertex(y2,x2);
			break;
		
		case 4:
			glVertex(-y2,x2);
			glVertex(-y2,-x2);
			glVertex(-y1,-x1);
			glVertex(-y1,x1);
			break;
		
		case 6:
			glVertex(y2,-x2);
			glVertex(y2,x2);
			glVertex(y1,x1);
			glVertex(y1,-x1);
			break;
		
		case 7:
			glVertex(-y2,-y2);
			glVertex(-x2,-y2);
			glVertex(-x1,-y1);
			glVertex(-y1,-x1);
			glVertex(-y2,-x2);
			break;
		
		case 8:
			glVertex(-x2,-y2);
			glVertex(x2,-y2);
			glVertex(x1,-y1);
			glVertex(-x1,-y1);
			break;
		
		case 9:
			glVertex(y2,-y2);
			glVertex(y2,-x2);
			glVertex(y1,-x1);
			glVertex(x1,-y1);
			glVertex(x2,-y2);
			break;
		}
	glEnd();
	}

void QuikWriteTool::drawSquare(void) const
	{
	/* Calculate the square's layout: */
	Scalar squareSize=config.squareSize/Scalar(2);
	Scalar restSize=config.squareSize/Scalar(4);
	Scalar x1=Math::sin(Math::rad(-22.5))*restSize;
	Scalar y1=Math::cos(Math::rad(-22.5))*restSize;
	Scalar x2=Math::tan(Math::rad(-22.5))*squareSize;
	Scalar y2=squareSize;
	
	Scalar sh=Math::div2(getUiFont()->getTextHeight());
	Scalar sw=sh/Math::div2(Math::sqrt(Scalar(5))+Scalar(1));
	Scalar sr=sw/Math::sqrt(Scalar(2));
	
	Scalar xs1=-squareSize+(Scalar(4))*squareSize/Scalar(5);
	Scalar xs2=-squareSize+(Scalar(5))*squareSize/Scalar(5);
	Scalar xs3=-squareSize+(Scalar(6))*squareSize/Scalar(5);
	Scalar ys1=squareSize-squareSize/Scalar(10);
	Scalar ys2=-squareSize+squareSize/Scalar(10);
	
	/* Draw the square: */
	glBegin(GL_LINES);
	glVertex(-x1,y1);
	glVertex(-x2,y2);
	glVertex(x1,y1);
	glVertex(x2,y2);
	glVertex(y1,x1);
	glVertex(y2,x2);
	glVertex(y1,-x1);
	glVertex(y2,-x2);
	glVertex(x1,-y1);
	glVertex(x2,-y2);
	glVertex(-x1,-y1);
	glVertex(-x2,-y2);
	glVertex(-y1,-x1);
	glVertex(-y2,-x2);
	glVertex(-y1,x1);
	glVertex(-y2,x2);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex(xs1-sw,ys1-sh);
	glVertex(xs1+sw,ys1-sh);
	glVertex(xs1+sw,ys1+sh);
	glVertex(xs1-sw,ys1+sh);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex(xs2-sw,ys1);
	glVertex(xs2+sw,ys1-sw);
	glVertex(xs2+sw,ys1+sw);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex(xs3-sw,ys1-sw);
	glVertex(xs3+sw,ys1-sw);
	glVertex(xs3,ys1+sw);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex(xs1-sw,ys2+sw);
	glVertex(xs1+sw,ys2+sw);
	glVertex(xs1,ys2-sw);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex(xs2-sw,ys2+sw);
	glVertex(xs2-sw,ys2-sw);
	glVertex(xs2+sw,ys2);
	glEnd();
	
	glBegin(GL_LINE_LOOP);
	glVertex(xs3-sw,ys2);
	glVertex(xs3-sr,ys2-sr);
	glVertex(xs3,ys2-sw);
	glVertex(xs3+sr,ys2-sr);
	glVertex(xs3+sw,ys2);
	glVertex(xs3+sr,ys2+sr);
	glVertex(xs3,ys2+sw);
	glVertex(xs3-sr,ys2+sr);
	glEnd();
	}

QuikWriteTool::QuikWriteTool(const ToolFactory* sFactory,const ToolInputAssignment& inputAssignment)
	:UserInterfaceTool(factory,inputAssignment),
	 config(factory->config),
	 active(false),
	 alphabet(UPPERCASE) // Evil hack so that setAlphabet below works
	{
	}

QuikWriteTool::~QuikWriteTool(void)
	{
	}

void QuikWriteTool::configure(const Misc::ConfigurationFileSection& configFileSection)
	{
	/* Call the base class method: */
	UserInterfaceTool::configure(configFileSection);
	
	/* Override the current configuration: */
	config.read(configFileSection);
	}

void QuikWriteTool::storeState(Misc::ConfigurationFileSection& configFileSection) const
	{
	/* Call the base class method: */
	UserInterfaceTool::storeState(configFileSection);
	
	/* Store the current configuration: */
	config.write(configFileSection);
	}

void QuikWriteTool::initialize(void)
	{
	/* Set the interaction device: */
	if(config.useDevice)
		{
		interactionDevice=findInputDevice(config.deviceName.c_str());
		if(interactionDevice==0)
			Misc::throwStdErr("QuikWriteTool: Interaction device %s not found",config.deviceName.c_str());
		}
	else
		interactionDevice=getButtonDevice(0);
	
	/* Initialize the petal positions: */
	Scalar squareSize=config.squareSize/Scalar(2);
	for(int i=0;i<8;++i)
		{
		Scalar x=-squareSize+(Scalar(i)+Scalar(0.5))*squareSize/Scalar(5);
		if(i>=3)
			x+=squareSize/Scalar(10);
		if(i>=6)
			x+=squareSize/Scalar(10);
		petalPos[0+i][0]=x;
		petalPos[0+i][1]=squareSize-squareSize/Scalar(10);
		petalPos[0+i][2]=Scalar(0);
		petalPos[8+i][0]=squareSize-squareSize/Scalar(10);
		petalPos[8+i][1]=-x;
		petalPos[8+i][2]=Scalar(0);
		petalPos[16+i][0]=-x;
		petalPos[16+i][1]=-squareSize+squareSize/Scalar(10);
		petalPos[16+i][2]=Scalar(0);
		petalPos[24+i][0]=-squareSize+squareSize/Scalar(10);
		petalPos[24+i][1]=x;
		petalPos[24+i][2]=Scalar(0);
		}
	
	/* Initialize the petal labels: */
	for(int i=0;i<32;++i)
		{
		petals[i].setString("",*getUiFont());
		petals[i].setBackground(GLLabel::Color(config.backgroundColor));
		petals[i].setForeground(GLLabel::Color(config.foregroundColor));
		}
	setAlphabet(LOWERCASE);
	}

const ToolFactory* QuikWriteTool::getFactory(void) const
	{
	return factory;
	}

void QuikWriteTool::buttonCallback(int,InputDevice::ButtonCallbackData* cbData)
	{
	if(cbData->newButtonState) // Button has just been pressed
		{
		if(!active)
			{
			/* Activate the tool: */
			active=true;
			
			/* Initialize the QuikWrite square transformation: */
			ray=calcInteractionRay();
			Point hotSpot;
			if(isUseEyeRay()||interactionDevice->isRayDevice())
				{
				/* Find the intersection point of the interaction ray and a screen: */
				std::pair<VRScreen*,Scalar> si=findScreen(ray);
				if(si.first!=0)
					hotSpot=ray(si.second);
				else
					hotSpot=getDisplayCenter();
				}
			else
				{
				/* Use a point in front of the input device: */
				hotSpot=ray(config.initialSquareDist);
				}
			squareTransform=calcHUDTransform(hotSpot);
			squarePlane=Plane(squareTransform.getDirection(2),squareTransform.getOrigin());
			
			/* Initialize QuikWrite's state: */
			haveLeftRest=false;
			setAlphabet(LOWERCASE);
			strokeState=REST;
			unconfirmed=false;
			}
		else
			{
			/* Pretend device has left rest zone to deactivate tool on button release: */
			haveLeftRest=true;
			}
		}
	else // Button has just been released
		{
		if(active&&haveLeftRest)
			{
			/* Check if a confirmation event needs to be sent: */
			if(unconfirmed)
				{
				/* Send a confirmation event to the widget manager: */
				getWidgetManager()->textControl(GLMotif::TextControlEvent(GLMotif::TextControlEvent::CONFIRM));
				}
			
			/* Deactivate the tool: */
			active=false;
			}
		}
	}

void QuikWriteTool::frame(void)
	{
	if(active)
		{
		/* Calculate the current interaction ray: */
		ray=calcInteractionRay();
		
		/* Get the index of the zone currently pointed at: */
		int zoneIndex=getZone(strokeState==REST);
		
		/* Behave based on the current state: */
		if(zoneIndex>=1)
			{
			switch(strokeState)
				{
				case REST:
					/* Check if the device left the rest zone: */
					if(zoneIndex!=5)
						{
						/* Initialize the major and minor zones: */
						strokeMajor=zoneIndex;
						strokeMinor=zoneIndex;
						
						/* Go to minor-selecting state: */
						strokeState=MINOR;
						haveLeftRest=true;
						}
					
					break;
					
				case MINOR:
					{
					bool switchAlphabetBack=true;
					
					/* Check if the device entered the rest zone: */
					if(zoneIndex==5)
						{
						/* Handle control characters: */
						if(strokeMajor==2)
							{
							if(strokeMinor==1)
								{
								/* Select the punctuation alphabet: */
								switchAlphabet(PUNCTUATION);
								switchAlphabetBack=false;
								}
							else if(strokeMinor==2)
								{
								/* Send a backspace event to the widget manager: */
								getWidgetManager()->textControl(GLMotif::TextControlEvent(GLMotif::TextControlEvent::BACKSPACE));
								unconfirmed=true;
								}
							else if(strokeMinor==3)
								{
								/* Select the uppercase alphabet: */
								switchAlphabet(UPPERCASE);
								switchAlphabetBack=false;
								}
							}
						else if(strokeMajor==8)
							{
							if(strokeMinor==7)
								{
								/* Send a confirmation event to the widget manager: */
								getWidgetManager()->textControl(GLMotif::TextControlEvent(GLMotif::TextControlEvent::CONFIRM));
								unconfirmed=false;
								}
							else if(strokeMinor==8)
								{
								/* Send a space character: */
								getWidgetManager()->text(GLMotif::TextEvent(" "));
								unconfirmed=true;
								}
							else if(strokeMinor==9)
								{
								/* Select the numeric alphabet: */
								switchAlphabet(NUMERIC);
								switchAlphabetBack=false;
								}
							}
						else
							{
							/* Get the selected character: */
							char character=characters[alphabet-LOWERCASE][strokeMajor-1][strokeMinor-1];
							if(character!=0)
								{
								/* Send the selected character: */
								char string[2];
								string[0]=character;
								string[1]='\0';
								getWidgetManager()->text(GLMotif::TextEvent(string));
								unconfirmed=true;
								}
							}
						
						/* Check whether to switch back to lowercase alphabet: */
						if(switchAlphabetBack&&!alphabetLocked)
							setAlphabet(LOWERCASE);
						
						/* Go back to the rest state: */
						strokeState=REST;
						}
					else
						{
						/* Only update the minor zone if the result would be valid: */
						if(zoneIndex!=strokeMajor&&characters[alphabet][strokeMajor-1][zoneIndex-1]!='\0')
							{
							/* Update the minor zone: */
							strokeMinor=zoneIndex;
							}
						}
					
					break;
					}
				}
			}
		}
	}

void QuikWriteTool::display(GLContextData& contextData) const
	{
	if(active)
		{
		/* Get the context data item: */
		DataItem* dataItem=contextData.retrieveDataItem<DataItem>(this);
		
		/* Save and set up OpenGL state: */
		if(config.drawPoint)
			glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT|GL_POINT_BIT);
		else
			glPushAttrib(GL_ENABLE_BIT|GL_LINE_BIT);
		glDisable(GL_LIGHTING);
		
		if(isDrawRay()||config.drawPoint)
			{
			/* Get the interaction ray's intersection with the square plane: */
			Plane::HitResult hr;
			hr=squarePlane.intersectRay(ray);
			if(hr.isValid())
				{
				if(isDrawRay())
					{
					/* Draw the interaction ray: */
					glLineWidth(getRayWidth());
					glBegin(GL_LINES);
					glColor(getRayColor());
					glVertex(ray.getOrigin());
					glVertex(ray(hr.getParameter()));
					glEnd();
					}
				
				if(config.drawPoint)
					{
					/* Draw the intersection point: */
					glPointSize(config.pointSize);
					glBegin(GL_POINTS);
					glColor(config.pointColor);
					glVertex(ray(hr.getParameter()));
					glEnd();
					}
				}
			}
		
		/* Go to square coordinates: */
		glPushMatrix();
		glMultMatrix(squareTransform);
		
		if(strokeState!=REST)
			{
			/* Highlight the major and minor regions: */
			glColor3f(1.0f,0.5f,0.5f);
			drawRegion(strokeMajor);
			if(strokeMinor!=strokeMajor)
				{
				glColor3f(0.5f,0.5f,1.0f);
				drawRegion(strokeMinor);
				}
			}
		
		/* Draw the square and special symbols: */
		glCallList(dataItem->squareListId);
		
		/* Install a deferred label renderer: */
		{
		GLLabel::DeferredRenderer dr(contextData);
		
		/* Draw the petals: */
		for(int i=0;i<3;++i)
			petals[i].draw(contextData);
		for(int i=6;i<19;++i)
			petals[i].draw(contextData);
		for(int i=22;i<32;++i)
			petals[i].draw(contextData);
		}
		
		/* Go back to physical coordinates and reset OpenGL state: */
		glPopMatrix();
		glPopAttrib();
		}
	}

void QuikWriteTool::initContext(GLContextData& contextData) const
	{
	/* Create and store a data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create the square display list: */
	glNewList(dataItem->squareListId,GL_COMPILE);
	
	/* Draw the square's background: */
	glLineWidth(3.0f);
	glColor(config.backgroundColor);
	drawSquare();
	
	/* Draw the square's foreground: */
	glLineWidth(1.0f);
	glColor(config.foregroundColor);
	drawSquare();
	
	glEndList();
	}

}
