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

#include <Misc/CallbackList.h>

namespace Misc {

/***********************************************
Methods of class CallbackList::CallbackListItem:
***********************************************/

CallbackList::CallbackListItem::CallbackListItem(void)
	:succ(0)
	{
	}

CallbackList::CallbackListItem::~CallbackListItem(void)
	{
	}

/***********************************************
Methods of class CallbackList::FunctionCallback:
***********************************************/

CallbackList::FunctionCallback::FunctionCallback(CallbackList::FunctionCallback::CallbackFunction sCallbackFunction)
	:callbackFunction(sCallbackFunction)
	{
	}

bool CallbackList::FunctionCallback::operator==(const CallbackList::CallbackListItem& other) const
	{
	const FunctionCallback* other2=dynamic_cast<const FunctionCallback*>(&other);
	return other2!=0&&callbackFunction==other2->callbackFunction;
	}

void CallbackList::FunctionCallback::call(CallbackData* callbackData) const
	{
	/* Call the callback function: */
	callbackFunction(callbackData);
	}

/******************************************************
Methods of class CallbackList::FunctionVoidArgCallback:
******************************************************/

CallbackList::FunctionVoidArgCallback::FunctionVoidArgCallback(CallbackList::FunctionVoidArgCallback::CallbackFunction sCallbackFunction,void* sUserData)
	:callbackFunction(sCallbackFunction),
	 userData(sUserData)
	{
	}

bool CallbackList::FunctionVoidArgCallback::operator==(const CallbackList::CallbackListItem& other) const
	{
	const FunctionVoidArgCallback* other2=dynamic_cast<const FunctionVoidArgCallback*>(&other);
	return other2!=0&&callbackFunction==other2->callbackFunction&&userData==other2->userData;
	}

void CallbackList::FunctionVoidArgCallback::call(CallbackData* callbackData) const
	{
	/* Call the callback function: */
	callbackFunction(callbackData,userData);
	}

/*****************************
Methods of class CallbackList:
*****************************/

void CallbackList::addCli(CallbackList::CallbackListItem* newCli)
	{
	if(tail!=0)
		tail->succ=newCli;
	else
		head=newCli;
	tail=newCli;
	}

void CallbackList::addCliToFront(CallbackList::CallbackListItem* newCli)
	{
	newCli->succ=head;
	if(head==0)
		tail=newCli;
	head=newCli;
	}

void CallbackList::removeCli(const CallbackList::CallbackListItem& removeCli)
	{
	/* Find the first callback list item equal to the given one: */
	CallbackListItem* ptr1=0;
	for(CallbackListItem* ptr2=head;ptr2!=0;ptr1=ptr2,ptr2=ptr2->succ)
		if(*ptr2==removeCli)
			{
			/* Remove the item from the list: */
			if(tail==ptr2)
				tail=ptr1;
			if(ptr1!=0)
				ptr1->succ=ptr2->succ;
			else
				head=ptr2->succ;
			
			/* Delete the item: */
			delete ptr2;
			
			break;
			}
	}

CallbackList::CallbackList(void)
	:head(0),tail(0),
	 interruptRequested(false)
	{
	}

CallbackList::~CallbackList(void)
	{
	/* Delete all callback list items: */
	while(head!=0)
		{
		CallbackListItem* succ=head->succ;
		delete head;
		head=succ;
		}
	}

void CallbackList::call(CallbackData* callbackData) const
	{
	/* Reset the interrupt request flag: */
	interruptRequested=false;
	
	/* Set the originator pointer in the callback data structure: */
	callbackData->callbackList=this;
	
	/* Call all callbacks until interrupted: */
	for(const CallbackListItem* cliPtr=head;cliPtr!=0&&!interruptRequested;cliPtr=cliPtr->succ)
		cliPtr->call(callbackData);
	}

void CallbackList::requestInterrupt(void) const
	{
	/* Raise the interrupt request flag: */
	interruptRequested=true;
	}

}
