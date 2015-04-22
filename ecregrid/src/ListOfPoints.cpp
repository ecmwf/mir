/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "ListOfPoints.h"

#include "Exception.h"
#include "FieldPoint.h"
#include "Point.h"
#include "Area.h"
#include "Interpolator.h"
#include "Linear.h"
#include "NearestNeigbour.h"
#include "GridContext.h"
#include <eckit/log/Timer.h>

#include <queue>

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include <iterator>
#include <algorithm>




// Assumes that the scaning mode of the target grid will be north_south...

static bool north_south(const Point& a,const Point& b) {
    if(same(a.latitude(),b.latitude()))
        return a.longitude() < b.longitude();
    return a.latitude() > b.latitude();
}


ListOfPoints::ListOfPoints(const vector<Point>& points, double tolerance) :
    Grid(), points_(points), tolerance_(tolerance) {
    {
        eckit::Timer timer("ListOfPoints -> sort");
        std::sort(points_.begin(), points_.end(),north_south);
        if(points_.size() > 4) {
            cout << "ListOfPoints::ListOfPoints: First: "
                 << " " << points_[0]
                 << ", " << points_[1]
                 << " .. " << points_[points_.size() -2 ]
                 << ", " <<  points_[points_.size() -1 ]
                 << endl;
        }
    }

    {
        eckit::Timer timer("ListOfPoints -> area");
        Point upper_right = *(max_element(points_.begin(), points_.end()));
        Point lower_left  = *(min_element(points_.begin(), points_.end()));
        area_ = Area(upper_right.latitude(),lower_left.longitude(),lower_left.latitude(),upper_right.longitude());

        cout << "ListOfPoints::ListOfPoints " << area_ << endl;
//		cout << "ListOfPoints::ListOfPoints " << upper_right.latitude()<< "   " << lower_left.longitude()<< "   " << lower_left.latitude()<< "   " << upper_right.longitude() << endl;
    }


    if(same(tolerance_,0)) {
        // BR: Assumes uniform density of points
        // I am not sure the math are correct....

        double a = (area_.north() - area_.south()) * (area_.east() - area_.west());
        double g = 360*181;

        cout << "a = " << a << endl;
        cout << "sqrt(a) = " << sqrt(a) << endl;
        cout << "NB points = " << points_.size() << endl;

        double density = points_.size() / sqrt(a);

        double grid    = sqrt(g)/density;

        cout << "grid " << grid << endl;

        tolerance_ =  grid*2.0;
        tolerance_ = 1;
    }


    Point zero(0,0);
    tolerance_ = std::max(
                     zero.earthDistance(Point(tolerance_,0)),
                     zero.earthDistance(Point(0,tolerance_))
                 );

    cout << "ListOfPoints::ListOfPoints: Area: "     << area_     << endl;
    cout << "ListOfPoints::ListOfPoints: tolerance=" <<tolerance_ << endl;
}

ListOfPoints::ListOfPoints(const vector<Point>& points) :
    Grid(), tolerance_(0)
//	Grid(), points_(points), tolerance_(0)
{
    points_.clear();
    vector<Point>::const_iterator it = points.begin(), end = points.end();
    for( ; it != end; it++) {
        points_.push_back(it->checkup());
    }

    /*
    	cout << "ListOfPoints(const vector<Point>& points)" << endl;
    */

}

ListOfPoints::ListOfPoints(const Area& other, const vector<Point>& points, double tolerance) :
    Grid(other), tolerance_(tolerance)
//	Grid(other), points_(points), tolerance_(tolerance)
{
    points_.clear();
    vector<Point>::const_iterator it = points.begin(), end = points.end();
    for( ; it != end; it++) {
        points_.push_back(it->checkup());
    }

    /*
    cout << "ListOfPoints(const Area& other, const vector<Point>& points)" << endl;
    */
}


ListOfPoints::~ListOfPoints() {

}
void ListOfPoints::getGridAsLatLonList(double* lats, double* lons, long* length) const {
    ASSERT((long)points_.size() >= *length);

    vector<Point>::const_iterator i = points_.begin(), stop = points_.end();
    long count = 0;
    for( ; i != stop; ++i) {
        lats[count] = i->latitude();
        lons[count] = i->longitude();
        count++;
    }

    *length = points_.size();
}

