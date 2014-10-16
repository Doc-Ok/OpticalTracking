/***********************************************************************
Time - Wrapper classes for absolute and relative time measured from one
of a variety of POSIX clocks.
Copyright (c) 2014 Oliver Kreylos

This file is part of the Realtime Processing Library (Realtime).

The Realtime Processing Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Realtime Processing Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Realtime Processing Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#ifndef REALTIME_TIME_INCLUDED
#define REALTIME_TIME_INCLUDED

#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <Realtime/Config.h>

namespace Realtime {

class Time:public timespec // Base class for absolute time points or relative time vectors
	{
	/* Protected methods: */
	protected:
	void add(time_t otherSec,long otherNsec) // Adds a time point and a time vector or two time vectors
		{
		tv_sec+=otherSec;
		tv_nsec+=otherNsec;
		if(tv_nsec>=1000000000L)
			{
			++tv_sec;
			tv_nsec-=1000000000L;
			}
		}
	void subtract(time_t otherSec,long otherNsec) // Subtracts two compatible time points, a time point and a time vector, or two time vectors
		{
		tv_sec-=otherSec;
		tv_nsec-=otherNsec;
		if(tv_nsec<0L)
			{
			--tv_sec;
			tv_nsec+=1000000000L;
			}
		}
	#if !REALTIME_CONFIG_HAVE_POSIX_CLOCKS
	static void getCurrentTime(timespec* time) // Emulates clock_gettime function
		{
		timeval currentTime;
		gettimeofday(&currentTime,0);
		time->tv_sec=currentTime.tv_sec;
		time->tv_nsec=currentTime.tv_usec*1000L;
		}
	static void sleepUntil(const Time& wakeupTime) // Emulates clock_nanosleep function
		{
		while(true)
			{
			/* Get the current time: */
			timeval current;
			gettimeofday(&current,0);
			
			/* Calculate the sleep interval: */
			Time sleep(wakeupTime);
			sleep.subtract(current.tv_sec,current.tv_usec*1000L);
			
			/* Check if already passed: */
			if(sleep.tv_sec<0)
				break;
			
			/* Sleep for a while: */
			nanosleep(&sleep,0);
			}
		}
	#endif
	
	/* Constructors and destructors: */
	public:
	Time(void) // Creates uninitialized time point
		{
		}
	Time(const timespec& source) // Creates time point or vector from timespec structure
		{
		tv_sec=source.tv_sec;
		tv_nsec=source.tv_nsec;
		}
	Time(time_t sSec,long sNsec) // Creates time point or vector from given number of seconds and nanoseconds
		{
		tv_sec=sSec;
		tv_nsec=sNsec;
		if(tv_nsec<0L)
			{
			--tv_sec;
			tv_nsec+=1000000000L;
			}
		if(tv_nsec>=1000000000L)
			{
			++tv_sec;
			tv_nsec-=1000000000L;
			}
		}
	Time(double sSec) // Creates time point or vector from number of whole and fractional seconds
		{
		double wholeSec=floor(sSec);
		tv_sec=time_t(wholeSec);
		tv_nsec=long(floor((sSec-wholeSec)*1.0e9+0.5));
		}
	
	/* Methods: */
	operator timeval(void) const // Converts time object to struct timeval with microsecond resolution
		{
		timeval result;
		result.tv_sec=tv_sec;
		result.tv_usec=(tv_nsec+500L)/1000L;
		if(result.tv_usec>=1000000)
			{
			++result.tv_sec;
			result.tv_usec-=1000000;
			}
		return result;
		}
	operator double(void) const // Converts time object to number of whole and fractional seconds
		{
		return double(tv_sec)+double(tv_nsec)*1.0e-9;
		}
	};

class TimeVector:public Time // Class for relative time vectors
	{
	/* Constructors and destructors: */
	public:
	TimeVector(void) // Creates uninitialized time vector
		{
		}
	explicit TimeVector(const timespec& source) // Creates time vector from timespec structure
		:Time(source)
		{
		}
	TimeVector(time_t sSec,long sNsec) // Creates time vector from given number of seconds and nanoseconds
		:Time(sSec,sNsec)
		{
		}
	TimeVector(double sSec) // Creates time vector from number of whole and fractional seconds
		:Time(sSec)
		{
		}
	
	/* Methods: */
	bool operator<(const TimeVector& other) const
		{
		return tv_sec<other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec<other.tv_nsec);
		}
	bool operator<=(const TimeVector& other) const
		{
		return tv_sec<other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec<=other.tv_nsec);
		}
	bool operator>=(const TimeVector& other) const
		{
		return tv_sec>other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec>=other.tv_nsec);
		}
	bool operator>(const TimeVector& other) const
		{
		return tv_sec>other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec>other.tv_nsec);
		}
	TimeVector& operator+=(const TimeVector& other) // Adds two time vectors
		{
		add(other.tv_sec,other.tv_nsec);
		return *this;
		}
	TimeVector& operator-=(const TimeVector& other) // Subtracts two time vectors
		{
		subtract(other.tv_sec,other.tv_nsec);
		return *this;
		}
	};

