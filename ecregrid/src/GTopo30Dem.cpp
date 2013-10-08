/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GTopo30Dem.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#include <fstream>
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


inline long nearestLL(double x) { return long(x * 1000.0 + 0.5); }
inline long pixel(long   p) { if(p > 32000) p -= 65536; return p; }

GTopo30Dem::GTopo30Dem(const string& path,const string& name,double minLatitude,double maxLatitude,double minLongitude,double maxLongitude):
	path_(path),
	name_(name),
	fd_(-1),
	minLatitude_(minLatitude),
	maxLatitude_(maxLatitude),
	minLongitude_(minLongitude),
	maxLongitude_(maxLongitude)
{
}


GTopo30Dem::~GTopo30Dem()
{
	if(fd_>=0) ::close(fd_);
}


long GTopo30Dem::value(double latitude,double longitude) const
{
	if(fd_ < 0) const_cast<GTopo30Dem*>(this)->open();


	long  lat = nearestLL(latitude);
	long  lon = nearestLL(longitude);

	double xz1 = ulxmap_ - xdim_/2;
	double xz2 = ulxmap_ + xdim_/2;
	long xx = 0;

	while( ! (nearestLL(xz1) <= lon && lon <= nearestLL(xz2)))  
	{
		xz1 += xdim_;
		xz2 += xdim_;
		xx++;
	}

	double yz1 = ulymap_ - ydim_/2;
	double yz2 = ulymap_ + ydim_/2;
	long yy = 0;

	while( ! (nearestLL(yz1) <= lat && lat <= nearestLL(yz2)))  
	{
		yz1 -= ydim_;
		yz2 -= ydim_;
		yy++;
	}

	long pos = xx + yy * ncols_;

	if(::lseek(fd_,pos*2,SEEK_SET) < 0)
    	throw ReadError(name_);

	unsigned char px[2];	
	if(::read(fd_,px,2) != 2)
    	throw ReadError(name_);

	long p = px[0] << 8 | px[1];

	return pixel(p);

}

void GTopo30Dem::open()
{
	string hdr = path_ + "/" + name_ + ".HDR";
	ifstream in(hdr.c_str());
	if(!in) 
		throw CantOpenFile("GTopo30Dem::open " + hdr);
				 	

	map<string,string> m;
	string name,value;

	while(in >> name >> value) {
		m[name] = value;
	}

	ncols_  = atol(m["NCOLS"].c_str());

	ulxmap_ = atof(m["ULXMAP"].c_str());
	ulymap_ = atof(m["ULYMAP"].c_str());
	xdim_   = atof(m["XDIM"].c_str());
	ydim_   = atof(m["YDIM"].c_str());

	/*

	TODO:

	assert( BYTEORDER == M)
	assert( LAYOUT == BIL)
	assert( NBANDS = 1)
	assert(NBITS == 16)
	assert(NODATA == -9999)
	...


	*/


	string dem = path_ + "/" + name_ + ".DEM";
	fd_ = ::open(dem.c_str(),O_RDONLY);
	if(fd_ < 0)
		throw CantOpenFile("GTopo30Dem::open " + dem);
}