void ListOfPoints::findWestAndEastIndex(GridContext* ctx, long j, double west, double east, int& westLongitudeIndex, int& eastLongitudeIndex ) const {
    throw NotImplementedFeature("ListOfPoints::findWestAndEastIndex");
}

string ListOfPoints::predefinedLsmFileName() const {
    return "list.txt";
    throw NotImplementedFeature("ListOfPoints::predefinedLsmFileName");
}

void ListOfPoints::accumulatedDataWE(const double* data, unsigned long dataLength, vector<double>& newData) const {
    throw NotImplementedFeature("ListOfPoints::accumulatedDataWE");
}

void ListOfPoints::reOrderNewData(const vector<double>& data_in, vector<double>& data_out, int scMode) const {
    throw NotImplementedFeature("ListOfPoints::reOrderNewData");
}

long ListOfPoints::getIndex(int i, int j) const {
    throw NotImplementedFeature("ListOfPoints::getIndex");
}

void ListOfPoints::addWeightsAlongLatitude(vector<double>& weights, double weightY, long i) const {
    throw NotImplementedFeature("ListOfPoints::addWeightsAlongLatitude");
}

void ListOfPoints::weightsY(vector<double>& weights) const {
    throw NotImplementedFeature("ListOfPoints::weightsY");
}

bool ListOfPoints::sameAs(const ListOfPoints& other) const {
    return false;
    throw NotImplementedFeature("ListOfPoints::sameAs");
}

void ListOfPoints::setGlobalNorthSouth(double& north, double& south) const {
    throw NotImplementedFeature("ListOfPoints::setGlobalNorthSouth");
}

Grid* ListOfPoints::newListGrid(const Grid& other) const {
    NearestNeigbour nn;
    vector<Point> newPoints;
    newPoints.reserve(points_.size());

    nn.findNearestPoints(other,points_,newPoints);
    sort(newPoints.begin(), newPoints.end(),north_south);

//	Point upper_right = *(max_element(newPoints.begin(), newPoints.end()));
//	Point lower_left  = *(min_element(newPoints.begin(), newPoints.end()));

    vector<Point>::const_iterator i = newPoints.begin(), stop = newPoints.end();
    double north = i->latitude();
    double south = north;
    double east  = i->longitude();
    double west  = east;
    for( ; i != stop; ++i) {
        double lat = i->latitude();
        double lon = i->longitude();
        if(lat > north)
            north = lat;
        if( south > lat)
            south = lat;
        if(lon > east)
            east = lon;
        if( west > lon)
            west = lon;
    }

//	Area newArea(upper_right.latitude(),lower_left.longitude(),lower_left.latitude(),upper_right.longitude());
    Area newArea(north,west,south,east);

    return new ListOfPoints(newArea, newPoints,0);
}

Grid* ListOfPoints::newGrid(const Grid& other) const {
    Area area = areaFix(other);
    return new ListOfPoints(area, points_,0);
}

Grid* ListOfPoints::newGrid(const Area& area) const {
    return new ListOfPoints(area, points_,0);
}

Area ListOfPoints::adjustArea(const Area& area, bool a, bool b) const {
    return Area(area.north(), area.west(), area.south(), area.east());
}

Area ListOfPoints::adjustArea(const Area& area) const {
    return Area(area.north(), area.west(), area.south(), area.east());
}

Area ListOfPoints::adjustAreaMars(const Area& area) const {
    return Area(0, 0, 0, 0);
}

Area ListOfPoints::setGlobalArea() const {
    return Area(0, 0, 0, 0);
}

Area ListOfPoints::fitToGlobalArea(double west) const {
    return setGlobalArea(west);
}

Area ListOfPoints::setGlobalArea(double west) const {
    return Area(0, 0, 0, 0);
}

int  ListOfPoints::truncate(int truncation) const {
    throw NotImplementedFeature("ListOfPoints::truncate");
}

int  ListOfPoints::matchGaussian() const {
    throw NotImplementedFeature("ListOfPoints::matchGaussian");
}

int  ListOfPoints::getLatitudeOffset(double lat) const {
    throw NotImplementedFeature("ListOfPoints::getLatitudeOffset");
}

long  ListOfPoints::getLatitudeOffset(double lat, long& current) const {
    throw NotImplementedFeature("ListOfPoints::getLatitudeOffset");
}

