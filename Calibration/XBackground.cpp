#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <X11/keysym.h>
#include <X11/Xlib.h>

/**************************************************
Helper function to load an RGB image in PPM format:
**************************************************/

unsigned char* loadPPMFile(const char* ppmFileName,int ppmSize[2])
	{
	/* Open PPM file: */
	FILE* ppmFile=fopen(ppmFileName,"rb");
	if(ppmFile==0)
		{
		std::ostringstream str;
		str<<"loadPPMFile: Could not open input file "<<ppmFileName;
		throw std::runtime_error(str.str());
		}
	
	/* Parse PPM file header: */
	char line[256];
	if(fgets(line,sizeof(line),ppmFile)==0||strcmp(line,"P6\n")!=0)
		{
		fclose(ppmFile);
		std::ostringstream str;
		str<<"loadPPMFile: Input file "<<ppmFileName<<" is not a binary RGB PPM file";
		throw std::runtime_error(str.str());
		}
	
	/* Skip all comment lines: */
	do
		{
		if(fgets(line,sizeof(line),ppmFile)==0)
			break;
		}
	while(line[0]=='#');
	
	/* Read image size: */
	if(sscanf(line,"%d %d",&ppmSize[0],&ppmSize[1])!=2)
		{
		fclose(ppmFile);
		std::ostringstream str;
		str<<"loadPPMFile: Input file "<<ppmFileName<<" has a malformed PPM header";
		throw std::runtime_error(str.str());
		}
	
	/* Read (and ignore) maxvalue: */
	if(fgets(line,sizeof(line),ppmFile)==0)
		{
		fclose(ppmFile);
		std::ostringstream str;
		str<<"loadPPMFile: Input file "<<ppmFileName<<" has a malformed PPM header";
		throw std::runtime_error(str.str());
		}
	
	/* Read image data: */
	unsigned char* result=new unsigned char[ppmSize[1]*ppmSize[0]*3];
	#if 1
	if(fread(result,sizeof(unsigned char)*3,ppmSize[1]*ppmSize[0],ppmFile)!=size_t(ppmSize[1]*ppmSize[0]))
		{
		fclose(ppmFile);
		delete[] result;
		std::ostringstream str;
		str<<"loadPPMFile: Error while reading from input file "<<ppmFileName;
		throw std::runtime_error(str.str());
		}
	#else
	for(int y=ppmSize[1]-1;y>=0;--y)
		if(fread(&result[y*ppmSize[0]*3],sizeof(unsigned char)*3,ppmSize[0],ppmFile)!=ppmSize[0])
			{
			fclose(ppmFile);
			delete[] result;
			std::ostringstream str;
			str<<"loadPPMFile: Error while reading from input file "<<ppmFileName;
			throw std::runtime_error(str.str());
			}
	#endif
	
	fclose(ppmFile);
	return result;
	}

/*************************************
Helper class to represent X11 windows:
*************************************/

