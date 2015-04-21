/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif


#include <fstream>

Grid::Grid( double north/*=0.0*/, double west/*=0.0*/, double south/*=0.0*/, double east/*=0.0*/):
    area_(north,west,south,east) {
}

Grid::Grid(const Area& other) :
    area_(other) {
}

Grid::~Grid() {
//cout << "Grid: cleaning up." << endl;
}

void  Grid::aWeights(vector<double>& weights) const {
    weights.clear();

    int nsNumber= northSouthNumberOfPoints();

    vector<double> wy(nsNumber);

    weightsY(wy);

//	cout << "Grid::aWeights nsNumber " << nsNumber << endl;

    for(int i = 0; i < nsNumber; i++) {
        addWeightsAlongLatitude(weights,wy[i],i);
    }
}

Grid* Grid::newGrid() const {
    return newGrid(area_);
}

Grid& Grid::getGridType() {
    return *this;
}


bool Grid::operator==(const Grid& other) const {
    if(other.area_.empty())
        return this->equals(other);
    return area_ == other.area_ && this->equals(other);
}

bool Grid::operator!=(const Grid& other) const {
    return this->equals(other);
}

double Grid::north() const {
    return area_.north();
}

double Grid::west()  const {
    return area_.west();
}

double Grid::east()  const {
    return area_.east();
}

double Grid::south() const {
    return area_.south();
}


void Grid::setGlobalWestEast( double& west, double& east, double increment) const {
    if (west == -180.0) {
        east = 180.0 - increment;
        return;
    }

//    else if(west == 0){
    else {
        west = 0;
        east =  EQUATOR - increment;
        return;
    }

// It is allowed only two values for west 0 and -180 at the moment

    if(west <= 0)
        east = west + 360.0 - increment;
    else
        east = west - 360.0 + increment;
}

void Grid::adjustAreaWestEastMars( double& west, double& east, double increment) const {
    double newWest, newEast;
    int n;

    if ( west > 0) {
        // for emos simulation we need our new value to lie outside the
        // original west value, i.e. have a lower positive value
        n = int((west / increment) + AREA_FACTOR );
        newWest = n * increment;
    } else {
        // for emos simulation we want out new value to have a
        // higher negative value than the original... i.e. be more
        // westward than the subarea specification
        n = int(0.5 + (((-west) / increment) + AREA_FACTOR) );
        newWest = (-n) * increment;
    }

    n = int((east - newWest) / increment + AREA_FACTOR);

    newEast = newWest + (n * increment);

    if (DEBUG)
        cout << "Grid::adjustAreaWestEastMars west=" << west << " east=" << east << " before adjustment (emos style)" << endl;


    if (newEast < east)
        while (east > newEast && !same(east, newEast))
            newEast += increment;

    east = newEast;

    if (newWest > west)
        while (west < newWest && !same(west, newWest))
            newWest -= increment;

    west = newWest;

// new piece of code
    if ( (east - west) >= 360.0 )
        east = east - increment;

    // want east and west > 0 and east > west
    if (east < west)
        east+=360.0;
    while (east < ROUNDING_FACTOR) {
        east+=360.0;
        west+=360.0;
    }

    if (DEBUG)
        cout << "Grid::adjustAreaWestEastMars west=" << west << " east=" << east << " after adjustment (emos style)" << endl;

}