class TimePointRealtime:public Time // Class for absolute time points in seconds since the epoch (January 1st, 1970, 12:00 midnight)
	{
	/* Constructors and destructors: */
	public:
	TimePointRealtime(void) // Creates time point by sampling the realtime clock
		{
		/* Get the current time: */
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		clock_gettime(CLOCK_REALTIME,this);
		#else
		getCurrentTime(this);
		#endif
		}
	explicit TimePointRealtime(const timespec& source) // Creates time point from timespec structure
		:Time(source)
		{
		}
	TimePointRealtime(time_t sSec,long sNsec) // Creates time point from given number of seconds and nanoseconds
		:Time(sSec,sNsec)
		{
		}
	TimePointRealtime(double sSec) // Creates time point from number of whole and fractional seconds
		:Time(sSec)
		{
		}
	
	/* Methods: */
	TimePointRealtime& set(void) // Sets time point by sampling the realtime clock
		{
		/* Get the current time: */
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		clock_gettime(CLOCK_REALTIME,this);
		#else
		getCurrentTime(this);
		#endif
		
		return *this;
		}
	TimeVector setAndDiff(void) // Sets time point by sampling the realtime clock and returns time difference from last value
		{
		/* Get the current time: */
		timespec current;
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		clock_gettime(CLOCK_REALTIME,&current);
		#else
		getCurrentTime(&current);
		#endif
		
		/* Calculate the difference to the previous time point: */
		TimeVector result(current.tv_sec-tv_sec,current.tv_nsec-tv_nsec);
		
		/* Update this time point: */
		tv_sec=current.tv_sec;
		tv_nsec=current.tv_nsec;
		
		/* Return the difference vector: */
		return result;
		}
	bool operator<(const TimePointRealtime& other) const
		{
		return tv_sec<other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec<other.tv_nsec);
		}
	bool operator<=(const TimePointRealtime& other) const
		{
		return tv_sec<other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec<=other.tv_nsec);
		}
	bool operator>=(const TimePointRealtime& other) const
		{
		return tv_sec>other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec>=other.tv_nsec);
		}
	bool operator>(const TimePointRealtime& other) const
		{
		return tv_sec>other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec>other.tv_nsec);
		}
	TimePointRealtime& operator+=(const TimeVector& other) // Adds a time vector to a time point
		{
		add(other.tv_sec,other.tv_nsec);
		return *this;
		}
	TimePointRealtime& operator-=(const TimeVector& other) // Subtracts a time vector from a time point
		{
		subtract(other.tv_sec,other.tv_nsec);
		return *this;
		}
	static void sleep(const TimePointRealtime& wakeupTime) // Suspends calling thread until realtime clock reaches given time point
		{
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		/* Keep calling clock_nanosleep until the wake-up time has definitely passed: */
		while(clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,&wakeupTime,0))
			;
		#else
		sleepUntil(wakeupTime);
		#endif
		}
	static void sleep(const TimeVector& sleepTime) // Suspends calling thread until the realtime clock has advanced by the given amount from current time
		{
		/* Calculate the wake-up time: */
		TimePointRealtime wakeupTime;
		wakeupTime+=sleepTime;
		
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		/* Keep calling clock_nanosleep until the wake-up time has definitely passed: */
		while(clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,&wakeupTime,0))
			;
		#else
		sleepUntil(wakeupTime);
		#endif
		}
	void sleepFromCurrent(const TimeVector& sleepTime) const // Suspends calling thread until the realtime clock has advanced by the given amount from the time point
		{
		/* Calculate the wake-up time: */
		TimePointRealtime wakeupTime=*this;
		wakeupTime+=sleepTime;
		
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		/* Keep calling clock_nanosleep until the wake-up time has definitely passed: */
		while(clock_nanosleep(CLOCK_REALTIME,TIMER_ABSTIME,&wakeupTime,0))
			;
		#else
		sleepUntil(wakeupTime);
		#endif
		}
	};

