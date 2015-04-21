/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Area.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#include<algorithm>
static const double radius =  6371220.0;

Area::Area(double north, double west, double south, double east, int scanMode):
    north_(north), west_(west), south_(south), east_(east) {
    if (scanMode == 1) {
        check();
    } else {
        reOrderBoundaries(scanMode);
        check();
    }
}

Area::Area(double north, double west, double south, double east):
    north_(north), west_(west), south_(south), east_(east) {
    check();
}

Area::Area(const Area &other):
    north_(other.north_), west_(other.west_), south_(other.south_), east_(other.east_) {
    check();
}

Area::~Area() {
}

Area &Area::operator=(const Area &a) {
    if (this == &a)
        return *this;

    north_ = a.north_;
    south_ = a.south_;
    east_  = a.east_;
    west_  = a.west_;

    return *this;
}

bool Area::operator==(const Area &other) const {
    //	return east_ == other.east_ && west_ == other.west_ && north_ == other.north_ && south_ == other.south_;
    // this is due to gaussian latitudes
    //	return east_ == other.east_ && west_ == other.west_ && fabs(north_ - other.north_) < 0.001 && fabs( south_ - other.south_) < 0.001;

    return same(east_, other.east_) && same(west_, other.west_) && same(north_, other.north_) && same( south_, other.south_);
}

bool Area::isGlobalWestEast(double step) const {
    //  cout << " Area::isGlobalWestEast " <<  " west " << west_ << " east " << east_ << " step " << step <<  endl;

    double east = east_;
    if (east < 0)
        east += 360.0;

    double west = west_;
    if (west < 0)
        west += 360.0;

    return (east - west + step + AREA_FACTOR) >= 360.0;
}

double Area::north() const {
    return north_;
}

double Area::west()  const {
    return west_;
}

double Area::east()  const {
    return east_;
}

double Area::south() const {
    return south_;
}

void Area::reOrderBoundaries(int scMode) {
    if (scMode == 2) {
        double n = north_;
        north_ = south_;
        south_ = n;
    } else if (scMode == 3) {
        double w = west_;
        west_ = east_;
        east_ = w;
    } else if (scMode == 4) {
        double w = west_;
        west_ = east_;
        east_ = w;
        double n = north_;
        north_ = south_;
        south_ = n;
    }
}

void Area::check() {
    //	ASSERT (north_ <= 90.0);
    //	ASSERT (north_ >= -90.0);
    //	ASSERT (south_ <= 90.0);
    //	ASSERT (south_ >= -90.0);

    if (north_ > 90.0) {
        cout << "\t" << "North adjusted to 90.0" << endl;
        north_ = 90.0;
    }
    if (north_ < -90.0) {
        cout << "\t" << "North adjusted to -90.0" << endl;
        north_ = -90.0;
    }
    if (south_ > 90.0) {
        cout << "\t" << "South adjusted to 90.0" << endl;
        south_ = 90.0;
    }
    if (south_ < -90.0) {
        cout << "\t" << "South adjusted to -90.0" << endl;
        south_ = -90.0;
    }

    ASSERT (west_  <= 360.0);
    ASSERT (west_  >= -360.0);


    ASSERT (east_  <= 360.0);
    ASSERT (east_  >= -360.0);

    //  cout << " Area: check " << north_ << " " << south_ << " " << west_ << " " << east_ << endl;
    ASSERT (north_ >= south_);

    /* ASSERT (east_  != west_); */
    /* ASSERT ((east_ - west_) < 360.0); */

    /*
     Standardise range:
     1) West and East are >= 0
     2) East > West.
    */
    /*
       	if (west_ <= 0)
       	{
       		west_ = west_ + 360.0;
       		east_ = east_ + 360.0;
       	}
    */

    //   	if(west_ >= east_)
    // 		east_ = east_ + 360.0;

    if (west_ > 0 && west_ > east_) {
        //		if(DEBUG)
        //			cout << "Area::check CASE west > 0 and  west_ > east_ " << endl;
        west_ = west_ - 360.0;
    }

    // Input west 0 east -1.875 for global field
    if (iszero(west_) && 0 > east_) {
        if (DEBUG)
            cout << "Area::check CASE west = 0 east < 0" << endl;
        east_ = east_ + 360.0;
    }
}

bool Area::empty() const {
    return iszero(north_) && iszero(west_) && iszero(south_) && iszero(east_);
}

bool Area::isOnePoint() const {
    if (iszero(north_) && iszero(west_) && iszero(south_) && iszero(east_))
        return false;

    return same(north_, south_) && same(east_, west_);
}

bool Area::northernHemisphereOnly() const {
    return north_ > 0 && south_ >= 0;
}

bool Area::southernHemisphereOnly() const {
    return north_ > 0 && south_ >= 0;
}

bool Area::symmetricalAboutEquator() const {
    return iszero(north_ + south_);
}

bool Area::moreNorthThanSouth() const {
    return north_ > -south_ ;
}

bool Area::moreSouthThanNorth() const {
    return north_ < -south_ ;
}


