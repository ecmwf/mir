/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "LatLon.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Area_H
#include "Area.h"
#endif

#ifndef Tokenizer_H
#include "Tokenizer.h"
#endif

#include "DefinitionsCache.h"

#include <fstream>
#include <sstream>

static const double rad    =  4.0 * atan(1.0) / 180.0 ;
//static const double rad    =  2.0 * asin(1.0) / 180.0 ;

static const double radius =  6371220.0;

LatLon::LatLon(double ns, double we) 
		  : Grid(), ns_(ns), we_(we)
{
	if (!areValidIncrements())
        throw WrongIncrements(we,ns);
    
    // where area is not supplied, we set it to the extremes of the grid
    area_ = setGlobalArea();

	latitudesLocal(latitudes_);
	latitudesSize_ = latitudes_.size();
}

LatLon::LatLon(double north, double west, double south, double east, double ns, double we)
		  : Grid(north,west,south,east),ns_(ns), we_(we)
{
	if (!areValidIncrements())
        throw WrongIncrements(we,ns);
	latitudesLocal(latitudes_);
	latitudesSize_ = latitudes_.size();
}

LatLon::LatLon(const Area& other, double ns, double we) 
	  : Grid(other),ns_(ns), we_(we)
{
	if (!areValidIncrements())
        throw WrongIncrements(we,ns);
	latitudesLocal(latitudes_);
	latitudesSize_ = latitudes_.size();
}

LatLon::LatLon(double ns) 
		  : Grid(), ns_(ns), we_(0)
{
	ASSERT(ns);
	latitudesLocal(latitudes_);
	latitudesSize_ = latitudes_.size();
}

LatLon::LatLon(double north, double west, double south, double east, double ns)
		  : Grid(north,west,south,east),ns_(ns), we_(0)
{
	ASSERT(ns);
	latitudesLocal(latitudes_);
	latitudesSize_ = latitudes_.size();
}

LatLon::LatLon(const Area& other, double ns) 
	  : Grid(other),ns_(ns), we_(0)
{
	ASSERT(ns);
	latitudesLocal(latitudes_);
	latitudesSize_ = latitudes_.size();
}

// reduced
LatLon::LatLon(double north, double west, double south, double east, double ns, long nptsNS)
		  : Grid(north,west,south,east),ns_(ns), we_(0)
{
	ASSERT(nptsNS);
	double newns = calculateNortSouthIncrement(north,south,nptsNS);
	if(newns != ns_){
		cout << "LatLon::LatLon North-South increment reset from: " << ns_ << " to: " << newns << endl;
		ns_ = newns;
	}

	ASSERT(ns_);
}

LatLon::LatLon(const Area& other, double ns, long nptsNS) 
	  : Grid(other),ns_(ns), we_(0)
{
	ASSERT(nptsNS);
	double newns = calculateNortSouthIncrement(other.north(),other.south(),nptsNS);
	if(newns != ns_){
		cout << "LatLon::LatLon North-South increment reset from: " << ns_ << " to: " << newns << endl;
		ns_ = newns;
	}

	ASSERT(ns_);
}

LatLon::~LatLon() 
{ 
}

double LatLon::we() const 
{ 
	return we_; 
}

double LatLon::ns() const 
{ 
	return ns_; 
}

void LatLon::weightsY(vector<double>& weights) const
{
	int size  = northSouthNumberOfPoints();
    weights.resize(size);

	//double rr = ns_ * rad * radius;
	for(int i = 0; i < size; i++) {
		weights[i] = cos(latitudes_[i] * rad);
//		weights[i] =	(111.0*111.0)*1.e6 * cos(latitudes_[i])*(ns_*ns_);
//		weights[i] = rr;	
	}
}


bool LatLon::areValidIncrements() const 
{ 
	return (ns_ > 0) && (we_ > 0 || iszero(we_)); 
}

