/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GridField.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Parameter_H
#include "Parameter.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif


#include <fstream>
#include <sstream>
#include <algorithm>

// Output fields
GridField::GridField(Grid* grid, int scan,int frame, bool bitmap, const string& bitmapFile, int bitsPerValue, int editionNumber, double missingValue):
    Field(bitsPerValue, editionNumber),
    grid_(grid),
    data_(vector<double>()),
    scanningMode_(scan),
    frame_(frame),
    bitmap_(bitmap),
    bitmapFile_(bitmapFile),
    missingValue_(missingValue) {
}

// Input Fields
GridField::GridField(Grid* grid, const Parameter& param, const string& units, int editionNumber, int centre, const string& levType, int level,int date, int time, const string& stepUnits, int startStep, int endStep, int bitsPerValue, int scan, bool bitmap, const vector<double>& values, double missingValue) :
    Field(param,units,editionNumber,centre,levType,level,date,time,stepUnits,startStep,endStep,bitsPerValue),
    grid_(grid),
    data_(values),
    scanningMode_(scan),
    frame_(0),
    bitmap_(bitmap),
    bitmapFile_("none"),
    missingValue_(missingValue) {
//	ASSERT(grid->calculatedNumberOfPoints() == length);
    ASSERT(dataLength() > 0);
}


// Output Fields after Transformation
GridField::GridField(const Parameter& param,Grid* grid, const Field& f, int bitsPerValue, int editionNumber, int scan, bool bitmap, const vector<double>& values, double missingValue) :
    Field(param,f,bitsPerValue,editionNumber),
    grid_(grid),
    data_(values),
    scanningMode_(scan),
    frame_(0),
    bitmap_(bitmap),
    bitmapFile_("none"),
    missingValue_(missingValue) {
//	ASSERT(values);
    if(dataLength() == 0)
        throw UserError("GridField::GridField after TRANS There are no input values");
}

// Output Fields after Transformation
GridField::GridField(Grid* grid, const Field& f, int bitsPerValue, int editionNumber, int scan, bool bitmap, const vector<double>& values, double missingValue) :
    Field(f,bitsPerValue,editionNumber),
    grid_(grid),
    data_(values),
    scanningMode_(scan),
    frame_(0),
    bitmap_(bitmap),
    bitmapFile_("none"),
    missingValue_(missingValue) {
    if(dataLength() == 0)
        throw UserError("GridField::GridField after TRANS There are no input values");
}

GridField::~GridField() {
}

void GridField::squaredData(vector<double>& data) const {
    // CACHING OPPORTUNITY:
    vector<double> newData = data_;
    for (size_t i = 0; i < newData.size(); i++)
        newData[i] *= newData[i];

    data = newData;
}

double GridField::dataAverage() const {
    double sum = 0;
    for(unsigned long i = 0; i < dataLength(); i++) {
        sum += data_[i];
    }


    if(iszero(sum))
        return sum;

    cout << " sum: " << sum << " len: " << dataLength() << endl;
//	double average = sum / dataLength();
    sum /= dataLength();
    cout << " average: " << sum << endl;

//	return (sum / dataLength());
    return sum;
}

Grid& GridField::grid() const {
    return *grid_;
}

bool GridField::operator==(const GridField& input) const {
//	cout << "GridField::operator==  frame_ " << frame_ << " input.frame() " << input.frame() << endl;
//	return *grid_ == input.grid() && frame_ == input.frame() && bitmap_ == input.bitmap() && scanningMode_ == input.scanningMode_ ;
    return *grid_ == input.grid() && frame_ == input.frame() && scanningMode_ == input.scanningMode_ ;
}

void GridField::outputArea(double* area) const {
    ASSERT(area);
    area[0] = grid().north();
    area[1] = grid().west();
    area[2] = grid().south();
    area[3] = grid().east();
}

void GridField::setOutputAreaAndBasics(FieldDescription& out) const {
    Field::setOutputAreaAndBasics(out);
    out.scanningMode(scanningMode_);
//	out.frame(frame_);
    out.area(grid().north(),grid().west(),grid().south(),grid().east());
}

bool GridField::extrapolateAverageOnPole(double& northValue, double& southValue) const {
    bool aver = !wind() && grid_->isGlobalNorthSouth() && grid_->north() != 90.0 && grid_->south() != 90.0 ;
//	cout << " GridField::extrapolateAverageOnPole is wind: " << wind() << " is global: " << grid_->isGlobalNorthSouth() << " north: " << grid_->north() << " south: " << grid_->south() << endl;
    if(DEBUG)
        cout << "GridField::extrapolateAverageOnPole - Is average for Pole triggered: " << aver << endl;
    if(aver) {
        northValue = averageForPole("northPole");
        southValue = averageForPole("southPole");
    }

    return aver;
}

double GridField::averageForPole(const string& pole) const {
    if(pole == "northPole") {
        long firstLine = grid_->lengthOfFirstLatitude();
        return average(0,firstLine);
    }

    long lastLine = grid_->lengthOfLastLatitude();
    unsigned long end = grid_->calculatedNumberOfPoints();
    ASSERT(end == dataLength());

    long offset = end - lastLine;
    if(DEBUG)
        cout << "GridField::averageForPole lastLine " << lastLine << " end " << end << " offset " << offset << endl;
    return average(offset,end);
}

double GridField::average(long offset, long end) const {
    double sum = 0;
    long count = 0;

    for( long i = offset; i < end ; i++) {
        if(!isMissingValue(data_[i])) {
            sum += data_[i];
            count++;
        }
    }

    if(DEBUG)
        cout << "GridField::average sum: " << sum << " end-offset " << (end - offset) <<  "  average: "  << sum / (end - offset) << endl;
//	return sum / (end - offset);

    if(iszero(sum))
        return sum;

    return sum / count;
}

