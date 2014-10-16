/***********************************************************************
WidgetManager - Class to manage top-level GLMotif UI components and user
events.
Copyright (c) 2001-2014 Oliver Kreylos

This file is part of the GLMotif Widget Library (GLMotif).

The GLMotif Widget Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GLMotif Widget Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the GLMotif Widget Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef GLMOTIF_WIDGETMANAGER_INCLUDED
#define GLMOTIF_WIDGETMANAGER_INCLUDED

#include <vector>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>
#include <Misc/HashTable.h>
#include <Misc/ThrowStdErr.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/Types.h>
#include <GLMotif/WidgetAttribute.h>

/* Forward declarations: */
namespace Misc {
class TimerEventScheduler;
}
class GLContextData;
namespace GLMotif {
class Event;
class TextEvent;
class TextControlEvent;
struct StyleSheet;
class Widget;
}

namespace GLMotif {

class WidgetManager
	{
	/* Embedded classes: */
	public:
	typedef Geometry::OrthogonalTransformation<Scalar,3> Transformation;
	
	struct CallbackData:public Misc::CallbackData // Base class for widget manager events
		{
		/* Elements: */
		public:
		WidgetManager* widgetManager; // Pointer to the widget manager that caused the event
		
		/* Constructors and destructors: */
		CallbackData(WidgetManager* sWidgetManager)
			:widgetManager(sWidgetManager)
			{
			}
		};
	
	struct WidgetPopCallbackData:public CallbackData // Class for widget pop-up or pop-down events
		{
		/* Elements: */
		public:
		bool popup; // Flag if the event was a pop-up event
		Widget* topLevelWidget; // The popped-up or popped-down widget
		bool primary; // Flag if the widget was popped up as a primary widget
		
		/* Constructors and destructors: */
		WidgetPopCallbackData(WidgetManager* sWidgetManager,bool sPopup,Widget* sTopLevelWidget,bool sPrimary)
			:CallbackData(widgetManager),popup(sPopup),topLevelWidget(sTopLevelWidget),primary(sPrimary)
			{
			}
		};
	
	struct WidgetMoveCallbackData:public CallbackData // Class for widget movement events
		{
		/* Elements: */
		public:
		Transformation widgetToWorld; // The widget's new transformation
		Widget* topLevelWidget; // The moved top-level widget
		bool primary; // Flag if the widget is a primary widget
		
		/* Constructors and destructors: */
		WidgetMoveCallbackData(WidgetManager* sWidgetManager,const Transformation& sWidgetToWorld,Widget* sTopLevelWidget,bool sPrimary)
			:CallbackData(widgetManager),widgetToWorld(sWidgetToWorld),topLevelWidget(sTopLevelWidget),primary(sPrimary)
			{
			}
		};
	
	private:
	struct PopupBinding // Structure to bind top level widgets
		{
		/* Elements: */
		public:
		Widget* topLevelWidget; // Pointer to top level widget
		Transformation widgetToWorld; // Transformation from widget to world coordinates or owner widget's coordinates
		bool visible; // Flag if top level widget should be drawn
		PopupBinding* parent; // Pointer to the binding this binding is secondary to
		PopupBinding* pred; // Pointer to previous binding in same hierarchy level
		PopupBinding* succ; // Pointer to next binding in same hierarchy level
		PopupBinding* firstSecondary; // Pointer to first secondary top level window
		
		/* Constructors and destructors: */
		PopupBinding(Widget* sTopLevelWidget,const Transformation& sWidgetToWorld,PopupBinding* sParent,PopupBinding* sSucc);
		~PopupBinding(void);
		
		/* Methods: */
		const PopupBinding* getSucc(void) const; // Get the successor in a DFS-traversal of bindings
		PopupBinding* getSucc(void); // Ditto
		PopupBinding* findTopLevelWidget(const Point& point);
		PopupBinding* findTopLevelWidget(const Ray& ray);
		void draw(bool overlayWidgets,GLContextData& contextData) const;
		};
	
	typedef Misc::HashTable<const Widget*,PopupBinding*> PopupBindingMap; // Type to map top-level widgets to their popup bindings
	