void LatLon::northHemisphereLatitudes(vector<double>& lats) const
{ 
	double lat = NORTH_POLE;

	while(lat > 0 || iszero(lat)) {
		lats.push_back(lat);
		lat -= ns_;
	}
}

double LatLon::calculateNortSouthIncrement(double north, double south, long npts) const
{ 
	ASSERT(npts);	
	return (north - south) / (npts - 1);
}

double LatLon::calculateWestEastIncrement(double east, double west, double inc, long npts) const
{ 
	ASSERT(npts);	
	double sum = east - west + inc + AREA_FACTOR;
	if( sum > EQUATOR || same(sum,EQUATOR))
        return  EQUATOR / npts;

	return (east - west) / (npts - 1);
}

double LatLon::calculateWestEastIncrement(long npts) const
{ 
	ASSERT(npts);	
	if(isGlobalWestEast())
        return  EQUATOR / npts;

	return (east() - west()) / (npts - 1);
}

void LatLon::latitudes(vector<double>& lats) const
{ 
	lats.clear();
	lats.reserve(latitudesSize_);
	for ( int i = 0 ; i < latitudesSize_ ; i++ ) {
		lats.push_back(latitudes_[i]);
//		cout.precision(15);
//		cout << latitudes_[i] << endl;
	}
}

void LatLon::latitudesLocal(vector<double>& lats) const
{
    lats.clear();
    int nsNumber  = northSouthNumberOfPoints();
    lats.reserve(nsNumber);
	double northLat = north();
    double lat = northLat;

    for ( int i = 0 ; i < nsNumber ; i++ ) {
        lats.push_back(lat);

// ssp accumulate RERR
        lat -= ns_;
//		lat = northLat - ns_ * (i+1);
    }
}

int LatLon::truncate(int truncationIn) const
{

	const string resol = getShareDir() + "/definitions/resolutions";
    ref_counted_ptr< const vector<string> > resolSpec = DefinitionsCache::get(resol);

    vector<string>::const_iterator it = resolSpec->begin();
	int truncationOut, gaussian;
	double llmin, llmax;
	double step = max(we_,ns_);


	if(DEBUG){
		cout << "LatLon::truncate In  : " << truncationIn << endl;
		cout << "LatLon::truncate step : " << step << endl;
	}

    while (it != resolSpec->end())
    {
        istringstream in(*it);
	    in >> truncationOut >> llmin >> llmax >> gaussian;

		if(llmin <= step && step < llmax){
			if(truncationOut > truncationIn)
				truncationOut = truncationIn;
				if(DEBUG)
					cout << "LatLon::truncate : " << truncationOut << endl;
			ASSERT(truncationOut);
			return truncationOut;
		}
        it++;
	}
	return truncationIn;
}

int LatLon::matchGaussian() const
{
	const string resol = getShareDir() + "/definitions/resolutions";
    ref_counted_ptr< const vector<string> > resolSpec = DefinitionsCache::get(resol);

    vector<string>::const_iterator it = resolSpec->begin();
	int truncationOut, gaussian;
	double llmin, llmax;
	double step = max(we_,ns_);

	if(DEBUG)
		cout << "LatLon::matchGaussian step : " << step << endl;

    while (it != resolSpec->end())
    {
        istringstream in(*it);
	    in >> truncationOut >> llmin >> llmax >> gaussian;
		if(llmin <= step && step <= llmax){
			if(DEBUG)
				cout << "LatLon::matchGaussian : " << gaussian << endl;
			return gaussian;
		}

        it++;
	}
	throw UserError("LatLon::matchGaussian ");
}


Area LatLon::fitToGlobalArea(double west) const
{
	if(same(west,0)){
		Area area(NORTH_POLE, 0, SOUTH_POLE, EQUATOR);
		return adjustArea(area,true,true);
	}
	else if (same(west,-180.0)) {
		Area area(NORTH_POLE, -180.0, SOUTH_POLE, 180.0);
		return adjustArea(area,true,true);
	}
	throw WrongValue("LatLon::fitToGlobalArea - Don't know how to fit global for west: ", west);
}

