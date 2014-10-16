/***********************************************************************
WidgetAttribute - Generic class to associate attributes of arbitrary
types with widgets.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef GLMOTIF_WIDGETATTRIBUTE_INCLUDED
#define GLMOTIF_WIDGETATTRIBUTE_INCLUDED

class WidgetAttributeBase // Base class for widget attributes
	{
	/* Constructors and destructors: */
	public:
	virtual ~WidgetAttributeBase(void)
		{
		}
	};

template <class AttributeParam>
class WidgetAttribute:public WidgetAttributeBase
	{
	/* Embedded classes: */
	public:
	typedef AttributeParam Attribute; // Type of attribute
	
	/* Elements: */
	private:
	Attribute value; // Value of widget attribute
	
	/* Constructors and destructors: */
	public:
	WidgetAttribute(const Attribute& sValue) // Constructs attribute with given value
		:value(sValue)
		{
		}
	private:
	WidgetAttribute(const WidgetAttribute& source); // Prohibit copy constructor
	WidgetAttribute& operator=(const WidgetAttribute& source); // Prohibit assignment operator
	
	/* New methods: */
	public:
	const Attribute& getValue(void) const // Returns attribute's value
		{
		return value;
		}
	Attribute& getValue(void) // Ditto
		{
		return value;
		}
	};

#endif