void  ListOfPoints::getOffsets(vector<int>& offsets) const {
    throw NotImplementedFeature("ListOfPoints::getOffsets");
}

void ListOfPoints::generateGrid1D(vector<Point>& points) const {
    //	vector<Point>::const_iterator i = points_.begin(), stop = points_.end();
    //    for( ; i != stop; ++i)
    //		cout << " ListOfPoints::generateGrid1D " << *i << endl;
    points.assign( points_.begin(), points_.end() );
}

void ListOfPoints::generateGrid1D(vector<Point>& llgrid, const Grid& grid)  const {
    if(grid.rotated()) {
        throw NotImplementedFeature("ListOfPoints::generateGrid1D rotation");
    } else
        generateGrid1D(llgrid);
}

class Prioritize {
  public:
    int operator() ( const pair<double, unsigned int>& p1,
                     const pair<double, unsigned int>& p2 ) {
        return p1.first > p2.first;
    }
};


struct ListOfPointsContext : public GridContext {
    int last_i_;
    ListOfPointsContext():last_i_(0) {}

};


GridContext *ListOfPoints::getGridContext() const {
    return new ListOfPointsContext();
}

void ListOfPoints::nearestsByIndex(const Point& where, vector<FieldPoint>& nearest, const vector<double>& data, int scMode, double missingValue, int howMany) const {
    throw NotImplementedFeature("ListOfPoints::nearestsByIndex");
}

double ListOfPoints::conserving(GridContext* ctx,const Point& where, const vector<double>& data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    throw NotImplementedFeature("ListOfPoints::consrving");
}

void ListOfPoints::cellsAreas(vector<Area>& areas, vector<double>& areasSize)  const {
    throw NotImplementedFeature("ListOfPoints::cellsAreas");
}

double ListOfPoints::fluxConserving(GridContext* ctx, const Point& where, const vector<Area>& areas, const vector<double>& inputCellSize, const double* data, int scMode, double missingValue, double outNSincrement, double outWEincrement)  const {
    throw NotImplementedFeature("ListOfPoints::fluxConserving");
    return 0;
}

double ListOfPoints::averageWeighted(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, int scMode, double missingValue, const vector<double>& outLats, double outWEincrement) const {
    throw NotImplementedFeature("ListOfPoints::averageWeighted");
    return 0;
}

double ListOfPoints::averageWeightedLsm(GridContext* ctx, const Point& where, const vector<double>& weights, const vector<double>& data, const vector<double>& dataLsmIn, const vector<double>& dataLsmOut, int scMode, double missingValue, const vector<double>& outLats, double we) const {
    throw NotImplementedFeature("ListOfPoints::averageWeightedLsm");
    return 0;
}

void ListOfPoints::nearest4pts(GridContext* ctx, const Point& where, vector<Point>& result) const {
    throw NotImplementedFeature("ListOfPoints::nearest4pts");
//    nearest4(ctx,where,result);
}

void ListOfPoints::nearestPoints(GridContext* ctx,const Point& where, vector<FieldPoint>& result, const vector<double>& data, int scMode, int howMany)  const {
    result.clear();
    priority_queue< pair< double, unsigned int >,
                    vector <pair< double, unsigned int > >, Prioritize >   pq;


    int psize = points_.size() ;
    ListOfPointsContext* c = static_cast<ListOfPointsContext*>(ctx);
    ASSERT(c);

    int last_i = c->last_i_;
    double last_latitude = 0;

    for(int ii = 0 ; ii < psize ; ii++) {

        int i = (ii + last_i) % psize;

        double d = where.earthDistance(points_[i]);

        if(d <= tolerance_) {
            if(pq.size() == 0) {
                c->last_i_ = i;
            }
            pq.push( pair<double, int>( d, i) );
        } else {
            if(pq.size() == 0 ) {
                last_latitude =  points_[i].latitude();
            } else {
                if(points_[i].latitude() < last_latitude ) {
                    break;
                }
            }
        }

    }

#if 0
    cout << "priority_queue " << pq.size() << endl;
#endif


    howMany = std::min(size_t(howMany),pq.size());


    for(int i = 0 ; i < howMany ; i++) {
        double value = data[pq.top().second];
//		if (!same(value,missingValue))
        result.push_back( FieldPoint(points_[pq.top().second],value));
        pq.pop();
    }
    sort(result.begin(), result.end());
}

