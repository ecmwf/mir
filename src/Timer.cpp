/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Timer_H
#include "Timer.h"
#endif

#ifndef Seconds_H
#include "Seconds.h"
#endif

#include <string>
#include <iostream>
using std::string;
using std::endl;
using std::ostream;

timeval operator-(const timeval& a,const timeval& b)
{
	timeval diff;

    diff.tv_sec  = a.tv_sec  - b.tv_sec;
    diff.tv_usec = a.tv_usec - b.tv_usec;

    if (diff.tv_usec < 0)
    {
          diff.tv_sec--;
          diff.tv_usec += 1000000;
    }

	return diff;
}


Timer::Timer(const string& name,ostream& out):
    name_(name),
    cpu_(clock()),
    out_(out)
{
    gettimeofday(&start_,0);
}

Timer::~Timer()
{
    double  s   = elapsed();
    clock_t cpu =  clock();
    out_ << name_ << ": " 
		 << Seconds(s) << " elapsed, " 
		 << Seconds(double(cpu-cpu_)/CLOCKS_PER_SEC) 
		 << " cpu" << endl;
}

double Timer::elapsed()
{
    timeval stop;
    gettimeofday(&stop,0);  
    timeval diff = stop - start_;
    return (double)diff.tv_sec + ((double)diff.tv_usec / 1000000.);
}
