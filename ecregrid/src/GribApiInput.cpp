/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GribApiInput.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldIdentity_H
#include "FieldIdentity.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef RegularLatLon_H
#include "RegularLatLon.h"
#endif

#ifndef RotatedRegularLatLon_H
#include "RotatedRegularLatLon.h"
#endif

#ifndef RegularGaussian_H
#include "RegularGaussian.h"
#endif

#ifndef PseudoRegularGaussian_H
#include "PseudoRegularGaussian.h"
#endif

#ifndef ReducedGaussian_H
#include "ReducedGaussian.h"
#endif

#ifndef ReducedLatLon_H
#include "ReducedLatLon.h"
#endif

#ifndef ListOfPoints_H
#include "ListOfPoints.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#ifndef RegularLatLonCellCentered_H
#include "RegularLatLonCellCentered.h"
#endif

GribApiInput::GribApiInput(const string &name):
    Input(name) {
}

GribApiInput::GribApiInput():
    Input() {
}

GribApiInput::~GribApiInput() {
}

grib_handle  *GribApiInput::next(FILE *fp) {
    int err;
    return grib_handle_new_from_file(0, fp, &err);
}
/*
bool  GribApiInput::next(FILE* fp)
{
	throw NotImplementedFeature(GribApiInput::next);
	return true;
}
*/

Input *GribApiInput::newInput(const string &name) const {
    return new GribApiInput(name);
}

int GribApiInput::scanningMode(long iScansNegatively, long jScansPositively) const {
    if (iScansNegatively == 0 && jScansPositively == 0)
        return 1;
    if (iScansNegatively == 0 && jScansPositively == 1)
        return 2;
    if (iScansNegatively == 1 && jScansPositively == 0)
        return 3;
    if (iScansNegatively == 1 && jScansPositively == 1)
        return 4;

    ASSERT(iScansNegatively == 0 || iScansNegatively == 1);
    ASSERT(jScansPositively == 0 || jScansPositively == 1);

    return 1;
}