class TimePointMonotonic:public Time // Class for absolute time points in seconds since an unspecified time in the past
	{
	/* Constructors and destructors: */
	public:
	TimePointMonotonic(void) // Creates time point by sampling the monotonic clock
		{
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		clock_gettime(CLOCK_MONOTONIC,this);
		#else
		getCurrentTime(this);
		#endif
		}
	explicit TimePointMonotonic(const timespec& source) // Creates time point from timespec structure
		:Time(source)
		{
		}
	TimePointMonotonic(time_t sSec,long sNsec) // Creates time point from given number of seconds and nanoseconds
		:Time(sSec,sNsec)
		{
		}
	TimePointMonotonic(double sSec) // Creates time point from number of whole and fractional seconds
		:Time(sSec)
		{
		}
	
	/* Methods: */
	TimePointMonotonic& set(void) // Sets time point by sampling the monotonic clock
		{
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		clock_gettime(CLOCK_MONOTONIC,this);
		#else
		getCurrentTime(this);
		#endif
		
		return *this;
		}
	TimeVector setAndDiff(void) // Sets time point by sampling the monotonic clock and returns time difference from last value
		{
		/* Get the current time: */
		timespec current;
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		clock_gettime(CLOCK_MONOTONIC,&current);
		#else
		getCurrentTime(&current);
		#endif
		
		/* Calculate the difference to the previous time point: */
		TimeVector result(current.tv_sec-tv_sec,current.tv_nsec-tv_nsec);
		
		/* Update this time point: */
		tv_sec=current.tv_sec;
		tv_nsec=current.tv_nsec;
		
		/* Return the difference vector: */
		return result;
		}
	bool operator<(const TimePointMonotonic& other) const
		{
		return tv_sec<other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec<other.tv_nsec);
		}
	bool operator<=(const TimePointMonotonic& other) const
		{
		return tv_sec<other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec<=other.tv_nsec);
		}
	bool operator>=(const TimePointMonotonic& other) const
		{
		return tv_sec>other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec>=other.tv_nsec);
		}
	bool operator>(const TimePointMonotonic& other) const
		{
		return tv_sec>other.tv_sec||(tv_sec==other.tv_sec&&tv_nsec>other.tv_nsec);
		}
	TimePointMonotonic& operator+=(const TimeVector& other) // Adds a time vector to a time point
		{
		add(other.tv_sec,other.tv_nsec);
		return *this;
		}
	TimePointMonotonic& operator-=(const TimeVector& other) // Subtracts a time vector from a time point
		{
		subtract(other.tv_sec,other.tv_nsec);
		return *this;
		}
	static void sleep(const TimePointMonotonic& wakeupTime) // Suspends calling thread until monotonic clock reaches given time point
		{
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		/* Keep calling clock_nanosleep until the wake-up time has definitely passed: */
		while(clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&wakeupTime,0))
			;
		#else
		sleepUntil(wakeupTime);
		#endif
		}
	static void sleep(const TimeVector& sleepTime) // Suspends calling thread until the realtime clock has advanced by the given amount
		{
		/* Calculate the wake-up time: */
		TimePointMonotonic wakeupTime;
		wakeupTime+=sleepTime;
		
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		/* Keep calling clock_nanosleep until the wake-up time has definitely passed: */
		while(clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&wakeupTime,0))
			;
		#else
		sleepUntil(wakeupTime);
		#endif
		}
	void sleepFromCurrent(const TimeVector& sleepTime) const // Suspends calling thread until the monotonic clock has advanced by the given amount from the time point
		{
		/* Calculate the wake-up time: */
		TimePointMonotonic wakeupTime=*this;
		wakeupTime+=sleepTime;
		
		#if REALTIME_CONFIG_HAVE_POSIX_CLOCKS
		/* Keep calling clock_nanosleep until the wake-up time has definitely passed: */
		while(clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&wakeupTime,0))
			;
		#else
		sleepUntil(wakeupTime);
		#endif
		}
	};

/*************************************
Operations on time points and vectors:
*************************************/

inline TimePointRealtime operator+(const TimePointRealtime& t1,const TimeVector& t2)
	{
	return TimePointRealtime(t1.tv_sec+t2.tv_sec,t1.tv_nsec+t2.tv_nsec);
	}

inline TimePointRealtime operator+(const TimeVector& t1,const TimePointRealtime& t2)
	{
	return TimePointRealtime(t1.tv_sec+t2.tv_sec,t1.tv_nsec+t2.tv_nsec);
	}

inline TimePointRealtime operator-(const TimePointRealtime& t1,const TimeVector& t2)
	{
	return TimePointRealtime(t1.tv_sec-t2.tv_sec,t1.tv_nsec-t2.tv_nsec);
	}

inline TimeVector operator-(const TimePointRealtime& t1,const TimePointRealtime& t2)
	{
	return TimeVector(t1.tv_sec-t2.tv_sec,t1.tv_nsec-t2.tv_nsec);
	}

inline TimePointMonotonic operator+(const TimePointMonotonic& t1,const TimeVector& t2)
	{
	return TimePointMonotonic(t1.tv_sec+t2.tv_sec,t1.tv_nsec+t2.tv_nsec);
	}

inline TimePointMonotonic operator+(const TimeVector& t1,const TimePointMonotonic& t2)
	{
	return TimePointMonotonic(t1.tv_sec+t2.tv_sec,t1.tv_nsec+t2.tv_nsec);
	}

inline TimePointMonotonic operator-(const TimePointMonotonic& t1,const TimeVector& t2)
	{
	return TimePointMonotonic(t1.tv_sec-t2.tv_sec,t1.tv_nsec-t2.tv_nsec);
	}

inline TimeVector operator-(const TimePointMonotonic& t1,const TimePointMonotonic& t2)
	{
	return TimeVector(t1.tv_sec-t2.tv_sec,t1.tv_nsec-t2.tv_nsec);
	}

inline TimeVector operator+(const TimeVector& t1,const TimeVector& t2)
	{
	return TimeVector(t1.tv_sec+t2.tv_sec,t1.tv_nsec+t2.tv_nsec);
	}

inline TimeVector operator-(const TimeVector& t1,const TimeVector& t2)
	{
	return TimeVector(t1.tv_sec-t2.tv_sec,t1.tv_nsec-t2.tv_nsec);
	}

}

#endif
