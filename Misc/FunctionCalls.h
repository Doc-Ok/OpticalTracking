/***********************************************************************
FunctionCalls - Set of functor objects implementing function (or method)
calls as first-class variables.
Copyright (c) 2009-2014 Oliver Kreylos

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

#ifndef MISC_FUNCTIONCALLS_INCLUDED
#define MISC_FUNCTIONCALLS_INCLUDED

namespace Misc {

/* Abstract base class for function call functors: */
template <class ParameterParam>
class FunctionCall
	{
	/* Embedded classes: */
	public:
	typedef ParameterParam Parameter; // Function call parameter type
	
	/* Constructors and destructors: */
	public:
	virtual ~FunctionCall(void)
		{
		}
	
	/* Methods: */
	virtual void operator()(Parameter parameter) const =0; // Function call operator
	};

/* Class to call C-style functions: */
template <class ParameterParam>
class VoidFunctionCall:public FunctionCall<ParameterParam>
	{
	/* Embedded classes: */
	public:
	typedef typename FunctionCall<ParameterParam>::Parameter Parameter;
	typedef void (*Function)(Parameter); // Type for function pointers
	
	/* Elements: */
	private:
	Function function; // The function pointer
	
	/* Constructors and destructors: */
	public:
	VoidFunctionCall(Function sFunction) // Creates a functor wrapper for the given function
		:function(sFunction)
		{
		}
	
	/* Methods from FunctionCall: */
	virtual void operator()(Parameter parameter) const
		{
		/* Simply call the function: */
		function(parameter);
		}
	};

/* Class to call C-style functions taking a single additional argument of arbitrary type: */
template <class ParameterParam,class ArgumentParam>
class SingleArgumentFunctionCall:public FunctionCall<ParameterParam>
	{
	/* Embedded classes: */
	public:
	typedef typename FunctionCall<ParameterParam>::Parameter Parameter;
	typedef ArgumentParam Argument; // Argument type
	typedef void (*Function)(Parameter,Argument); // Type for function pointers
	
	/* Elements: */
	private:
	Function function; // The function pointer
	Argument argument; // The argument to pass to the function
	
	/* Constructors and destructors: */
	public:
	SingleArgumentFunctionCall(Function sFunction,const Argument& sArgument) // Creates a functor wrapper for the given function and argument
		:function(sFunction),argument(sArgument)
		{
		}
	
	/* Methods from FunctionCall: */
	virtual void operator()(Parameter parameter) const
		{
		/* Call the function with the provided argument: */
		function(parameter,argument);
		}
	
	/* New methods: */
	void setArgument(const Argument& newArgument) // Changes the function call argument
		{
		argument=newArgument;
		}
	};

/* Class to call C++ methods: */
template <class ParameterParam,class CalleeParam>
class VoidMethodCall:public FunctionCall<ParameterParam>
	{
	/* Embedded classes: */
	public:
	typedef typename FunctionCall<ParameterParam>::Parameter Parameter;
	typedef CalleeParam Callee; // Type of called objects
	typedef void (Callee::*Method)(Parameter); // Type for method pointers
	
	/* Elements: */
	private:
	Callee* callee; // Object whose method to call
	Method method; // The method pointer
	
	/* Constructors and destructors: */
	public:
	VoidMethodCall(Callee* sCallee,Method sMethod) // Creates a functor wrapper for the given method on the given object
		:callee(sCallee),method(sMethod)
		{
		}
	
	/* Methods from FunctionCall: */
	virtual void operator()(Parameter parameter) const
		{
		/* Call the method on the provided object: */
		(callee->*method)(parameter);
		}
	};

/* Class to call C++ methods on const objects: */
template <class ParameterParam,class CalleeParam>
class VoidConstMethodCall:public FunctionCall<ParameterParam>
	{
	/* Embedded classes: */
	public:
	typedef typename FunctionCall<ParameterParam>::Parameter Parameter;
	typedef CalleeParam Callee; // Type of called objects
	typedef void (Callee::*Method)(Parameter) const; // Type for method pointers
	
	/* Elements: */
	private:
	const Callee* callee; // Object whose method to call
	Method method; // The method pointer
	
	/* Constructors and destructors: */
	public:
	VoidConstMethodCall(const Callee* sCallee,Method sMethod) // Creates a functor wrapper for the given method on the given object
		:callee(sCallee),method(sMethod)
		{
		}
	
	/* Methods from FunctionCall: */
	virtual void operator()(Parameter parameter) const
		{
		/* Call the method on the provided object: */
		(callee->*method)(parameter);
		}
	};