Grid *GribApiInput::defineGridForCheck(const string &path) const {
    /* Open Input file */
    FILE *fp = fopen((path + "/" + fileName_).c_str(), "r");
    if (!fp) {
        throw CantOpenFile("GribApiInput::defineGridForChec " + path + fileName_);
    }
    int err;
    grib_handle *handle = grib_handle_new_from_file(0, fp, &err);

    char gridType[80];
    size_t size = sizeof(gridType);

    GRIB_CHECK(grib_get_string(handle, "typeOfGrid", gridType, &size), 0);
    string composedName(gridType);

    Grid *grid;

    FieldIdentity id(composedName);

    if (!id.gridOrSpectral_) {
        // Spherical harmonics
        fclose(fp);
        if (handle) {
            grib_handle_delete(handle);
            handle = NULL;
        }
        throw UserError("GribApiInput::defineGridForCheck Lsm MUST BE Grid -> ", composedName);
    }
    //===============================================================
    double north, west, south, east;
    double weIncrement = 0, nsIncrement = 0;
    long gaussianNumber;
    size_t rgSpecLength = 0;
    // CACHING OPPORTUNITY: ?
    vector<long> rgSpec;
    bool decide = false;
    long scanMode;
    // Pick up Area and Scanning Mode if the field is Grid
    // Area
    GRIB_CHECK(grib_get_double(handle, "latitudeOfFirstGridPointInDegrees", &north), 0);
    GRIB_CHECK(grib_get_double(handle, "longitudeOfFirstGridPointInDegrees", &west), 0);
    GRIB_CHECK(grib_get_double(handle, "latitudeOfLastGridPointInDegrees", &south), 0);
    GRIB_CHECK(grib_get_double(handle, "longitudeOfLastGridPointInDegrees", &east), 0);
    // Scanning mode
    long iScansNegatively = 0, jScansPositively = 0;
    GRIB_CHECK(grib_get_long(handle, "iScansNegatively", &iScansNegatively), 0);
    GRIB_CHECK(grib_get_long(handle, "jScansPositively", &jScansPositively), 0);
    scanMode = scanningMode(iScansNegatively, jScansPositively);
    if (DEBUG) {
        cout << "GribApiInput::defineGridForCheck - Latitude Of First Grid Point: " << north << endl;
        cout << "GribApiInput::defineGridForCheck - Longitude Of First Grid Point: " << west << endl;
        cout << "GribApiInput::defineGridForCheck - Latitude Of Last Grid Point: " << south << endl;
        cout << "GribApiInput::defineGridForCheck - Longitude Of Last Grid Point: " << east << endl;
        cout << "GribApiInput::defineGridForCheck - Scanning Mode: " << scanMode << endl;
    }
    if (id.reduced_) {
        /* get reduced grid specification */
        GRIB_CHECK(grib_get_size(handle, "pl", &rgSpecLength), 0);
        if (DEBUG)
            cout << "GribApiInput::defineGridForCheck - rgSpecLength: " << rgSpecLength << endl;

        rgSpec.resize(rgSpecLength);
        GRIB_CHECK(grib_get_long_array(handle, "pl", &rgSpec[0], &rgSpecLength), 0);
    }
    // Check if boundaries have to be reorder and set area
    Area area(north, west, south, east, scanMode);

    // Gaussian grid
    if (id.gridType_ == "gaussian") {
        // Gaussian Grid Number
        GRIB_CHECK(grib_get_long(handle, "numberOfParallelsBetweenAPoleAndTheEquator", &gaussianNumber), 0);
        if (DEBUG)
            cout << "GribApiInput::defineGridForCheck - Gaussian Number: " << gaussianNumber << endl;
        decide = true;
        if (id.reduced_) {
            ASSERT(rgSpecLength > 0);
            grid = new ReducedGaussian(area, gaussianNumber, &rgSpec[0], rgSpecLength);
        } else {
            long nptsWE;
            GRIB_CHECK(grib_get_long(handle, "Ni", &nptsWE), 0);
            if (area.isGlobalWestEast(90.0 / gaussianNumber) && !same(90.0 / gaussianNumber, 360.0 / nptsWE)) {
                cout << "GribApiInput::defineGridForCheck WARNING pseudo gaussian grid: " << gaussianNumber << endl;
                grid = new PseudoRegularGaussian(area, gaussianNumber, nptsWE);
            } else {
                grid = new RegularGaussian(area, gaussianNumber);
            }
        }
    }
    // Lat-Lon grid
    else if (id.gridType_ == "latlon") {
        // J Direction Increment
        long nptsNS;
        //GRIB_CHECK(grib_get_long(handle,"numberOfPointsAlongAMeridian",&nptsNS),0);
        GRIB_CHECK(grib_get_long(handle, "Nj", &nptsNS), 0);
        GRIB_CHECK(grib_get_double(handle, "jDirectionIncrementInDegrees", &nsIncrement), 0);
        if (DEBUG)
            cout << "GribApiInput::defineGridForCheck - j Direction Increment: " << nsIncrement << " j Direction Number Of Points: " << nptsNS << endl;
        decide = true;

        if (id.reduced_) {
            ASSERT(rgSpecLength > 0);
            grid = new ReducedLatLon(area, nsIncrement, weIncrement, &rgSpec[0], rgSpecLength);
        } else {
            // I Direction Increment
            GRIB_CHECK(grib_get_double(handle, "iDirectionIncrementInDegrees", &weIncrement), 0);
            long nptsWE;
            GRIB_CHECK(grib_get_long(handle, "Ni", &nptsWE), 0);
            if (DEBUG)
                cout << "GribApiInput::defineGridForCheck -- i Direction Increment = " << weIncrement << " i Direction Number Of Points: " << nptsWE << endl;
            if (area.isCellCentered(nsIncrement, weIncrement)) {
                bool shift = false;
                if (area.west() > 0)
                    shift = true;
                grid = new RegularLatLonCellCentered(nptsNS, nptsWE, shift);
            } else {
                grid = new RegularLatLon(area, nsIncrement, weIncrement);
            }

        }
    } else
        throw UserError("GribApiInput::defineGridForCheck Lsm MUST BE Lat-Lon or Gaussian -> ", composedName);


    fclose(fp);
    if (handle) {
        grib_handle_delete(handle);
        handle = NULL;
    }
    return grid;
}

