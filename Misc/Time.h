/***********************************************************************
Time - Wrapper class for time specifications used for thread and signal
system calls. Time objects can either be thought of as absolute times
(time points) or time differences (time vectors). The class makes no
distinction between the two. Which is bad, because some system calls
take time point arguments, others take time vector arguments. Maybe in
the next version...
Copyright (c) 2005-2009 Oliver Kreylos

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

#ifndef MISC_TIME_INCLUDED
#define MISC_TIME_INCLUDED

#include <math.h>
#include <time.h>
#include <sys/time.h>

namespace Misc {

class Time:public timespec
	{
	/* Constructors and destructors: */
	public:
	Time(void) // Creates uninitialized time object
		{
		}
	Time(long seconds,long nanoseconds) // Creates time vector from given number of seconds and nanoseconds
		{
		/* Precondition: 0 <= nanoseconds < 1000000000 */
		tv_sec=seconds;
		tv_nsec=nanoseconds;
		}
	Time(double seconds) // Creates time vector from given number of seconds
		{
		double sec=floor(seconds);
		tv_sec=long(sec);
		tv_nsec=long((seconds-sec)*1.0e9);
		}
	static Time now(void) // Creates time point from current time
		{
		struct timeval tod;
		gettimeofday(&tod,0);
		return Time(tod.tv_sec,tod.tv_usec*1000);
		}
	
	/* Methods: */
	operator struct timeval(void) const // Converts time object to struct timeval
		{
		struct timeval result;
		result.tv_sec=tv_sec;
		result.tv_usec=tv_nsec/1000;
		return result;
		}
	Time& increment(long seconds,long nanoseconds) // Increments time object by given values
		{
		/* Precondition: 0 <= nanoseconds < 1000000000 */
		tv_sec+=seconds;
		tv_nsec+=nanoseconds;
		if(tv_nsec>=1000000000)
			{
			++tv_sec;
			tv_nsec-=1000000000;
			}
		return *this;
		}
	Time& increment(double seconds) // Increments time object from given number of seconds
		{
		double sec=floor(seconds);
		tv_sec+=long(sec);
		tv_nsec+=long((seconds-sec)*1.0e9);
		if(tv_nsec>=1000000000)
			{
			++tv_sec;
			tv_nsec-=1000000000;
			}
		return *this;
		}
	Time& operator+=(const Time& other) // Adds another time object
		{
		tv_sec+=other.tv_sec;
		tv_nsec+=other.tv_nsec;
		if(tv_nsec>=1000000000)
			{
			++tv_sec;
			tv_nsec-=1000000000;
			}
		return *this;
		}
	Time& operator-=(const Time& other) // Subtracts another time object
		{
		tv_sec-=other.tv_sec;
		if(tv_nsec<other.tv_nsec)
			{
			--tv_sec;
			tv_nsec+=1000000000;
			}
		tv_nsec-=other.tv_nsec;
		return *this;
		}
	};

inline bool operator==(const Time& t1,const Time& t2) // Equality operator
	{
	return t1.tv_sec==t2.tv_sec&&t1.tv_nsec==t2.tv_nsec;
	}

inline bool operator!=(const Time& t1,const Time& t2) // Inequality operator
	{
	return t1.tv_sec!=t2.tv_sec||t1.tv_nsec!=t2.tv_nsec;
	}

inline bool operator<(const Time& t1,const Time& t2) // Less-than operator
	{
	return t1.tv_sec<t2.tv_sec||(t1.tv_sec==t2.tv_sec&&t1.tv_nsec<t2.tv_nsec);
	}

inline bool operator<=(const Time& t1,const Time& t2) // Less-than-or-equal operator
	{
	return t1.tv_sec<t2.tv_sec||(t1.tv_sec==t2.tv_sec&&t1.tv_nsec<=t2.tv_nsec);
	}

inline bool operator>=(const Time& t1,const Time& t2) // Larger-than-or-equal operator
	{
	return t1.tv_sec>t2.tv_sec||(t1.tv_sec==t2.tv_sec&&t1.tv_nsec>=t2.tv_nsec);
	}

inline bool operator>(const Time& t1,const Time& t2) // Larger-than operator
	{
	return t1.tv_sec>t2.tv_sec||(t1.tv_sec==t2.tv_sec&&t1.tv_nsec>t2.tv_nsec);
	}

inline Time operator+(const Time& t1,const Time& t2) // Returns sum of two time objects
	{
	Time result(t1);
	result+=t2;
	return result;
	}

inline Time operator-(const Time& t1,const Time& t2) // Returns difference between two time objects
	{
	Time result(t1);
	result-=t2;
	return result;
	}

/*********************************
Functions on objects of type Time:
*********************************/

inline void sleep(const Time& time) // Blocks until at least the given amount of time has passed
	{
	Time remaining=time;
	while(nanosleep(&remaining,&remaining)<0)
		;
	}

}

#endif