Area LatLon::setGlobalArea(double west) const
{
	if(getenv("ECREGRID_SHIFTED_GLOBAL_AREA")){
		double east = 0;
		if(west < 0 || same(west,0))
			east = west + EQUATOR - we_;
		else
			east = west - EQUATOR + we_;
		return Area(NORTH_POLE, west, SOUTH_POLE, east);
	}

	if (same(west,-180.0))
		return Area(NORTH_POLE, -180.0, SOUTH_POLE, 180.0 - we_);

	return Area(NORTH_POLE, 0, SOUTH_POLE, EQUATOR - we_);


}

Area LatLon::setGlobalArea() const
{
	return Area(NORTH_POLE, 0, SOUTH_POLE, EQUATOR - we_);
}

int LatLon::westEastNumberOfPoints() const
{
	if(isGlobalWestEast()){
		if(DEBUG)
			cout << "LatLon::westEastNumberOfPoints for GLOBAL West-East " << int(360.0 / we_  + AREA_FACTOR ) << endl;
		return int(EQUATOR / we_ + AREA_FACTOR );
	}

	if(DEBUG)
		cout << "LatLon::westEastNumberOfPoints: " << int((east() - west()) / we_ + AREA_FACTOR) + 1<< endl;

	return int((east() - west()) / we_ + AREA_FACTOR) + 1;
}

int LatLon::northSouthNumberOfPoints() const
{
	int npts = int(fabs( north() - south() ) / ns_ + AREA_FACTOR) + 1;
	if(DEBUG)
		cout << "LatLon::northSouthNumberOfPoints: " << npts << endl;

    return npts;
}

bool LatLon::isGlobalWestEast() const
{
//		cout << "LatLon::isGlobalWestEast " << east() << " west " << west() << " we_ " << we_<< "  ns_ " << ns_ <<  endl;
// Because of Reduced Lat-Lon
	double we = we_;
	if(!we)
		we = ns_;
	double res = east() - west() + we + AREA_FACTOR;	
	return (res > EQUATOR || same(res,EQUATOR));
}

bool LatLon::isGlobalNorthSouth() const
{
	return (same(north(), NORTH_POLE)) && (same(south(),SOUTH_POLE));
}

void LatLon::setGlobalNorthSouth(double& north, double& south) const
{
	north = NORTH_POLE; 
	south = SOUTH_POLE;
}

Area LatLon::adjustArea(const Area& area, bool globalWE, bool globalNS) const
{
	double west = area.west();
	double east = area.east();

	if(DEBUG)
		cout << "LatLon::adjustArea west: " << west << " east " << east << endl;

	if(globalWE)
		setGlobalWestEast(west,east,we_);	
	else
		adjustAreaWestEast(west,east,we_);

	double north = 0, south = 0;

	if(globalNS){
		setGlobalNorthSouth(north,south);
	}
	else{

		int	n = int(area.north() / ns_ + ROUNDING_FACTOR);
        
        // check we are within the area they requested
        // (works for both +ve and -ve values of n)
        while ( n * ns_ > area.north() && !same(n * ns_, area.north()))
            n--;

#if ECREGRID_EMOS_SIMULATION
        if (DEBUG)
            cout << "LatLon::adjustArea non-adjusted north of " << (n * ns_) << endl;
        // ensure we return the latitude outside the area specified
        while (n * ns_ < area.north() && !same(n * ns_, area.north()))
            n++;
#endif
		north = n * ns_;

		n = int((north - area.south()) / ns_ + ROUNDING_FACTOR);
#if ECREGRID_EMOS_SIMULATION
        // ensure we return the latitude outside the area specified
        while (n * ns_ < (north - area.south()) && !same(n * ns_, area.south()))
            n++;
#endif
		south = north - n * ns_;
	}

    if (DEBUG)
        cout << "LatLon::adjustArea adjusted area north = " << north << " west = " << west << " south = " << south << " east = " << east << endl;
   
	return Area(north,west,south,east);	
}