Field *GribApiInput::defineField(grib_handle *handle, vector<double> &values ) const {
    Grid        *grid;


    long   parameter = 0, table, scanMode = 0;
    //	long   accuracy = 0;
    long   bitmapPresent = 0;
    bool   bitmapPr = false;
    double north, west, south, east;

    double weIncrement = 0, nsIncrement = 0;
    long editionNumber = 1;
    long level = 0, date = 0, time = 0;
    //	long stepU = 0;
    long startStep = 0, endStep = 0;
    long bitsPerValue = 0;
    bool decide = false;
    long truncation, gaussianNumber;
    //	long localDefinitionNumber;
    size_t rgSpecLength = 0;

    // CACHING OPPORTUNITY: ?
    vector<long> rgSpec;

    long centre = 0;

    char levelType[80] ;
    char stepUnits[80] ;
    char gridType[80];
    char units[180] ;

    size_t size = sizeof(levelType);
    size_t size1 = sizeof(units);

    GRIB_CHECK(grib_get_string(handle, "typeOfGrid", gridType, &size), 0);
    string composedName(gridType);

    FieldIdentity id(composedName);

    GRIB_CHECK(grib_get_string(handle, "levelType", levelType, &size), 0);
    GRIB_CHECK(grib_get_long(handle, "level", &level), 0);


    GRIB_CHECK(grib_get_long(handle, "dataDate", &date), 0);
    GRIB_CHECK(grib_get_long(handle, "dataTime", &time), 0);

    //	GRIB_CHECK(grib_get_long(handle,"localDefinitionNumber",&localDefinitionNumber),0);

    GRIB_CHECK(grib_get_long(handle, "centre", &centre), 0);
    GRIB_CHECK(grib_get_string(handle, "units", units, &size1), 0);

    GRIB_CHECK(grib_get_string(handle, "stepUnits", stepUnits, &size), 0);
    GRIB_CHECK(grib_get_long(handle, "startStep", &startStep), 0);
    GRIB_CHECK(grib_get_long(handle, "endStep", &endStep), 0);

    GRIB_CHECK(grib_get_long(handle, "bitsPerValue", &bitsPerValue), 0);

    string fileType;
    GRIB_CHECK(grib_get_long(handle, "editionNumber", &editionNumber), 0);
    if (editionNumber == 1) {
        //		GRIB_CHECK(grib_get_long(handle,"parameter",&parameter),0);
        fileType = "grib1";
    } else
        fileType = "grib2";

    GRIB_CHECK(grib_get_long(handle, "paramId", &parameter), 0);
    //		cout << "GribApiInput::defineField -- ParamId = " << parameter << endl;


    if (parameter > 1000) {
        table     = parameter / 1000;
        parameter = parameter % 1000;
    } else
        table = 128;

    Parameter param = Parameter(parameter, table, levelType);

    if (DEBUG)
        cout << "GribApiInput::defineField param: " << param << endl;

    // If bitmap Present
    if (id.gridOrSpectral_) {
        GRIB_CHECK(grib_get_long(handle, "bitmapPresent", &bitmapPresent), 0);
        if (bitmapPresent) {
            bitmapPr = true;
            GRIB_CHECK(grib_set_double(handle, "missingValue", MISSING_VALUE), 0);
        }
    }

    if (DEBUG) {
        cout << "GribApiInput::defineField -- Edition Number = " << editionNumber << endl;
        cout << "GribApiInput::defineField -- Type Of Grid = " << gridType << endl;
        cout << "GribApiInput::defineField -- Parameter = " << parameter << endl;
        cout << "GribApiInput::defineField -- Centre = " << centre << endl;
        cout << "GribApiInput::defineField -- Date = " << date << endl;
        cout << "GribApiInput::defineField -- Time = " << time << endl;
        cout << "GribApiInput::defineField -- Level Type = " << levelType << endl;
        cout << "GribApiInput::defineField -- Level = " << level << endl;
        cout << "GribApiInput::defineField -- Units = " << units << endl;
        cout << "GribApiInput::defineField -- Step Units = " << stepUnits << endl;
        cout << "GribApiInput::defineField -- Step Start = " << startStep << endl;
        cout << "GribApiInput::defineField -- Step End = " << endStep << endl;
        cout << "GribApiInput::defineField -- Bits Per Value = " <<  bitsPerValue << endl;
        cout << "GribApiInput::defineField -- Bitmap Present  = " <<  bitmapPr << endl;
        //		cout << "GribApiInput::defineField -- Local Definition = " <<  localDefinitionNumber << endl;
    }

    if (DEBUG)
        cout << "GribApiInput::defineField values size " << values.size() << endl;
    ASSERT(values.size() > 0);

    size_t valuesLength = values.size();
    GRIB_CHECK(grib_get_double_array(handle, "values", &values[0], &valuesLength), 0);

    if (!id.gridOrSpectral_) {
        // Spherical harmonics
        GRIB_CHECK(grib_get_long(handle, "pentagonalResolutionParameterJ", &truncation), 0);
        if (DEBUG) {
            cout << "GribApiInput::defineField - Spectral Truncation: " << truncation << endl;
            cout << "------------------------------------------------" << endl;
        }
        return new SpectralField(truncation, param, units, editionNumber, centre, string(levelType), level, date, time, stepUnits, startStep, endStep, bitsPerValue, values);
    } else {
        // Pick up Area and Scanning Mode if the field is Grid
        // Area
        GRIB_CHECK(grib_get_double(handle, "latitudeOfFirstGridPointInDegrees", &north), 0);
        GRIB_CHECK(grib_get_double(handle, "longitudeOfFirstGridPointInDegrees", &west), 0);
        GRIB_CHECK(grib_get_double(handle, "latitudeOfLastGridPointInDegrees", &south), 0);
        GRIB_CHECK(grib_get_double(handle, "longitudeOfLastGridPointInDegrees", &east), 0);
        // Scanning mode
        long iScansNegatively = 0, jScansPositively = 0;
        GRIB_CHECK(grib_get_long(handle, "iScansNegatively", &iScansNegatively), 0);
        GRIB_CHECK(grib_get_long(handle, "jScansPositively", &jScansPositively), 0);
        scanMode = scanningMode(iScansNegatively, jScansPositively);
        if (DEBUG) {
            cout << "GribApiInput::defineField - Latitude Of First Grid Point: " << north << endl;
            cout << "GribApiInput::defineField - Longitude Of First Grid Point: " << west << endl;
            cout << "GribApiInput::defineField - Latitude Of Last Grid Point: " << south << endl;
            cout << "GribApiInput::defineField - Longitude Of Last Grid Point: " << east << endl;
            cout << "GribApiInput::defineField - Scanning Mode: " << scanMode << endl;
        }
        if (id.reduced_) {
            /* get reduced grid specification */
            GRIB_CHECK(grib_get_size(handle, "pl", &rgSpecLength), 0);
            if (DEBUG)
                cout << "GribApiInput::defineField - rgSpecLength: " << rgSpecLength << endl;

            //CACHING OPPORTUNITY: ?
            rgSpec.resize(rgSpecLength);
            GRIB_CHECK(grib_get_long_array(handle, "pl", &rgSpec[0], &rgSpecLength), 0);
        }
    }
    // Check if boundaries have to be reorder and set area
    Area area(north, west, south, east, scanMode);

    // Gaussian grid
    if (id.gridType_ == "gaussian") {
        // Gaussian Grid Number
        GRIB_CHECK(grib_get_long(handle, "numberOfParallelsBetweenAPoleAndTheEquator", &gaussianNumber), 0);
        if (DEBUG)
            cout << "GribApiInput::defineField - Gaussian Number: " << gaussianNumber << endl;
        decide = true;
        if (id.reduced_) {
            ASSERT(rgSpecLength > 0);
            //			grid = new ReducedGaussian(north,west,south,east,gaussianNumber,rgSpec,rgSpecLength);
            grid = new ReducedGaussian(area, gaussianNumber, &rgSpec[0], rgSpecLength);
        } else {
            long nptsWE;
            GRIB_CHECK(grib_get_long(handle, "Ni", &nptsWE), 0);
            if (area.isGlobalWestEast(90.0 / gaussianNumber) && 90.0 / gaussianNumber != 360.0 / nptsWE) {
                cout << "GribApiInput::defineGrid WARNING pseudo gaussian grid: " << gaussianNumber << endl;
                grid = new PseudoRegularGaussian(area, gaussianNumber, nptsWE);
            } else {
                grid = new RegularGaussian(area, gaussianNumber);
            }
            //				cout << "GribApiInput::defineField -- Regular Gaussian Grid = " << *grid << endl;
        }
    }
    // Lat-Lon grid
    else if (id.gridType_ == "latlon") {
        // J Direction Increment
        long nptsNS;
        //GRIB_CHECK(grib_get_long(handle,"numberOfPointsAlongAMeridian",&nptsNS),0);
        GRIB_CHECK(grib_get_long(handle, "Nj", &nptsNS), 0);
        GRIB_CHECK(grib_get_double(handle, "jDirectionIncrementInDegrees", &nsIncrement), 0);
        if (DEBUG)
            cout << "GribApiInput::defineField - j Direction Increment: " << nsIncrement << " j Direction Number Of Points: " << nptsNS << endl;
        decide = true;

        if (id.reduced_) {
            ASSERT(rgSpecLength > 0);
            //		grid = new ReducedLatLon(north,west,south,east,nsIncrement,weIncrement,rgSpec,rgSpecLength);
            grid = new ReducedLatLon(area, nsIncrement, weIncrement, &rgSpec[0], rgSpecLength);
        } else {
            // I Direction Increment
            GRIB_CHECK(grib_get_double(handle, "iDirectionIncrementInDegrees", &weIncrement), 0);
            long nptsWE;
            //	GRIB_CHECK(grib_get_long(handle,"numberOfPointsAlongAParalel",&nptsWE),0);
            GRIB_CHECK(grib_get_long(handle, "Ni", &nptsWE), 0);
            if (DEBUG)
                cout << "GribApiInput::defineField -- i Direction Increment = " << weIncrement << " i Direction Number Of Points: " << nptsWE << endl;

            bool shift = false;
            if (area.west() > 0)
                shift = true;
            if (getenv("ECREGRID_CELL_CENTER")) {
                grid = new RegularLatLonCellCentered(nptsNS, nptsWE, shift);
            } else {
                if (area.isCellCentered(nsIncrement, weIncrement)) {
                    grid = new RegularLatLonCellCentered(nptsNS, nptsWE, shift);
                } else {
                    grid = new RegularLatLon(area, nsIncrement, weIncrement);
                }
            }

        }
    } else {
        vector<Point> points;
        getLatLonValues(points);
        grid = new ListOfPoints(points);
    }
    ASSERT(valuesLength == grid->calculatedNumberOfPoints());
    if (DEBUG)
        cout << "------------------------------------------------" << endl;
    return new GridField(grid, param, units, editionNumber, centre, string(levelType), level, date, time, stepUnits, startStep, endStep, bitsPerValue, scanMode, bitmapPr, values, MISSING_VALUE);
}



