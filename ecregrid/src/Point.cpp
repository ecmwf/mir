/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Point.h"

#ifndef Exception_H
#include "Exception.h"
#endif

inline double deg2rad(double x) {
    return x/180.0*M_PI;
}
inline double rad2deg(double x) {
    return x*180.0/M_PI;
}

Point::Point(const Point& p):
    latitude_(p.latitude_), longitude_(p.longitude_), iIndex_(p.iIndex_), jIndex_(p.jIndex_), k1dIndex_(p.k1dIndex_) {
}

Point::Point(double latitude, double longitude, int i, int j, long k):
    latitude_(latitude), longitude_(longitude), iIndex_(i), jIndex_(j), k1dIndex_(k) {
//	cout << "Point::Point latlon  latitude "  << latitude << " longitude " << longitude << endl;
}

Point::Point(double x, double y, double z, int i, int j, long k):
    latitude_(asin(z)*FACTOR2), longitude_(fabs(x) > 0.0001 ? atan2(y,x)*FACTOR2 : (y > 0 ? 90.0 : -90.0)), iIndex_(i), jIndex_(j), k1dIndex_(k) {
//	cout << "Point::Point latlon  xyz latitude: "  << latitude_ << " longitude: " << longitude_ << endl;
}

Point::~Point() {
}

Point& Point::operator=(const Point& p) {
    if (this == &p)
        return *this;

    latitude_    = p.latitude_;
    longitude_   = p.longitude_;

    iIndex_  = p.iIndex_;
    jIndex_  = p.jIndex_;
    k1dIndex_ = p.k1dIndex_;

    return *this;
}

double Point::x() const {
    return cos(latitude_ * FACTOR1) * cos(longitude_ * FACTOR1);
}

double Point::y() const {
    return cos(latitude_ * FACTOR1) * sin(longitude_ * FACTOR1);
}

double Point::z() const {
    return sin(latitude_ * FACTOR1);
}

double Point::unnormalisedWeight(const Point& oposite) const {
    return fabs(oposite.latitude() - latitude_) * fabs(oposite.longitude() - longitude_) ;
}

Point Point::checkup() const {
    ASSERT (latitude_ <= 90.0);
    ASSERT (latitude_ >= -90.0);
    ASSERT (longitude_  <= 360.0);
    ASSERT (longitude_  >= -360.0);

    if(longitude_ < 0)
        return Point(latitude_,longitude_ + 360.0,iIndex_,jIndex_,k1dIndex_);
    return *this;
}

double Point::quickDistance(const Point& other) const {
    return (latitude_ - other.latitude_)   * (latitude_ - other.latitude_)  +
           (longitude_ - other.longitude_) * (longitude_ - other.longitude_);
}

double Point::euclidianDistance(const Point& other) const {
    return sqrt(quickDistance(other));
}

double Point::sphericalDistance(const Point& other) const {
    double radius = 6371.0;
    double tet1   = 90.0 - latitude_;
    double tet2   = 90.0 - other.latitude_;

    double a = cos(longitude_ - other.latitude_);
    double b = sin(tet1) * sin(tet2);
    double c = cos(tet1) * cos(tet2);

    double cosal = a * b + c;
    cosal = max(-1.0, min(cosal,1.0));

    return fabs(radius * acos(cosal));
}

double Point::earthDistance(const Point& other) const {
    double lat1 = deg2rad(latitude_);
    double lon1 = deg2rad(longitude_);
    double lat2 = deg2rad(other.latitude_);
    double lon2 = deg2rad(other.longitude_);

    return acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon2-lon1));
}


double Point::longitudesDistance(const Point& other) const {
    if (longitude_ < other.longitude_)
        return longitude_ + 360.0 - other.longitude_;

    return longitude_ - other.longitude_;
}

double Point::earthDistanceInKM(const Point& other) const {
    const double R = 180*60 / M_PI * 1.852;
    return earthDistance(other) * R;
}

double Point::angle(const Point& other) const {
    const double epsilon = 1e-7;
    double d = earthDistance(other);

    if(d < epsilon)
        return 0;

    double lat1 = deg2rad(latitude_);
    if(cos(lat1) < epsilon) {
        if(lat1>0)
            return M_PI; // North pole
        else
            return 2*M_PI; // South pole
    }

    double lon1 = deg2rad(longitude_);
    double lat2 = deg2rad(other.latitude_);
    double lon2 = deg2rad(other.longitude_);

    double x =  acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1)));
    return (sin(lon1-lon2)<=0) ? x : (2*M_PI-x);

}

void Point::print(ostream& out) const {
    out << "Point{ lat=[" << setw(7) << latitude_ << ";" << "], lon=["<< setw(7) << longitude_ << "],  i=[" << iIndex_ << "], j=[" << jIndex_ << "] , k=[" << k1dIndex_ << "] }"  ;
}
