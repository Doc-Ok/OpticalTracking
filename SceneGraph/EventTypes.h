/***********************************************************************
EventTypes - Classes for typed event sources and sinks.
Copyright (c) 2009-2010 Oliver Kreylos

This file is part of the Simple Scene Graph Renderer (SceneGraph).

The Simple Scene Graph Renderer is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Simple Scene Graph Renderer is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Simple Scene Graph Renderer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef SCENEGRAPH_EVENTTYPES_INCLUDED
#define SCENEGRAPH_EVENTTYPES_INCLUDED

// DEBUGGING
#include <iostream>
#include <SceneGraph/EventOut.h>
#include <SceneGraph/EventIn.h>
#include <SceneGraph/Route.h>
#include <SceneGraph/FieldTypes.h>

namespace SceneGraph {

/* Forward declarations: */
template <class FieldParam>
class EO;
template <class FieldParam>
class EI;

/******************************************
Generic class for routes connecting fields:
******************************************/

template <class FieldParam>
class R:public Route
	{
	/* Embedded classes: */
	public:
	typedef FieldParam Field; // Field type
	typedef typename FieldParam::Value Value; // Value type
	};

template <class ValueParam>
class R<SF<ValueParam> >:public Route
	{
	/* Embedded classes: */
	public:
	typedef SF<ValueParam> Field; // Field type
	typedef typename Field::Value Value; // Value type
	
	/* Elements: */
	protected:
	const Node* sourceNode;
	const Field* sourceField;
	Node* sinkNode;
	Field* sinkField;
	
	/* Constructors and destructors: */
	public:
	R(const EO<Field>* sSource,const EI<Field>* sSink)
		:sourceNode(sSource->getNode()),sourceField(sSource->field),
		 sinkNode(sSink->getNode()),sinkField(sSink->field)
		{
		std::cout<<"Created single-valued field route"<<std::endl;
		}
	};

template <class ValueParam>
class R<MF<ValueParam> >:public Route
	{
	/* Embedded classes: */
	public:
	typedef MF<ValueParam> Field; // Field type
	typedef typename Field::Value Value; // Value type
	typedef typename Field::ValueList ValueList; // Value list type
	
	/* Elements: */
	protected:
	const Node* sourceNode;
	const Field* sourceField;
	Node* sinkNode;
	Field* sinkField;
	
	/* Constructors and destructors: */
	public:
	R(const EO<Field>* sSource,const EI<Field>* sSink)
		:sourceNode(sSource->getNode()),sourceField(sSource->field),
		 sinkNode(sSink->getNode()),sinkField(sSink->field)
		{
		std::cout<<"Created multi-valued field route"<<std::endl;
		}
	};

/***************************************
Generic class for fields as event sinks:
***************************************/

template <class FieldParam>
class EI:public EventIn
	{
	/* Embedded classes: */
	public:
	typedef FieldParam Field; // Field type
	typedef typename FieldParam::Value Value; // Value type
	};

template <class ValueParam>
class EI<SF<ValueParam> >:public EventIn
	{
	friend class R<SF<ValueParam> >;
	
	/* Embedded classes: */
	public:
	typedef SF<ValueParam> Field; // Field type
	typedef typename Field::Value Value; // Value type
	
	/* Elements: */
	protected:
	Field* field; // Pointer to the field receiving events
	
	/* Constructors and destructors: */
	public:
	EI(Node* sNode,Field* sField) // Creates an event sink for the given node and field
		:EventIn(sNode),
		 field(sField)
		{
		}
	};

template <class ValueParam>
class EI<MF<ValueParam> >:public EventIn
	{
	friend class R<MF<ValueParam> >;
	
	/* Embedded classes: */
	public:
	typedef MF<ValueParam> Field; // Field type
	typedef typename Field::Value Value; // Value type
	typedef typename Field::ValueList ValueList; // Value list type
	
	/* Elements: */
	protected:
	Field* field; // Pointer to the field receiving events
	
	/* Constructors and destructors: */
	public:
	EI(Node* sNode,Field* sField) // Creates an event sink for the given node and field
		:EventIn(sNode),
		 field(sField)
		{
		}
	};

template <class FieldParam>
EI<FieldParam>* makeEventIn(Node* sNode,FieldParam& sField)
	{
	return new EI<FieldParam>(sNode,&sField);
	}

/*****************************************
Generic class for fields as event sources:
*****************************************/

template <class FieldParam>
class EO:public EventOut
	{
	/* Embedded classes: */
	public:
	typedef FieldParam Field; // Field type
	typedef typename FieldParam::Value Value; // Value type
	};

template <class ValueParam>
class EO<SF<ValueParam> >:public EventOut
	{
	friend class R<SF<ValueParam> >;
	
	/* Embedded classes: */
	public:
	typedef SF<ValueParam> Field; // Field type
	typedef typename Field::Value Value; // Value type
	
	/* Elements: */
	protected:
	const Field* field; // Pointer to the field generating events
	
	/* Constructors and destructors: */
	public:
	EO(const Node* sNode,const Field* sField) // Creates an event source for the given node and field
		:EventOut(sNode),
		 field(sField)
		{
		}
	
	/* Methods from EventOut: */
	virtual Route* connectTo(const EventIn* sink) const
		{
		/* Check if the event sink is of the same type: */
		const EI<Field>* mySink=dynamic_cast<const EI<Field>*>(sink);
		if(mySink==0)
			throw Route::TypeMismatchError();
		
		/* Return a new route: */
		return new R<Field>(this,mySink);
		}
	};

template <class ValueParam>
class EO<MF<ValueParam> >:public EventOut
	{
	friend class R<MF<ValueParam> >;
	
	/* Embedded classes: */
	public:
	typedef MF<ValueParam> Field; // Field type
	typedef typename Field::Value Value; // Value type
	typedef typename Field::ValueList ValueList; // Value list type
	
	/* Elements: */
	protected:
	const Field* field; // Pointer to the field generating events
	
	/* Constructors and destructors: */
	public:
	EO(const Node* sNode,const Field* sField) // Creates an event source for the given node and field
		:EventOut(sNode),
		 field(sField)
		{
		}
	
	/* Methods from EventOut: */
	virtual Route* connectTo(const EventIn* sink) const
		{
		/* Check if the event sink is of the same type: */
		const EI<Field>* mySink=dynamic_cast<const EI<Field>*>(sink);
		if(mySink==0)
			throw Route::TypeMismatchError();
		
		/* Return a new route: */
		return new R<Field>(this,mySink);
		}
	};

template <class FieldParam>
EO<FieldParam>* makeEventOut(const Node* sNode,const FieldParam& sField)
	{
	return new EO<FieldParam>(sNode,&sField);
	}

}

#endif