bool *GribApiInput::getLsmBoolValues(size_t *valuesLength) const {
    /* Open input file */
    FILE *in = fopen(fileName_.c_str(), "r");
    if (! in) {
        throw CantOpenFile("GribApiInput::getLsmBoolValues " + fileName_);
    }

    grib_handle *h = NULL;
    int err;

    /* Create new handles from a messages in a file */
    h = grib_handle_new_from_file(0, in, &err);
    cout << "GribApiInput::getLsmBoolValuesFromGrib gribFile -- " << fileName_ << endl;
    if (h == NULL) {
        cerr << "GribApiInput::getLsmBoolValuesFromGrib -- unable to create handle from file: " << fileName_ << endl;
        throw ReadError(fileName_);
    }

    /* get the size of the values array */
    GRIB_CHECK(grib_get_size(h, "values", valuesLength), 0);

    double *values = new double[*valuesLength];

    // get data values
    GRIB_CHECK(grib_get_double_array(h, "values", values, valuesLength), 0);
    fclose(in);

    bool *lsm = new bool[*valuesLength];

    for ( unsigned int i = 0 ; i < *valuesLength ;  i++ ) {
        if (values[i] >= 0.5)
            lsm[i] = true;
        else
            lsm[i] = false;
    }
    if (lsm)
        cout << "GribApiInput::getLsmBoolValuesFromGrib gribFile -- " << fileName_ << endl;

    return lsm;
}