void Grid::adjustAreaWestEast( double& west, double& east, double increment) const {
#if ECREGRID_EMOS_SIMULATION
    return adjustAreaWestEastMars(west, east, increment);
#else
    int n;
    double newWest;
    double westOrig = west, eastOrig = east;

    if(getenv("ECREGRID_OLD_ADJUST_WE")) {
        if ( west > 0) {
            n = int((west + increment - 1) / increment + 0.5 );
            newWest = n * increment;
            if (newWest < west )
                west = newWest + increment;
            else
                west = newWest;
        } else {
            // ssp grib limits AREA_FACTOR
            n = int((-west + AREA_FACTOR ) / increment );
            newWest = (-n) * increment;
            if (newWest + AREA_FACTOR < westOrig )
                west = newWest + increment;
            else
                west = newWest;
        }
        n = int((east - west) / increment + 0.5);
        east = west + (n * increment);
        return;
    }

    if ( west > 0) {
        // we want our new west to lie inside the original, i.e. be of a
        // higher value. So we must round up to the next grid boundary
        n = int(0.5 + (west / increment) + AREA_FACTOR );
        west = n * increment;
    } else {
        // for negative values, we want our new value to have a lower value
        // than the original, i.e. be less negative and lie within the
        // original value
        n = int((-west) / increment + AREA_FACTOR );
        west = (-n) * increment;
    }


    ASSERT(west + AREA_FACTOR > westOrig );

    n = int((east - west) / increment + AREA_FACTOR);

    east = west + (n * increment);

    ASSERT(east < eastOrig + AREA_FACTOR );

// new piece of code
    if ( (east - west) >= 360.0 ) {
        east = east - increment;
    }
#endif
}


bool Grid::isGlobal() const {
    if(DEBUG)
        cout << "Grid::isGlobal west-east -> " <<  isGlobalWestEast() << " north-south -> " << isGlobalNorthSouth() << endl;
    return (isGlobalWestEast() && isGlobalNorthSouth());
}

Grid* Grid::newListGrid(const Grid& other) const {
    throw UserError("Grid::newListGrid => "," is called only for List of points");
    return 0;
}

int Grid::match(int truncation) const {
    throw UserError("Grid::match => "," is called only for Rotated grids");
}

void Grid::angularChange(vector <double>& angles) const {
    throw UserError("Grid::angularChange => "," is called only for Rotated grids");
}

Area Grid::areaFix(const Grid& input) const {
    if(DEBUG) {
        cout << "Grid::areaFix input: " << input << endl;
        cout << "Grid::areaFix is input area global? " << input.isGlobal() << endl;
        cout << "Grid::areaFix Output Area => " << area_ << endl;
        cout << "Grid::areaFix is Output area empty? => " << area_.empty()  << endl;

    }

    if (area_.empty() && input.isGlobal()) {
        return setGlobalArea(input.west());
    }

    if(DEBUG)
        cout << "Grid::areaFix is Output area global? => " << isGlobal()  << endl;

    if (isGlobal() && input.isGlobal()) {
        return setGlobalArea(west());
    }

    if(DEBUG)
        cout << "Grid::areaFix is input area global? " << input.isGlobal() << endl;

    if (input.isGlobal()) {
        return adjustArea(area_,isGlobalWestEast(),isGlobalNorthSouth());
    }

    bool globalWE = input.isGlobalWestEast();
    bool globalNS = input.isGlobalNorthSouth();

    if(DEBUG) {
        cout << "Grid::areaFix is Output area empty? => " << area_.empty()  << endl;
        cout << "Grid::areaFix is input area global? " << input.isGlobal() << endl;
    }

    if (area_.empty() && !input.isGlobal()) {
        return adjustArea(input.area_,globalWE,globalNS);
    }

    // for where the area is initialised to global: If we have a global
    // output area and non-global input area then we are okay to
    // proceed using the input area
    if (isGlobal() && !input.isGlobal()) {
        return adjustArea(input.area_,globalWE,globalNS);
    }

    if(area_.contain(input.area_,globalWE,globalNS)) {
        return adjustArea(area_,globalWE,globalNS);
    }

    if(DEBUG)
        cout << "Grid::areaFix Area contain => " << area_.contain(input.area_) << endl;
    throw UserError("Grid::areaFix => ","there is NO common AREA");
    Area newArea = area_.intersection(input.area_);
    if ( !newArea.empty()) {
        return adjustArea(newArea,false,false);
    }

    throw UserError("Grid::areaFix => ","there is NO common AREA");
}

void Grid::print(ostream& out) const {
    out << "Grid{ north=[" << north() << "], west=[" << west() <<  "], east=[" << east() << "}, south=["<< south() << "] }";
}