struct WindowState
	{
	/* Elements: */
	public:
	Display* display; // The display connection
	Window window; // X11 window handle
	int origin[2]; // Window origin in pixels
	int size[2]; // Window width and height in pixels
	GC gc; // Graphics context for the window
	XImage* image; // Image to display in the window (or 0 if no image was given)
	
	/* Constructors and destructors: */
	public:
	WindowState(void)
		:display(0),
		 image(0)
		{
		origin[0]=origin[1]=0;
		size[0]=size[1]=128;
		}
	~WindowState(void)
		{
		if(image!=0)
			{
			delete[] (int*)image->data;
			delete image;
			}
		XFreeGC(display,gc);
		XDestroyWindow(display,window);
		}
	
	/* Methods: */
	void init(Display* sDisplay,int screen,bool makeFullscreen)
		{
		/* Store the display connection: */
		display=sDisplay;
		
		/* Get root window of this screen: */
		Window root=RootWindow(display,screen);
		
		/* Get root window's size: */
		XWindowAttributes rootAttr;
		XGetWindowAttributes(display,root,&rootAttr);
		
		/* Create the new window: */
		// size[0]=rootAttr.width;
		// size[1]=rootAttr.height;
		window=XCreateSimpleWindow(display,root,origin[0],origin[1],size[0],size[1],0,WhitePixel(display,screen),BlackPixel(display,screen));
		XSelectInput(display,window,ExposureMask|StructureNotifyMask|KeyPressMask);
		XMapRaised(display,window);
		
		if(makeFullscreen)
			{
			/* Get relevant window manager protocol atoms: */
			Atom netwmStateAtom=XInternAtom(display,"_NET_WM_STATE",True);
			Atom netwmStateFullscreenAtom=XInternAtom(display,"_NET_WM_STATE_FULLSCREEN",True);
			if(netwmStateAtom!=None&&netwmStateFullscreenAtom!=None)
				{
				/* Ask the window manager to make this window fullscreen: */
				XEvent fullscreenEvent;
				fullscreenEvent.xclient.type=ClientMessage;
				fullscreenEvent.xclient.serial=0;
				fullscreenEvent.xclient.send_event=True;
				fullscreenEvent.xclient.display=display;
				fullscreenEvent.xclient.window=window;
				fullscreenEvent.xclient.message_type=netwmStateAtom;
				fullscreenEvent.xclient.format=32;
				fullscreenEvent.xclient.data.l[0]=1; // Should be _NET_WM_STATE_ADD, but that doesn't work for some reason
				fullscreenEvent.xclient.data.l[1]=netwmStateFullscreenAtom;
				fullscreenEvent.xclient.data.l[2]=0;
				XSendEvent(display,RootWindow(display,screen),False,SubstructureRedirectMask|SubstructureNotifyMask,&fullscreenEvent);
				XFlush(display);
				}
			else
				{
				/*******************************************************************
				Use hacky method of adjusting window size just beyond the root
				window.
				*******************************************************************/
				
				/* Query the window's geometry to calculate its offset inside its parent window (the window manager decoration): */
				Window win_root;
				int win_x,win_y;
				unsigned int win_width,win_height,win_borderWidth,win_depth;
				XGetGeometry(display,window,&win_root,&win_x,&win_y,&win_width,&win_height,&win_borderWidth,&win_depth);
				
				/* Set the window's position and size such that the window manager decoration falls outside the root window: */
				XMoveResizeWindow(display,window,-win_x,-win_y,DisplayWidth(display,screen),DisplayHeight(display,screen));
				}
			}
		
		/* Raise the window to the top of the stacking hierarchy: */
		XRaiseWindow(display,window);
		
		/* Hide the mouse cursor: */
		static char emptyCursorBits[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		                               0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		Pixmap emptyCursorPixmap=XCreatePixmapFromBitmapData(display,window,emptyCursorBits,16,16,1,0,1);
		XColor black,white; // Actually, both are dummy colors
		Cursor emptyCursor=XCreatePixmapCursor(display,emptyCursorPixmap,emptyCursorPixmap,&black,&white,0,0);
		XDefineCursor(display,window,emptyCursor);
		XFreeCursor(display,emptyCursor);
		XFreePixmap(display,emptyCursorPixmap);
		
		/* Create a graphics context for the window: */
		gc=XCreateGC(display,window,0x0,0);
		XSetBackground(display,gc,BlackPixel(display,screen));
		XSetForeground(display,gc,WhitePixel(display,screen));
		}
	void loadImage(const char* ppmFileName,const char* components)
		{
		/* Parse components string: */
		bool useRed=false;
		bool useGreen=false;
		bool useBlue=false;
		for(const char* compPtr=components;*compPtr!='\0';++compPtr)
			switch(toupper(*compPtr))
				{
				case 'R':
					useRed=true;
					break;
				
				case 'G':
					useGreen=true;
					break;
				
				case 'B':
					useBlue=true;
					break;
				}
		
		/* Get window's attributes: */
		XWindowAttributes windowAttr;
		XGetWindowAttributes(display,window,&windowAttr);
		
		/* Allocate an image data buffer: */
		int bitsPerPixel=32;
		int bytesPerLine=((bitsPerPixel*windowAttr.width+31)/32)*4;
		int colorMask[3];
		colorMask[0]=windowAttr.visual->red_mask;
		colorMask[1]=windowAttr.visual->green_mask;
		colorMask[2]=windowAttr.visual->blue_mask;
		int colorShift[3],colorScale[3];
		for(int i=0;i<3;++i)
			{
			for(colorShift[i]=0;(colorMask[i]&0x1)==0x0;++colorShift[i],colorMask[i]>>=1)
				;
			colorScale[i]=colorMask[i];
			}
		int* imageData=new int[windowAttr.width*windowAttr.height];
		
		/* Read the image file: */
		int ppmSize[2];
		unsigned char* ppmData=loadPPMFile(ppmFileName,ppmSize);
		
		/* Initialize the image data: */
		int* imgPtr=imageData;
		const unsigned char* ppmPtr=ppmData;
		for(int y=0;y<windowAttr.height;++y)
			{
			for(int x=0;x<windowAttr.width;++x,++imgPtr)
				{
				if(x<ppmSize[0]&&y<ppmSize[1])
					{
					float col[3]={0.0f,0.0f,0.0f};
					if(useRed)
						col[0]=float(ppmPtr[0])/255.0f;
					if(useGreen)
						col[1]=float(ppmPtr[1])/255.0f;
					if(useBlue)
						col[2]=float(ppmPtr[2])/255.0f;
					int pixel=0x0;
					for(int i=0;i<3;++i)
						pixel|=int(floorf(col[i]*float(colorScale[i])+0.5f))<<colorShift[i];
					*imgPtr=pixel;
					ppmPtr+=3;
					}
				}
			}
		
		/* Delete the image file data: */
		delete[] ppmData;
		
		/* Create an appropriate XImage structure: */
		image=new XImage;
		image->width=windowAttr.width;
		image->height=windowAttr.height;
		image->xoffset=0;
		image->format=ZPixmap;
		image->data=(char*)imageData;
		image->byte_order=ImageByteOrder(display);
		image->bitmap_unit=BitmapUnit(display);
		image->bitmap_bit_order=BitmapBitOrder(display);
		image->bitmap_pad=BitmapPad(display);
		image->depth=windowAttr.depth;
		image->bytes_per_line=bytesPerLine;
		image->bits_per_pixel=bitsPerPixel;
		image->red_mask=windowAttr.visual->red_mask;
		image->green_mask=windowAttr.visual->green_mask;
		image->blue_mask=windowAttr.visual->blue_mask;
		XInitImage(image);
		}
	};

void redraw(const WindowState& ws,int winOriginX,int winOriginY,int winWidth,int winHeight,int imageType,int squareSize)
	{
	if(ws.image!=0)
		{
		/* Draw the image: */
		XPutImage(ws.display,ws.window,ws.gc,ws.image,0,0,winOriginX,winOriginY,winWidth,winHeight);
		}
	else
		{
		switch(imageType)
			{
			case 0: // Calibration grid
				{
				/* Draw a set of vertical lines: */
				for(int hl=0;hl<=20;++hl)
					{
					int x=int(floor(double(hl)*double(winWidth-1)/20.0+0.5))+winOriginX;
					XDrawLine(ws.display,ws.window,ws.gc,x,winOriginY,x,winOriginY+winHeight-1);
					}
				
				/* Draw a set of horizontal lines: */
				for(int vl=0;vl<=16;++vl)
					{
					int y=int(floor(double(vl)*double(winHeight-1)/16.0+0.5))+winOriginY;
					XDrawLine(ws.display,ws.window,ws.gc,winOriginX,y,winOriginX+winWidth-1,y);
					}
				
				/* Draw some circles: */
				int r=winHeight/2;
				XDrawArc(ws.display,ws.window,ws.gc,winOriginX+winWidth/2-r,winOriginY+winHeight/2-r,r*2,r*2,0,360*64);
				r=(winHeight*2)/15;
				XDrawArc(ws.display,ws.window,ws.gc,winOriginX,winOriginY,r*2,r*2,0,360*64);
				XDrawArc(ws.display,ws.window,ws.gc,winOriginX+winWidth-1-r*2,winOriginY,r*2,r*2,0,360*64);
				XDrawArc(ws.display,ws.window,ws.gc,winOriginX+winWidth-1-r*2,winOriginY+winHeight-1-r*2,r*2,r*2,0,360*64);
				XDrawArc(ws.display,ws.window,ws.gc,winOriginX,winOriginY+winHeight-1-r*2,r*2,r*2,0,360*64);
				
				/* Draw a fence of vertical lines to check pixel tracking: */
				int fenceYMin=winOriginY+winHeight/2-winHeight/20;
				int fenceYMax=winOriginY+winHeight/2+winHeight/20;
				XSetForeground(ws.display,ws.gc,0xffffff);
				for(int x=0;x<winWidth;x+=2)
					XDrawLine(ws.display,ws.window,ws.gc,x,fenceYMin,x,fenceYMax);
				XSetForeground(ws.display,ws.gc,0x000000);
				for(int x=1;x<winWidth;x+=2)
					XDrawLine(ws.display,ws.window,ws.gc,x,fenceYMin,x,fenceYMax);
				break;
				}
			
			case 1: // Pixel tracking test
				/* Draw a set of vertical lines: */
				for(int x=winOriginX;x<winOriginX+winWidth;x+=2)
					XDrawLine(ws.display,ws.window,ws.gc,x,winOriginY,x,winOriginY+winHeight-1);
				break;
			
			case 2: // Calibration grid for TotalStation
				{
				/* Draw a set of vertical lines: */
				int offsetX=((winWidth-1)%squareSize)/2;
				for(int x=winOriginX+offsetX;x<winOriginX+winWidth;x+=squareSize)
					XDrawLine(ws.display,ws.window,ws.gc,x,winOriginY,x,winOriginY+winHeight-1);
				
				/* Draw a set of horizontal lines: */
				int offsetY=((winHeight-1)%squareSize)/2;
				for(int y=winOriginY+offsetY;y<winOriginY+winHeight;y+=squareSize)
					XDrawLine(ws.display,ws.window,ws.gc,winOriginX,y,winOriginX+winWidth-1,y);
				
				break;
				}
			
			case 3: // Checkerboard for camera calibration
				{
				/* Determine the offset for the top-left square: */
				int offsetX=((winWidth-1)%squareSize)/2;
				int offsetY=((winHeight-1)%squareSize)/2;
				
				/* Fill the window white: */
				XSetForeground(ws.display,ws.gc,0xffffff);
				XFillRectangle(ws.display,ws.window,ws.gc,winOriginX,winOriginY,winWidth,winHeight);
				
				/* Draw a checkerboard of black squares: */
				XSetForeground(ws.display,ws.gc,0x000000);
				for(int y=offsetY;y+squareSize<winHeight;y+=squareSize)
					for(int x=offsetX;x+squareSize<winWidth;x+=squareSize)
						if(((x-offsetX)/squareSize+(y-offsetY)/squareSize)%2==0)
							{
							XFillRectangle(ws.display,ws.window,ws.gc,x,y,squareSize,squareSize);
							}
				
				break;
				}
			
			case 4: // Blank screen
				break;
			}
		}
	}

int main(int argc,char* argv[])
	{
	/* Parse command line: */
	char* displayName=getenv("DISPLAY");
	// char* displayName=":0.0";
	int origin[2]={0,0};
	int size[2]={128,128};
	bool makeFullscreen=true;
	int imageType=0;
	int squareSize=300;
	char* imgFileName=0;
	const char* components="rgb";
	bool splitStereo=false;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"display")==0)
				{
				++i;
				displayName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"geometry")==0)
				{
				++i;
				
				/* Parse an X geometry string: */
				int geomIndex=0;
				int geometry[4];
				for(int j=0;j<2;++j)
					{
					geometry[j]=size[j];
					geometry[2+j]=origin[j];
					}
				const char* gPtr=argv[i];
				bool correct=true;
				while(*gPtr!='\0'&&geomIndex<4&&correct)
					{
					if(*gPtr=='x')
						{
						if(geomIndex==0)
							geomIndex=1;
						else
							correct=false;
						++gPtr;
						}
					else if(*gPtr=='+')
						{
						if(geomIndex<2)
							geomIndex=2;
						else if(geomIndex==2)
							geomIndex=3;
						else
							correct=false;
						++gPtr;
						}
					else if(*gPtr>='0'&&*gPtr<='9')
						{
						geometry[geomIndex]=0;
						do
							{
							geometry[geomIndex]=geometry[geomIndex]*10+int(*gPtr-'0');
							++gPtr;
							}
						while(*gPtr>='0'&&*gPtr<='9');
						}
					else
						correct=false;
					}
				
				if(correct)
					{
					for(int j=0;j<2;++j)
						{
						size[j]=geometry[j];
						origin[j]=geometry[2+j];
						}
					}
				else
					fprintf(stderr,"Ignoring invalid geometry string %s\n",argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"noFullscreen")==0)
				makeFullscreen=false;
			else if(strcasecmp(argv[i]+1,"type")==0)
				{
				++i;
				imageType=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"size")==0)
				{
				++i;
				squareSize=atoi(argv[i]);
				}
			else if(strcasecmp(argv[i]+1,"stereo")==0)
				splitStereo=true;
			}
		else if(imgFileName==0)
			imgFileName=argv[i];
		else
			components=argv[i];
		}
	
	/* Open a connection to the X server: */
	Display* display=XOpenDisplay(displayName);
	if(display==0)
		return 1;
	
	/* Check if the display name contains a screen name: */
	bool haveColon=false;
	const char* periodPtr=0;
	for(const char* dnPtr=displayName;*dnPtr!='\0';++dnPtr)
		{
		if(*dnPtr==':')
			haveColon=true;
		else if(haveColon&&*dnPtr=='.')
			periodPtr=dnPtr;
		}
	
	int numWindows;
	WindowState* ws=0;
	if(periodPtr!=0)
		{
		/* Create a window for the given screen: */
		numWindows=1;
		ws=new WindowState[numWindows];
		for(int j=0;j<2;++j)
			{
			ws[0].origin[j]=origin[j];
			ws[0].size[j]=size[j];
			}
		int screen=atoi(periodPtr+1);
		ws[0].init(display,screen,makeFullscreen);
		
		/* Load an image, if given: */
		if(imgFileName!=0&&strcasecmp(imgFileName,"Grid")!=0)
			ws[0].loadImage(imgFileName,components);
		}
	else
		{
		/* Create a window for each screen: */
		numWindows=ScreenCount(display);
		ws=new WindowState[numWindows];
		for(int screen=0;screen<numWindows;++screen)
			{
			for(int j=0;j<2;++j)
				{
				ws[screen].origin[j]=origin[j];
				ws[screen].size[j]=size[j];
				}
			ws[screen].init(display,screen,makeFullscreen);
			
			/* Load an image, if given: */
			if(imgFileName!=0&&strcasecmp(imgFileName,"Grid")!=0)
				ws[screen].loadImage(imgFileName,components);
			}
		}
	
	unsigned long stereoColors[2]={0x00df00,0xff20ff};
	
	/* Process X events: */
	bool goOn=true;
	while(goOn)
		{
		XEvent event;
		XNextEvent(display,&event);
		
		/* Find the target window of this event: */
		int i;
		for(i=0;i<numWindows&&event.xany.window!=ws[i].window;++i)
			;
		if(i<numWindows)
			switch(event.type)
				{
				case ConfigureNotify:
					ws[i].size[0]=event.xconfigure.width;
					ws[i].size[1]=event.xconfigure.height;
					break;
				
				case KeyPress:
					{
					XKeyEvent keyEvent=event.xkey;
					KeySym keySym=XLookupKeysym(&keyEvent,0);
					goOn=keySym!=XK_Escape;
					break;
					}
				
				case Expose:
					if(splitStereo)
						{
						/* Render test pattern for double-wide split-stereo screen: */
						XSetForeground(ws[i].display,ws[i].gc,stereoColors[0]);
						redraw(ws[i],0,0,ws[i].size[0]/2,ws[i].size[1],imageType,squareSize);
						XSetForeground(ws[i].display,ws[i].gc,stereoColors[1]);
						redraw(ws[i],ws[i].size[0]/2,0,ws[i].size[0]/2,ws[i].size[1],imageType,squareSize);
						}
					else
						{
						/* Render test pattern for regular-size screen: */
						XSetForeground(ws[i].display,ws[i].gc,0xffffff);
						redraw(ws[i],0,0,ws[i].size[0],ws[i].size[1],imageType,squareSize);
						}
					break;
				}
		}
	
	/* Clean up: */
	delete[] ws;
	XCloseDisplay(display);
	return 0;
	}