void GribApiInput::getLatLonValues(vector<Point> &points) const {
    /* Open input file */
    FILE *in = fopen(fileName_.c_str(), "r");
    if (! in) {
        throw CantOpenFile("GribApiInput::getLatLonValues " + fileName_);
    }

    grib_handle *h = NULL;
    int err;

    /* Create new handles from a messages in a file */
    h = grib_handle_new_from_file(0, in, &err);
    if (h == NULL) {
        cerr << "GribApiInput::getLatLonValues -- unable to create handle from file: " << fileName_ << endl;
        throw ReadError(fileName_);
    }

    double missingValue, lat, lon, value;
    /* Get the double representing the missing value in the field. */
    GRIB_CHECK(grib_get_double(h, "missingValue", &missingValue), 0);

    /* A new iterator on lat/lon/values is created from the message handle h. */
    grib_iterator *iter = grib_iterator_new(h, 0, &err);
    if (err != GRIB_SUCCESS) GRIB_CHECK(err, 0);

    long n = 0;
    /* Loop on all the lat/lon/values. */
    while (grib_iterator_next(iter, &lat, &lon, &value)) {
        if (lon < 0)
            lon = lon + 360.0;
        Point point(lat, lon);
        points.push_back(point);
        cout << "GribApiInput::getLatLonValuesFromGrib " << n << ":  lat " << lat << " lon " << lon <<  endl;
        n++;
    }

    /* At the end the iterator is deleted to free memory. */
    grib_iterator_delete(iter);

    /* At the end the grib_handle is deleted to free memory. */
    if (h) {
        grib_handle_delete(h);
        h = NULL;
    }

    fclose(in);
}
void GribApiInput::getDoubleValues(const string &name, vector<double> &values)  const {
    /* Open input file */
    FILE *in = fopen((name + "/" + fileName_).c_str(), "r");
    if (! in) {
        throw CantOpenFile("GribApiInput::getDoubleValues =>  " + name + fileName_);
    }

    grib_handle *h = NULL;
    int err;

    /* Create new handles from a messages in a file */
    h = grib_handle_new_from_file(0, in, &err);
    if (h == NULL) {
        cerr << "GribApiInput::getDoubleValuesFromGrib -- unable to create handle from file: " << name + fileName_ << endl;
        throw ReadError(name + fileName_);
    }
    /* get the size of the values array */
    size_t valuesLength;
    GRIB_CHECK(grib_get_size(h, "values", &valuesLength), 0);
    if (DEBUG)
        cout << "GribApiInput::getDoubleValuesFromGrib -- valuesLength: " << valuesLength << endl;

    //double* values = new double[*valuesLength];
    values.resize(valuesLength);
    if (valuesLength > 0) {
        // get data values
        GRIB_CHECK(grib_get_double_array(h, "values", &values[0], &valuesLength), 0);
        //	if(values)
        //		cout << "GribApiInput::getDoubleValuesFromGrib -- values: " << values[105] << endl;
    }
    fclose(in);
    if (h) {
        grib_handle_delete(h);
        h = NULL;
    }

}