/* Class to call C++ methods taking a single additional argument of arbitrary type: */
template <class ParameterParam,class CalleeParam,class ArgumentParam>
class SingleArgumentMethodCall:public FunctionCall<ParameterParam>
	{
	/* Embedded classes: */
	public:
	typedef typename FunctionCall<ParameterParam>::Parameter Parameter;
	typedef CalleeParam Callee; // Type of called objects
	typedef ArgumentParam Argument; // Argument type
	typedef void (Callee::*Method)(Parameter,Argument); // Type for method pointers
	
	/* Elements: */
	private:
	Callee* callee; // Object whose method to call
	Method method; // The method pointer
	Argument argument; // The argument to pass to the method
	
	/* Constructors and destructors: */
	public:
	SingleArgumentMethodCall(Callee* sCallee,Method sMethod,const Argument& sArgument) // Creates a functor wrapper for the given method on the given object and the given argument
		:callee(sCallee),method(sMethod),argument(sArgument)
		{
		}
	
	/* Methods from FunctionCall: */
	virtual void operator()(Parameter parameter) const
		{
		/* Call the method on the provided object with the provided argument: */
		(callee->*method)(parameter,argument);
		}
	
	/* New methods: */
	void setArgument(const Argument& newArgument) // Changes the method call argument
		{
		argument=newArgument;
		}
	};

/* Class to call C++ methods taking a single additional argument of arbitrary type on const objects: */
template <class ParameterParam,class CalleeParam,class ArgumentParam>
class SingleArgumentConstMethodCall:public FunctionCall<ParameterParam>
	{
	/* Embedded classes: */
	public:
	typedef typename FunctionCall<ParameterParam>::Parameter Parameter;
	typedef CalleeParam Callee; // Type of called objects
	typedef ArgumentParam Argument; // Argument type
	typedef void (Callee::*Method)(Parameter,Argument) const; // Type for method pointers
	
	/* Elements: */
	private:
	const Callee* callee; // Object whose method to call
	Method method; // The method pointer
	Argument argument; // The argument to pass to the method
	
	/* Constructors and destructors: */
	public:
	SingleArgumentConstMethodCall(const Callee* sCallee,Method sMethod,const Argument& sArgument) // Creates a functor wrapper for the given method on the given object and the given argument
		:callee(sCallee),method(sMethod),argument(sArgument)
		{
		}
	
	/* Methods from FunctionCall: */
	virtual void operator()(Parameter parameter) const
		{
		/* Call the method on the provided object with the provided argument: */
		(callee->*method)(parameter,argument);
		}
	
	/* New methods: */
	void setArgument(const Argument& newArgument) // Changes the method call argument
		{
		argument=newArgument;
		}
	};

/****************
Helper functions:
****************/

template <class ParameterParam>
inline
FunctionCall<ParameterParam>*
createFunctionCall(
	void (*function)(ParameterParam))
	{
	return new VoidFunctionCall<ParameterParam>(function);
	}

template <class ParameterParam,class ArgumentParam>
inline
FunctionCall<ParameterParam>*
createFunctionCall(
	void (*function)(ParameterParam,ArgumentParam),
	ArgumentParam argument)
	{
	return new SingleArgumentFunctionCall<ParameterParam,ArgumentParam>(function,argument);
	}

template <class ParameterParam,class CalleeParam>
inline
FunctionCall<ParameterParam>*
createFunctionCall(
	CalleeParam* callee,
	void (CalleeParam::*method)(ParameterParam))
	{
	return new VoidMethodCall<ParameterParam,CalleeParam>(callee,method);
	}

template <class ParameterParam,class CalleeParam>
inline
FunctionCall<ParameterParam>*
createFunctionCall(
	const CalleeParam* callee,
	void (CalleeParam::*method)(ParameterParam) const)
	{
	return new VoidConstMethodCall<ParameterParam,CalleeParam>(callee,method);
	}

template <class ParameterParam,class CalleeParam,class ArgumentParam>
inline
FunctionCall<ParameterParam>*
createFunctionCall(
	CalleeParam* callee,
	void (CalleeParam::*method)(ParameterParam,ArgumentParam),
	ArgumentParam argument)
	{
	return new SingleArgumentMethodCall<ParameterParam,CalleeParam,ArgumentParam>(callee,method,argument);
	}

template <class ParameterParam,class CalleeParam,class ArgumentParam>
inline
FunctionCall<ParameterParam>*
createFunctionCall(
	const CalleeParam* callee,
	void (CalleeParam::*method)(ParameterParam,const ArgumentParam&) const,
	ArgumentParam argument)
	{
	return new SingleArgumentConstMethodCall<ParameterParam,CalleeParam,const ArgumentParam&>(callee,method,argument);
	}

}

#endif