	public:
	class PoppedWidgetIterator // Class to iterate through popped-up widgets
		{
		friend class WidgetManager;
		
		/* Elements: */
		private:
		PopupBinding* bPtr; // Pointer to the widget's popup binding
		
		/* Constructors and destructors: */
		public:
		PoppedWidgetIterator(void) // Creates an invalid iterator
			:bPtr(0)
			{
			}
		private:
		PoppedWidgetIterator(PopupBinding* sBPtr) // Creates an iterator from a popup binding
			:bPtr(sBPtr)
			{
			}
		
		/* Methods: */
		public:
		friend bool operator==(const PoppedWidgetIterator& it1,const PoppedWidgetIterator& it2)
			{
			return it1.bPtr==it2.bPtr;
			}
		friend bool operator!=(const PoppedWidgetIterator& it1,const PoppedWidgetIterator& it2)
			{
			return it1.bPtr!=it2.bPtr;
			}
		Widget* operator*(void) const // Returns the bound top-level widget
			{
			return bPtr->topLevelWidget;
			}
		bool isPrimary(void) const // Returns true if the top-level widget is popped up as a primary widget
			{
			return bPtr->parent==0;
			}
		bool isVisible(void) const // Returns true if the top-level widget is visible
			{
			return bPtr->visible;
			}
		const Transformation& getWidgetToWorld(void) const // Returns the top-level widget's transformation relative to the world or its parent
			{
			return bPtr->widgetToWorld;
			}
		void setWidgetToWorld(const Transformation& newWidgetToWorld) // Sets the top-level widget's transformation
			{
			bPtr->widgetToWorld=newWidgetToWorld;
			}
		PoppedWidgetIterator beginSecondaryWidgets(void) const // Returns iterator to first secondary widget
			{
			return PoppedWidgetIterator(bPtr->firstSecondary);
			}
		PoppedWidgetIterator endSecondaryWidgets(void) const // Returns iterator after last secondary widget
			{
			return PoppedWidgetIterator(0);
			}
		PoppedWidgetIterator& operator--(void) // Decrements iterator
			{
			if(bPtr!=0)
				bPtr=bPtr->pred;
			return *this;
			}
		PoppedWidgetIterator& operator++(void) // Increments iterator
			{
			if(bPtr!=0)
				bPtr=bPtr->succ;
			return *this;
			}
		};
	
	class EventProcessingLocker // Helper class to safely signal ongoing event processing to the rest of the manager
		{
		/* Elements: */
		private:
		WidgetManager* manager; // Pointer to locked manager
		
		/* Constructors and destructors: */
		public:
		EventProcessingLocker(WidgetManager* sManager) // Starts event processing
			:manager(sManager)
			{
			manager->inEventProcessing=true;
			}
		~EventProcessingLocker(void) // Stops event processing
			{
			manager->inEventProcessing=false;
			if(!manager->deletionList.empty())
				manager->deleteQueuedWidgets();
			}
		};
	
	private:
	typedef Misc::HashTable<const Widget*,WidgetAttributeBase*> WidgetAttributeMap; // Type for hash tables mapping widgets to widget attributes
	
	/* Elements: */
	private:
	const StyleSheet* styleSheet; // The widget manager's style sheet
	Misc::TimerEventScheduler* timerEventScheduler; // Pointer to a scheduler for timer events managed by the OS/window system binding layer
	bool drawOverlayWidgets; // Flag whether widgets are drawn in an overlay layer on top of all other 3D imagery
	WidgetAttributeMap widgetAttributeMap; // Map from widgets to widget attributes
	PopupBinding* firstBinding; // Pointer to first bound top level widget
	PopupBindingMap popupBindingMap; // Map from currently popped-up top-level widgets to their popup bindings
	double time; // The time reported to widgets
	bool hardGrab; // Flag if the current pointer grab is a hard one
	Widget* pointerGrabWidget; // Pointer to the widget grabbing the input
	Transformation pointerGrabWidgetToWorld; // Transformation from the grabbing widget's coordinates to world coordinates
	Widget* textFocusWidget; // Pointer to the widget currently holding the text entry focus
	bool inEventProcessing; // Flag if the widget manager is currently processing widget events
	std::vector<Widget*> deletionList; // List of widgets to be deleted at the next opportunity
	int textBufferLength; // Length of current cut & paste buffer
	char* textBuffer; // Current cut & paste buffer
	Misc::CallbackList widgetPopCallbacks; // List of callbacks to be called when a primary or secondary widget is popped up or down
	Misc::CallbackList widgetMoveCallbacks; // List of callbacks to be called when a primary or secondary widget is moved
	