size_t GribApiInput::getReducedGridSpecification(vector<long> &values) const {
    /* Open input file */
    FILE *in = fopen(fileName_.c_str(), "r");
    if (! in) {
        throw CantOpenFile("GribApiInput::getReducedGridSpecification " + fileName_);
    }

    grib_handle *h = NULL;
    int err;

    /* Create new handles from a messages in a file */
    h = grib_handle_new_from_file(0, in, &err);
    cout << "GribApiInput::getReducedGridSpecFromGrib GRIB message -- " << endl;
    if (h == NULL) {
        cerr << "GribApiInput::getReducedGridSpecFromGrib -- unable to create handle from file: " << fileName_ << endl;
        throw ReadError(fileName_);
    }
    size_t valuesLength = 0;
    /* get the size of the values array */
    GRIB_CHECK(grib_get_size(h, "pl", &valuesLength), 0);
    cout << "GribApiInput::getReducedGridSpecFromGrib -- valuesLength: " << valuesLength << endl;
    values.clear();
    if (values.size() < valuesLength)
        values.resize(valuesLength);

    long *valuesGa = new long[valuesLength];

    // get data values
    GRIB_CHECK(grib_get_long_array(h, "pl", valuesGa, &valuesLength), 0);

    values.assign(valuesGa, valuesGa + valuesLength);
    delete [] valuesGa;
    fclose(in);

    if (h) {
        grib_handle_delete(h);
        h = NULL;
    }

    return valuesLength;
}