Area Area::join(const Area &other) const {
    return Area(std::max(north_, other.north_), std::min(west_, other.west_), std::min(south_, other.south_), std::max(east_, other.east_));
}

Area Area::intersection(const Area &other) const {
    double n = std::min(north_, other.north_);
    double w = std::max(west_, other.west_);
    double s = std::max(south_, other.south_);
    double e = std::min(east_, other.east_);


    //	double  s = south_ < other.south_ ? south_:other.south_;
    //  cout << " Area: intersection" << n << " " << s << " " << w << " " << e << endl;
    //	if(n < s || w > e)
    //		return Area(0,0,0,0);

    return  Area(n, w, s, e);
}

bool Area::intersect(const Area &other) const {
    //	return !intersection(other).empty();

    return (north_ > other.south_ || other.north_ > south_) && ((east_ > other.west_) || (east_ > other.east_ && west_ > other.west_));

}

bool Area::isCellCentered(double ns, double we) const {
    /*
    	if(DEBUG){
    		cout << " Area::isCellCentered north: " << north_ << "  south: " << south_ << endl;
    		cout << " Area::isCellCentered west: " << west_ << "  east: " << east_ << endl;
    	}
    */

    if (same(fabs(north_), fabs(south_)) && fabs(north_) < 90.0) {
        if (DEBUG) {
            if ((north_ + ns / 2 + AREA_FACTOR >= 90.0) && (east() - west() + we + AREA_FACTOR >= 360.0))
                cout << " Area::isCellCentered : YES" << endl;
            else
                cout << " Area::isCellCentered : NO" << endl;
        }
        return (north_ + ns / 2 + AREA_FACTOR >= 90.0) && (east() - west() + we + AREA_FACTOR >= 360.0);
    }
    return false;
}

bool Area::contain(const Area &other) const {
    //	return intersection(other) == *this;
    //  cout << " Area::contain " << withinWestEast(other) << " " << withinNorthSouth(other) << endl;

    return withinWestEast(other) && withinNorthSouth(other);
}

bool Area::contain(const Area &other, bool globalWestEast, bool globalNorthSouth) const {
    //	return intersection(other) == *this;
    //  cout << " Area::contain " << withinWestEast(other) << " " << withinNorthSouth(other) << endl;
    if (globalWestEast && globalNorthSouth)
        return true;
    if (globalWestEast)
        return withinNorthSouth(other);

    if (globalNorthSouth)
        return withinWestEast(other);

    return withinWestEast(other) && withinNorthSouth(other);
}

bool Area::issame(const Area &other) const {
    //  cout << " Area::same " << east_ << " " << other.east_ << " " << west_ << " " << other.west_ << " " << north_ << " " << other.north_ << " " << south_ << " " << other.south_ << endl;
    //  cout << " Area::same " << south_ << " " << other.south_ << endl;
    //	return south_ == other.south_;
    return same(east_, other.east_) && same(west_, other.west_) && same(north_, other.north_) && same(south_, other.south_);
}

bool Area::isPointWithinArea(const Point &p) const {
    double lon = p.longitude();
    double lat = p.latitude();

    double west = west_;
    if (west_ < 0)
        west = west_ + 360.0;

    if ((lat < north_ || same(lat, north_)) && (lat > south_ || same(lat, south_))) {
        //		if((lon > west || same(lon,west)) && (lon < east_ || same(lon,east_)))
        if ((lon > west || same(lon, west)))
            return true;
        if ((lon < east_ || same(lon, east_)))
            return true;
        /*
        else {
        	double t = lon - 360;
        	if(t <= east_ && t >= west_)
        	return true;
        }
        */
        //	cout << "Area::isPointWithinArea => Area west " << west_ << " east " << east_ << " " << north_ << " " << south_ << endl ;
        //	cout << "Area::isPointWithinArea => Point lon " << p.longitude() << " lat " << p.latitude() << endl ;
    }

    return false;

    //	return (p.longitude() >= west) && (p.longitude() <= east_) &&  (p.latitude() <= north_) && (p.latitude() >= south_);
}

bool Area::withinWestEast(const Area &other) const {
    //	return (west_ >= other.west_) || (east_ <= other.east_);
    // ssp not good yet
    double w = west_;
    if (getenv("LIST_AREA")) {
        if (west_ < 0  && (east_ - west_) < (other.east_ - other.west_))
            w = 360.0 - west_;
    }

    return (other.west_ <= w) && (east_ <= other.east_);
}

bool Area::withinNorthSouth(const Area &other) const {
    //  cout << " Area::withinNorthSouth SOUTH " << south_ << " " << other.south_ << endl;

    return south_ >= other.south_ && north_ <= other.north_;
}

double Area::size() const {
    return 2 * M_PI * radius * (sin(north_) - sin(south_)) * ((fabs(east_ - west_)) / 360);
}


void Area::print(ostream &out) const {
    out << "Area{ north=[" << north_ << "], west=[" << west_ <<  "], east=[" << east_ << "], south=[" << south_ << "] }";
}