	/* Private methods: */
	const PopupBinding* getRootBinding(const Widget* widget) const; // Returns the binding for a widget's root, or null if the widget's root is not bound
	PopupBinding* getRootBinding(Widget* widget); // Ditto
	void moveSecondaryWidgets(PopupBinding* parent,const Transformation& parentTransform); // Calls move callbacks for all secondary widgets belonging to the given parent
	void deleteWidgetImmediately(Widget* widget); // Immediately deletes the given widget and removes and locks or holds
	void deleteQueuedWidgets(void); // Deletes all widgets in the deletion list
	
	/* Constructors and destructors: */
	public:
	WidgetManager(void); // Constructs an empty widget manager
	~WidgetManager(void);
	
	/* Methods: */
	void setStyleSheet(const StyleSheet* newStyleSheet); // Sets the widget manager's style sheet
	const StyleSheet* getStyleSheet(void) const // Returns the widget manager's style sheet
		{
		return styleSheet;
		}
	void setTimerEventScheduler(Misc::TimerEventScheduler* newTimerEventScheduler); // Sets the widget manager's timer event scheduler
	const Misc::TimerEventScheduler* getTimerEventScheduler(void) const // Returns a pointer to the timer event scheduler
		{
		return timerEventScheduler;
		}
	Misc::TimerEventScheduler* getTimerEventScheduler(void) // Ditto
		{
		return timerEventScheduler;
		}
	void setDrawOverlayWidgets(bool newDrawOverlayWidgets); // Sets whether widgets are drawn in an overlay layer
	bool getDrawOverlayWidgets(void) const // Returns the current setting of the overlay flag
		{
		return drawOverlayWidgets;
		}
	void unmanageWidget(Widget* widget); // Tells the widget manager that the given widget is about to be destroyed; only called from Widget's destructor
	template <class AttributeParam>
	void setWidgetAttribute(const Widget* widget,const AttributeParam& attribute) // Associates an attribute of arbitrary type with a widget; deletes previous attribute
		{
		WidgetAttributeMap::Iterator waIt=widgetAttributeMap.findEntry(widget);
		if(waIt.isFinished())
			{
			/* Set a new attribute: */
			widgetAttributeMap.setEntry(WidgetAttributeMap::Entry(widget,new WidgetAttribute<AttributeParam>(attribute)));
			}
		else
			{
			/* Delete the previous attribute and store a new one: */
			delete waIt->getDest();
			waIt->getDest()=new WidgetAttribute<AttributeParam>(attribute);
			}
		}
	template <class AttributeParam>
	const AttributeParam& getWidgetAttribute(const Widget* widget) const // Returns a widget attribute of arbitrary type
		{
		WidgetAttributeMap::ConstIterator waIt=widgetAttributeMap.findEntry(widget);
		if(waIt.isFinished())
			Misc::throwStdErr("GLMotif::WidgetManager::getWidgetAttribute: No attribute for widget %p",widget);
		const WidgetAttribute<AttributeParam>* wa=dynamic_cast<const WidgetAttribute<AttributeParam>*>(waIt->getDest());
		if(wa==0)
			Misc::throwStdErr("GLMotif::WidgetManager::getWidgetAttribute: Attribute for widget %p is of wrong type",widget);
		return wa->getValue();
		}
	template <class AttributeParam>
	AttributeParam& getWidgetAttribute(const Widget* widget) // Ditto
		{
		WidgetAttributeMap::Iterator waIt=widgetAttributeMap.findEntry(widget);
		if(waIt.isFinished())
			Misc::throwStdErr("GLMotif::WidgetManager::getWidgetAttribute: No attribute for widget %p",widget);
		WidgetAttribute<AttributeParam>* wa=dynamic_cast<WidgetAttribute<AttributeParam>*>(waIt->getDest());
		if(wa==0)
			Misc::throwStdErr("GLMotif::WidgetManager::getWidgetAttribute: Attribute for widget %p is of wrong type",widget);
		return wa->getValue();
		}
	void popupPrimaryWidget(Widget* topLevelWidget,const Transformation& widgetToWorld); // Pops up a primary top level widget
	void popupSecondaryWidget(Widget* owner,Widget* topLevelWidget,const Vector& offset); // Pops up a secondary top level widget
	void popdownWidget(Widget* widget); // Pops down the top level widget containing the given widget
	PoppedWidgetIterator beginPrimaryWidgets(void) // Returns iterator to first primary widget
		{
		return PoppedWidgetIterator(firstBinding);
		}
	PoppedWidgetIterator endPrimaryWidgets(void) // Returns iterator after last primary widget
		{
		return PoppedWidgetIterator(0);
		}
	void show(Widget* widget); // Shows the top level widget containing the given widget
	void hide(Widget* widget); // Hides the top level widget containing the given widget
	bool isManaged(const Widget* widget) const; // Returns true if the top level widget containing the given widget is popped up
	bool isVisible(const Widget* widget) const; // Returns true if the top level widget containing the given widget is popped up and visible
	Widget* findPrimaryWidget(const Point& point); // Finds the primary top level widget whose descendants contain the given point
	Widget* findPrimaryWidget(const Ray& ray); // Finds the primary top level widget whose descendants are intersected by the given ray
	Transformation calcWidgetTransformation(const Widget* widget) const; // Returns the transformation associated with a widget's root
	void setPrimaryWidgetTransformation(Widget* widget,const Transformation& newWidgetToWorld); // Sets the transformation of a primary top level widget
	void deleteWidget(Widget* widget); // Method to delete a widget that is safe to call from within a callback belonging to the widget
	void setTime(double newTime); // Sets the widget manager's time
	double getTime(void) const // Returns the current time
		{
		return time;
		}
	void draw(GLContextData& contextData) const;
	bool pointerButtonDown(Event& event); // Handles a button down event
	bool pointerButtonUp(Event& event); // Handles a button up event
	bool pointerMotion(Event& event); // Handles a pointer motion event
	void grabPointer(Widget* widget); // Allows a widget to grab all pointer events
	void releasePointer(Widget* widget); // Releases a pointer grab
	bool isPointerGrabbed(void) const // Checks for an active pointer grab (hard or soft)
		{
		return pointerGrabWidget!=0;
		}
	bool requestFocus(Widget* widget); // Allows a widget to request the text entry focus; returns false if request was denied
	void releaseFocus(Widget* widget); // Allows a widget to release the text entry focus
	bool hasFocus(const Widget* widget) const // Returns true if the given widget currently has the text entry focus
		{
		return textFocusWidget==widget;
		}
	void focusPreviousWidget(void); // Moves the text entry focus to the previous widget in the list
	void focusNextWidget(void); // Moves the text entry focus to the next widget in the list
	bool text(const TextEvent& textEvent); // Handles a text event; returns true if event was received by a widget
	bool textControl(Event& event,const TextControlEvent& textControlEvent); // Handles a text control event; returns true if event was received by a widget
	bool textControl(const TextControlEvent& textControlEvent); // Ditto; sends text control event to current focus widget
	int getTextBufferLength(void) const // Returns length of current cut & paste buffer
		{
		return textBufferLength;
		}
	const char* getTextBuffer(void) const // Returns current cut & paste buffer
		{
		return textBuffer;
		}
	void setTextBuffer(int newTextBufferLength,const char* newTextBuffer); // Sets the cut & paste buffer
	Misc::CallbackList& getWidgetPopCallbacks(void) // Returns the list of widget pop-up callbacks
		{
		return widgetPopCallbacks;
		}
	Misc::CallbackList& getWidgetMoveCallbacks(void) // Returns the list of widget move callbacks
		{
		return widgetMoveCallbacks;
		}
	};

}

#endif