void LatLon::adjustAreaWestEastMars( double& west, double& east, double increment) const
{
	Grid::adjustAreaWestEastMars(west, east, increment);

//  Try to catch special case: global west-east with wrap-around.
	if (east - west > EQUATOR)
		east = west + EQUATOR - increment;
}

/*
Point LatLon::getPoint(int index) const
{
	// There must be a better algo
	double lat = south();
	double lon = west();
	int j =9;
	int i;
	
	for( i = 0 ; i < index ; i++)
	{
		lon += we_;
		if(lon >= east())
		{
			lat += ns_;
			lon = west();
		}
	}
	
	return Point(lat,lon);
}
*/

string LatLon::coeffInfo() const
{
	stringstream s;
	s <<"R" << ns_;

	return s.str();
}

size_t LatLon::loadGridSpec(vector<long>& rgSpec) const
{
	Tokenizer tokens("  ");
	string buffer;
	ifstream rgSource;

	stringstream s;
	s << getShareDir() << "/definitions/reduced_latlon/ll_" << ns_;
	const string path = s.str();

	rgSource.open(path.c_str());
	if (! rgSource)
	        throw CantOpenFile(path);

	size_t nlat = northSouthNumberOfPoints();
	rgSpec.clear();
	rgSpec.reserve(nlat);

	int i = 0;	
	while( getline(rgSource,buffer) ) {
		vector<string> v;
		tokens(buffer,v);
		int vsize = v.size();

		for ( int j = 0 ; j < vsize ; j++ ) {
			rgSpec.push_back(atol(v[j].c_str()));
			++i;
		}
	}
	rgSource.close();
	return nlat;
}

int LatLon::poleEquatorNumberOfPoints() const
{
	return 0;
//	throw NotImplementedFeature("LatLon::poleEquatorNumberOfPoints -> Not Defined");
}

int LatLon::northIndex(double north) const
{
// Round north boundary to nearest Latitude southwards
    for ( int i = 0 ; i < latitudesSize_ ; i++ ) {

		if(same(north,latitudes_[i])) {
            return i;   
        }
            
		if(north > latitudes_[i] ) {
#if ECREGRID_EMOS_SIMULATION                
            return i > 0 ? i - 1 : 0;
#else
			return i;
#endif            
		}
	}
	
	throw WrongValue("LatLon::northIndex - North is not within Area",north);
}

int LatLon::southIndex(double south) const
{
// Round south boundary to nearest FieldPoint northwards
    for ( int i = 0 ; i < latitudesSize_ ; i++ ) {
		if(same(south,latitudes_[i]))
			return i ;
		else if(south > latitudes_[i])
        {
#if ECREGRID_EMOS_SIMULATION                
                return i;
#else
    			return i - 1;
#endif            
        }
	}
	throw WrongValue("LatLon::southIndex - South is not within Area",south);
}

void LatLon::dump() const
{
	double lat = south();
	double lon = west();

	cout << "     ";
	while(lon < east())
	{
		cout << std::setw(5) << lon << " ";
		lon += we_;
	}
	cout << endl;
	
	lon = west();
	
	cout << std::setw(4) << lat;
	
	while(lat <= north())
	{
//		cout << " " << std::setw(5) << data_[i++];
		lon += we_;
		if(lon >= east())
		{
			cout << endl;
			lat += ns_;
			if(lat <= north())
				cout << std::setw(4) << lat;
			lon = west();
		}
	}
}

void LatLon::print(ostream& out) const
{
	out << "LatLon{ " ; Grid::print(out); 
	out << ", NS increment=[" << ns_ << "], WE increment=[" << we_ << "] }";
}

string LatLon::constructLsmFilename(const string& stub) const
{
    // we have a fixed format for all latlon LSM filenames
    // with correct formatting of the increment arguments
        
    stringstream lsmFile;
    lsmFile << stub << format_increments_value(ns_) << "_" << format_increments_value(we_);
    return lsmFile.str();
}