bool GridField::extrapolateLinearOnPole() const {
    return wind() && grid_->isGlobalNorthSouth() && grid_->north() != 90.0 && grid_->south() != 90.0 && !isRotated();
}

GridField* GridField::gridFieldForOtherScanningMode() const {
    Grid* newGrid;

    if(scanningMode_ == 2) {
        double north = grid_->south();
        double south = grid_->north();
        double east  = grid_->east();
        double west  = grid_->west();
        Area area(north,west,south,east);
        newGrid = grid_->newGrid(area);
    } else if(scanningMode_ == 3) {
        double north = grid_->north();
        double south = grid_->south();
        double east  = grid_->west();
        double west  = grid_->east();
        Area area(north,west,south,east);
        newGrid = grid_->newGrid(area);
    } else if(scanningMode_ == 4) {
        double north = grid_->south();
        double south = grid_->north();
        double east  = grid_->west();
        double west  = grid_->east();
        Area area(north,west,south,east);
        newGrid = grid_->newGrid(area);
    } else {
        throw WrongValue("GridField::gridFieldForOtherScanningMode scanning Mode: ", scanningMode_);
    }

    vector<double> newData;
    grid_->reOrderNewData(data_, newData, scanningMode_);

    return new GridField(newGrid,parameter(),units(),editionNumber(),centre(),levelType(),level(),date(),time(),stepUnits(),startStep(),endStep(),bitsPerValue(),scanningMode_,bitmap_,newData,missingValue_);

}

FieldDescription* GridField::makeOutput(const FieldDescription& out) const {
    FieldDescription* newOut = new FieldDescription;
    *newOut = out;

    if(newOut->decide())
        return newOut;

    int gaussian_number = grid_->poleEquatorNumberOfPoints();
    if(gaussian_number > 0) {
        newOut->gaussianNumber(gaussian_number);
        newOut->isReduced(grid_->reduced());
        if(DEBUG)
            cout << "GridField::makeOutput --> to Gaussian"  << endl;
        return newOut;
    }

    double we = grid_->westEastIncrement();
    double ns = grid_->northSouthIncrement();
    if(ns > 0)
        newOut->increments(we,ns);
    if(grid_->rotated())
        newOut->southPole(grid_->latPole(),grid_->lonPole());
    if(DEBUG)
        cout << "GridField::makeOutput --> to LatLon"  << endl;
    return newOut;
}

void GridField::resetOutput(FieldDescription* out) const {
    int gaussian_number = grid_->poleEquatorNumberOfPoints();
    if(gaussian_number > 0) {
        out->gaussianNumber(gaussian_number);
        if(DEBUG)
            cout << "GridField::resetOutput --> to Gaussian"  << endl;
        return;
    }

    double we = grid_->westEastIncrement();
    double ns = grid_->northSouthIncrement();
    if(ns > 0)
        out->increments(we,ns);
    if(DEBUG)
        cout << "GridField::resetOutput --> to LatLon"  << endl;
}

void GridField::dump2file(const string& name) const {
    ofstream llvalue;

    llvalue.open(name.c_str(),ios::out | ios::app);

    if (!llvalue)
        throw CantOpenFile(name);

    vector<Point> gridPoints;
    grid_->generateGrid1D(gridPoints);

    const size_t gridPoints_size = gridPoints.size();
    if(getenv("ECREGRID_JOHANNES_FLEMING")) {
        for ( size_t i = 0 ; i < gridPoints_size ; i++ ) {
            llvalue << setw(7)  <<  i + 1 ;
            llvalue << setw(10) <<  gridPoints[i].latitude() ;
            llvalue << setw(10) <<  gridPoints[i].longitude();
            llvalue << setw(10) <<  data_[i] ;
            llvalue << setw(8)  <<  number() << "." << table() ;
            llvalue << setw(5)  <<  level() ;
            llvalue << setw(10) <<  date()  ;
            llvalue << setw(6)  <<  time() << endl;
        }
    } else {
        for ( size_t i = 0 ; i < gridPoints_size ; i++ ) {
            llvalue << setw(10) << i + 1 <<  "  ";
            llvalue << setw(10) << gridPoints[i].latitude() <<  " ";
            llvalue << setw(10) << gridPoints[i].longitude();
            llvalue << " " << setw(20) << data_[i] << endl ;
        }
    }
//			llvalue << "-----------------------------------" << endl;

    llvalue.close();
}

void GridField::dump() const {

    vector<Point> gridPoints;

    grid_->generateGrid1D(gridPoints);

    cout << "               " << endl;

    //int k = 0;
    const size_t gridPoints_size = gridPoints.size();
    for ( size_t i = 0 ; i < gridPoints_size ; i++ ) {
        //k++ ;
        /*
        if ( k > 10 ) {
        	k = 0;
        	cout << endl ;
        }
        */

        cout << setw(4) << i+1 << ". ";
//			cout << "[";
        cout << setw(5) << gridPoints[i].latitude() <<  " ";
        cout << setw(5) << gridPoints[i].longitude();
//			cout  << "]";
        cout << " " << setw(7) << data_[i] << endl ;
    }
}

void GridField::print(ostream& out) const {
//	out << endl << "-----------------------------------------------"  << endl;
    out << "GridField:" << endl;
    out << *grid_ << endl << ", scanningMode=[" << scanningMode_ <<  "], dataLength=[" << dataLength()<< "] }" << endl;
    Field::print(out);
    out << endl << "-----------------------------------------------";
//	dump();
}