void ListOfPoints::dump2file(const string& name, const double* data) const {
    ofstream llvalue;
    llvalue.open(name.c_str(),ios::out | ios::app);
    if (!llvalue)
        throw CantOpenFile(name);

    vector<Point>::const_iterator i = points_.begin(), stop = points_.end();
    int count = 0;
    for( ; i != stop; ++i) {
        if(!area_.empty()) {
            if(area_.isPointWithinArea(*i)) {
                llvalue << setw(5) << i->latitude() <<  " ";
                llvalue << setw(5) << i->longitude();
                llvalue << " " << setw(7) << data[count++] << endl ;
            } else {
                llvalue << setw(5) << i->latitude() <<  " ";
                llvalue << setw(5) << i->longitude();
                llvalue << " " << setw(7) << MISSING_VALUE << endl ;
            }

        } else {
            llvalue << setw(5) << i->latitude() <<  " ";
            llvalue << setw(5) << i->longitude();
            llvalue << " " << setw(7) << data[count++] << endl ;
        }
    }
    llvalue.close();
}

size_t  ListOfPoints::getGridDefinition(vector<long>& v) const {
    throw NotImplementedFeature("ListOfPoints::getGridDefinition");
}

int  ListOfPoints::poleEquatorNumberOfPoints() const {
    throw NotImplementedFeature("ListOfPoints::poleEquatorNumberOfPoints");
}

Grid* ListOfPoints::getGlobalGrid() const {
    throw NotImplementedFeature("ListOfPoints::getGlobalGrid() const");
}

void ListOfPoints::northHemisphereLatitudes(vector<double>& lats) const {
    throw NotImplementedFeature("ListOfPoints::northHemisphereLatitudes() const");
}

void ListOfPoints::latitudes(vector<double>& lats) const {
    throw NotImplementedFeature("ListOfPoints::latitudes() const");
}

vector<double> ListOfPoints::latitudes() const {
    throw NotImplementedFeature("ListOfPoints::latitudes() const");
}

double ListOfPoints::westEastIncrement() const {
    throw NotImplementedFeature("ListOfPoints::westEastIncrement() const");
}

double ListOfPoints::northSouthIncrement() const {
    throw NotImplementedFeature("ListOfPoints::northSouthIncrement() const");
}

unsigned long ListOfPoints::calculatedNumberOfPoints() const {
    return (unsigned long)points_.size();
}

string ListOfPoints::coeffInfo() const {
    throw NotImplementedFeature("ListOfPoints::coeffInfo() const");
}

int ListOfPoints::westEastNumberOfPoints() const {
    throw NotImplementedFeature("ListOfPoints::westEastNumberOfPoints() const");
}

int ListOfPoints::northSouthNumberOfPoints() const {
    throw NotImplementedFeature("ListOfPoints::northSouthNumberOfPoints() const");
}


void ListOfPoints::adjustAreaWestEast(double& west, double& east, double increment) const {
    throw NotImplementedFeature("ListOfPoints::adjustAreaMars(double& west, double& east, double increment) const");
}

void ListOfPoints::adjustAreaWestEastMars(double& west, double& east, double increment) const {
    throw NotImplementedFeature("ListOfPoints::adjustAreaMars(double& west, double& east, double increment) const");
}


bool ListOfPoints::isGlobalWestEast() const {
    return false;
    //	throw NotImplementedFeature("ListOfPoints::isGlobalWestEast() const");
}

bool ListOfPoints::isGlobalNorthSouth() const {
    return false;
    //	throw NotImplementedFeature("ListOfPoints::isGlobalNorthSouth() const");
}

size_t ListOfPoints::loadGridSpec(vector<long>& rgSpec) const {
    throw NotImplementedFeature("ListOfPoints::loadGridSpec");
    return 0;
}

int ListOfPoints::northIndex(double north) const {
    throw NotImplementedFeature("ListOfPoints::northIndex");
    return 0;
}

int ListOfPoints::southIndex(double south) const {
    throw NotImplementedFeature("ListOfPoints::southIndex");
    return 0;
}

void ListOfPoints::print(ostream& out) const {
    out << "ListOfPoints: ";
    //	copy(points_.begin(), points_.end(), ostream_iterator<FieldPoint>(out, "\n"));
}
