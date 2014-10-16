/***********************************************************************
CallbackList - Class for lists of callback functions associated with
certain events. Uses new-style templatized callback mechanism and offers
backwards compatibility for traditional C-style callbacks.
Copyright (c) 2000-2012 Oliver Kreylos

This file is part of the Miscellaneous Support Library (Misc).

The Miscellaneous Support Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Miscellaneous Support Library is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Miscellaneous Support Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef MISC_CALLBACKLIST_INCLUDED
#define MISC_CALLBACKLIST_INCLUDED

#include <Misc/CallbackData.h>

namespace Misc {

class CallbackList // Class for lists of callbacks
	{
	/* Embedded classes: */
	private:
	
	/* Abstract base class for callback list items: */
	class CallbackListItem
		{
		friend class CallbackList;
		
		/* Elements: */
		private:
		CallbackListItem* succ; // Pointer to next item in callback list
		
		/* Constructors and destructors: */
		public:
		CallbackListItem(void); // Creates an empty singleton callback list item
		virtual ~CallbackListItem(void); // Virtual destructor
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const =0; // Virtual method to compare callbacks
		virtual void call(CallbackData* callbackData) const =0; // Virtual method to invoke callback
		};
	
	/* Class to call C functions: */
	class FunctionCallback:public CallbackListItem
		{
		/* Embedded classes: */
		public:
		typedef void (*CallbackFunction)(CallbackData*); // Type of called callback function
		
		/* Elements: */
		private:
		CallbackFunction callbackFunction; // Pointer to callback function
		
		/* Constructors and destructors: */
		public:
		FunctionCallback(CallbackFunction sCallbackFunction); // Creates callback for given function
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const;
		virtual void call(CallbackData* cbData) const;
		};
	
	/* Class to call C functions with an additional void* parameter (traditional C-style callback): */
	class FunctionVoidArgCallback:public CallbackListItem
		{
		/* Embedded classes: */
		public:
		typedef void (*CallbackFunction)(CallbackData*,void*); // Type of called callback function
		
		/* Elements: */
		private:
		CallbackFunction callbackFunction; // Pointer to callback function
		void* userData; // Additional callback function parameter
		
		/* Constructors and destructors: */
		public:
		FunctionVoidArgCallback(CallbackFunction sCallbackFunction,void* sUserData); // Creates callback for given function with given additional parameter
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const;
		virtual void call(CallbackData* cbData) const;
		};
	
	/* Class to call arbitrary methods on objects of arbitrary type: */
	template <class CallbackClassParam>
	class MethodCallback:public CallbackListItem
		{
		/* Embedded classes: */
		public:
		typedef CallbackClassParam CallbackClass; // Class of called objects
		typedef void (CallbackClass::*CallbackMethod)(CallbackData*); // Type of called callback method
		
		/* Elements: */
		private:
		CallbackClass* callbackObject; // Pointer to callback object
		CallbackMethod callbackMethod; // Pointer to callback method
		
		/* Constructors and destructors: */
		public:
		MethodCallback(CallbackClass* sCallbackObject,CallbackMethod sCallbackMethod) // Creates callback for given method on given object
			:callbackObject(sCallbackObject),callbackMethod(sCallbackMethod)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const
			{
			const MethodCallback* other2=dynamic_cast<const MethodCallback*>(&other);
			return other2!=0&&callbackObject==other2->callbackObject&&callbackMethod==other2->callbackMethod;
			}
		virtual void call(CallbackData* callbackData) const
			{
			/* Call the callback method on the callback object: */
			(callbackObject->*callbackMethod)(callbackData);
			}
		};
	
	/* Class to call arbitrary methods on objects of arbitrary type with an additional parameter of arbitrary type: */
	template <class CallbackClassParam,class ParameterParam>
	class MethodParameterCallback:public CallbackListItem
		{
		/* Embedded classes: */
		public:
		typedef CallbackClassParam CallbackClass; // Class of called objects
		typedef ParameterParam Parameter; // Type of additional parameter
		typedef void (CallbackClass::*CallbackMethod)(CallbackData*,const Parameter&); // Type of called callback method
		
		/* Elements: */
		private:
		CallbackClass* callbackObject; // Pointer to callback object
		CallbackMethod callbackMethod; // Pointer to callback method
		Parameter parameter; // Additional parameter
		
		/* Constructors and destructors: */
		public:
		MethodParameterCallback(CallbackClass* sCallbackObject,CallbackMethod sCallbackMethod,const Parameter& sParameter) // Creates callback for given method on given object with given additional parameter
			:callbackObject(sCallbackObject),callbackMethod(sCallbackMethod),parameter(sParameter)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const
			{
			const MethodParameterCallback* other2=dynamic_cast<const MethodParameterCallback*>(&other);
			return other2!=0&&callbackObject==other2->callbackObject&&callbackMethod==other2->callbackMethod&&parameter==other2->parameter;
			}
		virtual void call(CallbackData* callbackData) const
			{
			/* Call the callback method on the callback object: */
			(callbackObject->*callbackMethod)(callbackData,parameter);
			}
		};
	
	/* Class to call arbitrary methods taking a parameter derived from CallbackData on objects of arbitrary type: */
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	class MethodCastCallback:public CallbackListItem
		{
		/* Embedded classes: */
		public:
		typedef CallbackClassParam CallbackClass; // Class of called objects
		typedef DerivedCallbackDataParam DerivedCallbackData; // Class of callback data (must be derived from CallbackData)
		typedef void (CallbackClass::*CallbackMethod)(DerivedCallbackData*); // Type of called callback method
		
		/* Elements: */
		private:
		CallbackClass* callbackObject; // Pointer to callback object
		CallbackMethod callbackMethod; // Pointer to callback method
		
		/* Constructors and destructors: */
		public:
		MethodCastCallback(CallbackClass* sCallbackObject,CallbackMethod sCallbackMethod) // Creates callback for given method on given object
			:callbackObject(sCallbackObject),callbackMethod(sCallbackMethod)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const
			{
			const MethodCastCallback* other2=dynamic_cast<const MethodCastCallback*>(&other);
			return other2!=0&&callbackObject==other2->callbackObject&&callbackMethod==other2->callbackMethod;
			}
		virtual void call(CallbackData* callbackData) const
			{
			/* Call the callback method on the callback object with downcasted callback data: */
			(callbackObject->*callbackMethod)(static_cast<DerivedCallbackData*>(callbackData));
			}
		};
	
	/* Class to call arbitrary methods taking a parameter derived from CallbackData on objects of arbitrary type and an additional parameter of arbitrary type: */
	template <class CallbackClassParam,class DerivedCallbackDataParam,class ParameterParam>
	class MethodCastParameterCallback:public CallbackListItem
		{
		/* Embedded classes: */
		public:
		typedef CallbackClassParam CallbackClass; // Class of called objects
		typedef DerivedCallbackDataParam DerivedCallbackData; // Class of callback data (must be derived from CallbackData)
		typedef ParameterParam Parameter; // Type of additional parameter
		typedef void (CallbackClass::*CallbackMethod)(DerivedCallbackData*,const Parameter&); // Type of called callback method
		
		/* Elements: */
		private:
		CallbackClass* callbackObject; // Pointer to callback object
		CallbackMethod callbackMethod; // Pointer to callback method
		Parameter parameter; // Additional parameter
		
		/* Constructors and destructors: */
		public:
		MethodCastParameterCallback(CallbackClass* sCallbackObject,CallbackMethod sCallbackMethod,const Parameter& sParameter) // Creates callback for given method on given object with given additional parameter
			:callbackObject(sCallbackObject),callbackMethod(sCallbackMethod),parameter(sParameter)
			{
			}
		
		/* Methods: */
		virtual bool operator==(const CallbackListItem& other) const
			{
			const MethodCastParameterCallback* other2=dynamic_cast<const MethodCastParameterCallback*>(&other);
			return other2!=0&&callbackObject==other2->callbackObject&&callbackMethod==other2->callbackMethod&&parameter==other2->parameter;
			}
		virtual void call(CallbackData* callbackData) const
			{
			/* Call the callback method on the callback object with downcasted callback data and the additional parameter: */
			(callbackObject->*callbackMethod)(static_cast<DerivedCallbackData*>(callbackData),parameter);
			}
		};
	
	/* Elements: */
	private:
	CallbackListItem* head; // Pointer to first callback in list
	CallbackListItem* tail; // Pointer to last callback in list
	mutable bool interruptRequested; // Flag that the current call() operation is to be aborted after the current callback
	
	/* Private methods: */
	void addCli(CallbackListItem* newCli); // Adds a new callback list item to the back of the list
	void addCliToFront(CallbackListItem* newCli); // Adds a new callback list item to the front of the list
	void removeCli(const CallbackListItem& removeCli); // Removes the first callback list item equal to the given one from the list
	
	/* Constructors and destructors: */
	public:
	CallbackList(void); // Creates an empty callback list
	private:
	CallbackList(const CallbackList& source); // Prohibit copy constructor
	CallbackList& operator=(const CallbackList& source); // Prohibit assignment operator
	public:
	~CallbackList(void); // Destroys the callback list and all its callbacks
	
	/*********************************************************************
	Callback list creation/manipulation methods:
	*********************************************************************/
	
	/* Interface for C-style callbacks with no additional argument: */
	void add(FunctionCallback::CallbackFunction newCallbackFunction) // Adds a callback to the end of the list
		{
		addCli(new FunctionCallback(newCallbackFunction));
		}
	void addToFront(FunctionCallback::CallbackFunction newCallbackFunction) // Adds a callback to the front of the list
		{
		addCliToFront(new FunctionCallback(newCallbackFunction));
		}
	void remove(FunctionCallback::CallbackFunction removeCallbackFunction) // Removes the first matching callback from the list
		{
		removeCli(FunctionCallback(removeCallbackFunction));
		}
	
	/* Interface for traditional C-style callbacks (with void* argument): */
	void add(FunctionVoidArgCallback::CallbackFunction newCallbackFunction,void* newUserData) // Adds a callback to the end of the list
		{
		addCli(new FunctionVoidArgCallback(newCallbackFunction,newUserData));
		}
	void addToFront(FunctionVoidArgCallback::CallbackFunction newCallbackFunction,void* newUserData) // Adds a callback to the front of the list
		{
		addCliToFront(new FunctionVoidArgCallback(newCallbackFunction,newUserData));
		}
	void remove(FunctionVoidArgCallback::CallbackFunction removeCallbackFunction,void* removeUserData) // Removes the first matching callback from the list
		{
		removeCli(FunctionVoidArgCallback(removeCallbackFunction,removeUserData));
		}
	
	/* Interface for method callbacks: */
	template <class CallbackClassParam>
	void add(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(CallbackData*)) // Adds a callback to the end of the list
		{
		addCli(new MethodCallback<CallbackClassParam>(newCallbackObject,newCallbackMethod));
		}
	template <class CallbackClassParam>
	void addToFront(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(CallbackData*)) // Adds a callback to the front of the list
		{
		addCliToFront(new MethodCallback<CallbackClassParam>(newCallbackObject,newCallbackMethod));
		}
	template <class CallbackClassParam>
	void remove(CallbackClassParam* removeCallbackObject,void (CallbackClassParam::*removeCallbackMethod)(CallbackData*)) // Removes the first matching callback from the list
		{
		removeCli(MethodCallback<CallbackClassParam>(removeCallbackObject,removeCallbackMethod));
		}
	
	/* Interface for method callbacks with additional parameter: */
	template <class CallbackClassParam,class ParameterParam>
	void add(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(CallbackData*,const ParameterParam&),const ParameterParam& newParameter) // Adds a callback to the end of the list
		{
		addCli(new MethodParameterCallback<CallbackClassParam,ParameterParam>(newCallbackObject,newCallbackMethod,newParameter));
		}
	template <class CallbackClassParam,class ParameterParam>
	void addToFront(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(CallbackData*,const ParameterParam&),const ParameterParam& newParameter) // Adds a callback to the front of the list
		{
		addCliToFront(new MethodParameterCallback<CallbackClassParam,ParameterParam>(newCallbackObject,newCallbackMethod,newParameter));
		}
	template <class CallbackClassParam,class ParameterParam>
	void remove(CallbackClassParam* removeCallbackObject,void (CallbackClassParam::*removeCallbackMethod)(CallbackData*,const ParameterParam&),const ParameterParam& removeParameter) // Removes the first matching callback from the list
		{
		removeCli(MethodParameterCallback<CallbackClassParam,ParameterParam>(removeCallbackObject,removeCallbackMethod,removeParameter));
		}
	
	/* Interface for method callbacks with automatic callback data downcast: */
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	void add(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(DerivedCallbackDataParam*)) // Adds a callback to the end of the list
		{
		addCli(new MethodCastCallback<CallbackClassParam,DerivedCallbackDataParam>(newCallbackObject,newCallbackMethod));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	void addToFront(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(DerivedCallbackDataParam*)) // Adds a callback to the front of the list
		{
		addCliToFront(new MethodCastCallback<CallbackClassParam,DerivedCallbackDataParam>(newCallbackObject,newCallbackMethod));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam>
	void remove(CallbackClassParam* removeCallbackObject,void (CallbackClassParam::*removeCallbackMethod)(DerivedCallbackDataParam*)) // Removes the first matching callback from the list
		{
		removeCli(MethodCastCallback<CallbackClassParam,DerivedCallbackDataParam>(removeCallbackObject,removeCallbackMethod));
		}
	
	/* Interface for method callbacks with automatic callback data downcast and additional parameter: */
	template <class CallbackClassParam,class DerivedCallbackDataParam,class ParameterParam>
	void add(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(DerivedCallbackDataParam*,const ParameterParam&),const ParameterParam& newParameter) // Adds a callback to the end of the list
		{
		addCli(new MethodCastParameterCallback<CallbackClassParam,DerivedCallbackDataParam,ParameterParam>(newCallbackObject,newCallbackMethod,newParameter));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam,class ParameterParam>
	void addToFront(CallbackClassParam* newCallbackObject,void (CallbackClassParam::*newCallbackMethod)(DerivedCallbackDataParam*,const ParameterParam&),const ParameterParam& newParameter) // Adds a callback to the front of the list
		{
		addCliToFront(new MethodCastParameterCallback<CallbackClassParam,DerivedCallbackDataParam,ParameterParam>(newCallbackObject,newCallbackMethod,newParameter));
		}
	template <class CallbackClassParam,class DerivedCallbackDataParam,class ParameterParam>
	void remove(CallbackClassParam* removeCallbackObject,void (CallbackClassParam::*removeCallbackMethod)(DerivedCallbackDataParam*,const ParameterParam&),const ParameterParam& removeParameter) // Removes the first matching callback from the list
		{
		removeCli(MethodCastParameterCallback<CallbackClassParam,DerivedCallbackDataParam,ParameterParam>(removeCallbackObject,removeCallbackMethod,removeParameter));
		}
	
	/* Callback list calling interface: */
	void call(CallbackData* callbackData) const; // Calls all callbacks in the list
	void requestInterrupt(void) const; // Allows a callback to request interrupting callback processing
	};

}

#endif
