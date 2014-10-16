/***********************************************************************
Atomic - Class for integer data types with atomic addition / subtraction
operations.
Copyright (c) 2012 Oliver Kreylos

This file is part of the Portable Threading Library (Threads).

The Portable Threading Library is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Portable Threading Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Portable Threading Library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#ifndef THREADS_ATOMIC_INCLUDED
#define THREADS_ATOMIC_INCLUDED

#include <pthread.h>
#include <Threads/Config.h>
#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
#include <Threads/Spinlock.h>
#endif

namespace Threads {

template <class ValueParam>
class Atomic
	{
	/* Embedded classes: */
	public:
	typedef ValueParam Value; // Value type on which is operated atomically
	
	/* Elements: */
	private:
	#if !THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
	Spinlock mutex; // Busy-wait (if available) mutual exclusion semaphore protecting the atomic value
	#endif
	Value value; // The object's current value
	
	/* Constructors and destructors: */
	public:
	Atomic(Value sValue) // Initializes the object with the given value
		:value(sValue)
		{
		}
	private:
	Atomic(const Atomic& source); // Prohibit copy constructor
	Atomic& operator=(const Atomic& source); // Prohibit assignment operator
	
	/* Methods: */
	public:
	
	/* Pre-operation methods; return atomic value after operation: */
	Value preAdd(Value other) // Pre-addition
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_add_and_fetch(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		return value+=other;
		#endif
		}
	Value preSub(Value other) // Pre-subtraction
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_sub_and_fetch(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		return value-=other;
		#endif
		}
	Value preOr(Value other) // Pre-bitwise or
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_or_and_fetch(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		return value|=other;
		#endif
		}
	Value preAnd(Value other) // Pre-bitwise and
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_and_and_fetch(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		return value&=other;
		#endif
		}
	Value preX0r(Value other) // Pre-bitwise exclusive or
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_xor_and_fetch(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		return value^=other;
		#endif
		}
	Value preNand(Value other) // Pre-bitwise not-and
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_nand_and_fetch(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		value=~value&other;
		return value;
		#endif
		}
	
	/* Post-operation methods; return atomic value before operation: */
	Value postAdd(Value other) // Post-addition
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_fetch_and_add(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		value+=other;
		return result;
		#endif
		}
	Value postSub(Value other) // Post-subtraction
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_fetch_and_sub(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		value-=other;
		return result;
		#endif
		}
	Value postOr(Value other) // Post-bitwise or
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_fetch_and_or(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		value|=other;
		return result;
		#endif
		}
	Value postAnd(Value other) // Post-bitwise and
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_fetch_and_and(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		value&=other;
		return result;
		#endif
		}
	Value postX0r(Value other) // Post-bitwise exclusive or
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_fetch_and_xor(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		value^=other;
		return result;
		#endif
		}
	Value postNand(Value other) // Post-bitwise not-and
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_fetch_and_nand(&value,other);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		value=~value&other;
		return result;
		#endif
		}
	
	/* Compare-and-swap methods: */
	bool ifCompareAndSwap(Value testValue,Value newValue) // Compares value against testValue; if the same, set value to newValue; returns true if set was successful
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_bool_compare_and_swap(&value,testValue,newValue);
		#else
		Spinlock::Lock lock(mutex);
		if(value==testValue)
			{
			value=newValue;
			return true;
			}
		else
			return false;
		#endif
		}
	Value compareAndSwap(Value testValue,Value newValue) // Compares value against testValue; if the same, set value to newValue; returns previous value
		{
		#if THREADS_CONFIG_HAVE_BUILTIN_ATOMICS
		return __sync_val_compare_and_swap(&value,testValue,newValue);
		#else
		Spinlock::Lock lock(mutex);
		Value result=value;
		if(value==testValue)
			value=newValue;
		return result;
		#endif
		}
	};

}

#endif
