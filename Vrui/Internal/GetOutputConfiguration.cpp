/***********************************************************************
GetOutputConfiguration - Helper function to find the physical size and
panning domain of an output connector or connected output device using
the XRANDR extension.
Copyright (c) 2014 Oliver Kreylos

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

#include <Vrui/Internal/GetOutputConfiguration.h>

#include <string.h>
#include <X11/Xlib.h>
#include <Vrui/Internal/Config.h>
#if VRUI_INTERNAL_CONFIG_HAVE_XRANDR
#include <X11/Xatom.h>
#include <X11/extensions/Xrandr.h>
#endif

namespace Vrui {

/*********
Functions:
*********/

OutputConfiguration getOutputConfiguration(Display* display,const char* outputName)
	{
	/* Create a default output configuration by assuming the entire root window goes to a single output: */
	OutputConfiguration result;
	result.sizeMm[0]=DisplayWidthMM(display,DefaultScreen(display));
	result.sizeMm[1]=DisplayHeightMM(display,DefaultScreen(display));
	result.domainOrigin[0]=0;
	result.domainOrigin[1]=0;
	result.domainSize[0]=DisplayWidth(display,DefaultScreen(display));;
	result.domainSize[1]=DisplayHeight(display,DefaultScreen(display));;
	
	#if VRUI_INTERNAL_CONFIG_HAVE_XRANDR
	
	/* Check if the X server on the other end of the display connection understands XRANDR version >= 1.2: */
	int xrandrEventBase,xrandrErrorBase;
	if(!XRRQueryExtension(display,&xrandrEventBase,&xrandrErrorBase))
		return result;
	int xrandrMajor,xrandrMinor;
	if(!XRRQueryVersion(display,&xrandrMajor,&xrandrMinor)||xrandrMajor<1||(xrandrMajor==1&&xrandrMinor<2))
		return result;
	
	/* Get the root screen's resources: */
	XRRScreenResources* screenResources=XRRGetScreenResources(display,RootWindow(display,DefaultScreen(display)));
	if(screenResources==0)
		return result;
	
	/* Find the first CRT controller that has an output of the given name: */
	bool firstOutput=true;
	bool haveMatch=false;
	for(int crtcIndex=0;crtcIndex<screenResources->ncrtc&&!haveMatch;++crtcIndex)
		{
		/* Get the CRT controller's information structure: */
		XRRCrtcInfo* crtcInfo=XRRGetCrtcInfo(display,screenResources,screenResources->crtcs[crtcIndex]);
		if(crtcInfo!=0)
			{
			/* Try all outputs driven by the CRT controller: */
			for(int outputIndex=0;outputIndex<crtcInfo->noutput&&!haveMatch;++outputIndex)
				{
				/* Get the output's information structure: */
				XRROutputInfo* outputInfo=XRRGetOutputInfo(display,screenResources,crtcInfo->outputs[outputIndex]);
				
				/* Check if this output either matches the search parameter, or is the default output: */
				haveMatch=strcmp(outputInfo->name,outputName)==0;
				if(!haveMatch)
					{
					/* Check if the output has an associated EDID property: */
					int numProperties;
					Atom* properties=XRRListOutputProperties(display,crtcInfo->outputs[outputIndex],&numProperties);
					for(int propertyIndex=0;propertyIndex<numProperties;++propertyIndex)
						{
						char* propertyName=XGetAtomName(display,properties[propertyIndex]);
						if(strcasecmp(propertyName,"EDID")==0)
							{
							Atom propertyType;
							int propertyFormat;
							unsigned long numItems;
							unsigned long bytes_after;
							unsigned char* propertyValue;
							XRRGetOutputProperty(display,crtcInfo->outputs[outputIndex],properties[propertyIndex],0,100,False,False,AnyPropertyType,&propertyType,&propertyFormat,&numItems,&bytes_after,&propertyValue);
							if(propertyType==XA_INTEGER&&propertyFormat==8)
								{
								/* Check the EDID's checksum and header ID: */
								unsigned char checksum=0;
								for(unsigned long i=0;i<numItems;++i)
									checksum+=propertyValue[i];
								unsigned char edidHeaderId[8]={0x00U,0xffU,0xffU,0xffU,0xffU,0xffU,0xffU,0x00U};
								bool headerOk=true;
								for(int i=0;i<8&&headerOk;++i)
									headerOk=propertyValue[i]==edidHeaderId[i];
								if(checksum==0&&headerOk)
									{
									/* Find the monitor name among the extension blocks: */
									unsigned char* blockPtr=propertyValue+0x36;
									for(int i=0;i<4&&!haveMatch;++i,blockPtr+=18)
										if(blockPtr[0]==0x00U&&blockPtr[1]==0x00U&&blockPtr[2]==0x00U&&blockPtr[3]==0xfcU)
											{
											/* Extract the monitor name: */
											char monitorName[14];
											char* mnPtr=monitorName;
											for(unsigned char* namePtr=blockPtr+5;namePtr<blockPtr+18&&*namePtr!='\n';++namePtr,++mnPtr)
												*mnPtr=char(*namePtr);
											*mnPtr='\0';
											haveMatch=strcmp(monitorName,outputName)==0;
											}
									}
								}
							XFree(propertyValue);
							}
						XFree(propertyName);
						}
					XFree(properties);
					}
				
				if(firstOutput||haveMatch)
					{
					/* Remember the output's configuration: */
					result.sizeMm[0]=outputInfo->mm_width;
					result.sizeMm[1]=outputInfo->mm_height;
					result.domainOrigin[0]=crtcInfo->x;
					result.domainOrigin[1]=crtcInfo->y;
					result.domainSize[0]=crtcInfo->width;
					result.domainSize[1]=crtcInfo->height;
					
					firstOutput=false;
					}
				
				XRRFreeOutputInfo(outputInfo);
				}
			
			XRRFreeCrtcInfo(crtcInfo);
			}
		}
	
	XRRFreeScreenResources(screenResources);
	
	#endif
	
	return result;
	}

}
