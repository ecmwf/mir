/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SpectralToListOfPointsTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef SpectralToSpectralTransformer_H
#include "SpectralToSpectralTransformer.h"
#endif

#ifndef RegularLatLon_H
#include "RegularLatLon.h"
#endif

#ifndef ReducedGaussian_H
#include "ReducedGaussian.h"
#endif

#ifndef RegularGaussian_H
#include "RegularGaussian.h"
#endif

#ifndef Timer_H
#include <eckit/log/Timer.h>
#endif

typedef map<double,vector<Point>,greater<double> > mappedPoints;

SpectralToListOfPointsTransformer::SpectralToListOfPointsTransformer(const string& coeffMethod, int fftMax, bool auresol, bool conversion) :
    SpectralToGridTransformer(coeffMethod, fftMax, auresol, conversion) {
}

SpectralToListOfPointsTransformer::~SpectralToListOfPointsTransformer() {
}

Field* SpectralToListOfPointsTransformer::transform(const Field& in, const Field& out) const {
#if ECREGRID_TIMER
    eckit::Timer("SpectralToListOfPointsTransformer");
#endif
    const SpectralField& input = dynamic_cast<const SpectralField&>(in);
    const GridField&    output = dynamic_cast<const GridField&>(out);

    int truncation = input.truncation();

//	double ll = input.matchLatLon();
//	auto_ptr<Grid> temp (new RegularLatLon(ll,ll));

    int gaussianNumber = input.matchGaussian();
    ReducedGaussian temp(gaussianNumber);

    if(DEBUG) {
        cout << "SpectralToListOfPointsTransformer::transform  corresponding Gaussian number: " << gaussianNumber <<  endl;
    }

    auto_ptr<Grid>globalGrid( temp.getGlobalGrid() );

    Grid* possibleGrid =  output.grid().newListGrid(*globalGrid);
    long calculatedLengthList = possibleGrid->calculatedNumberOfPoints();

    // CACHING OPPORTUNITY
    vector<double> listValues(calculatedLengthList);

    if(DEBUG)
        cout << "SpectralToListOfPointsTransformer::transform possible area : " << possibleGrid->area() << endl;
    vector<Point> newPoints;
    possibleGrid->generateGrid1D(newPoints);

    vector<Point>::const_iterator i = newPoints.begin(), stop = newPoints.end();
    double lat = i->latitude();
    vector<Point> selectedPoints;
    mappedPoints mappedList;
    for( ; i != stop; ++i) {
        double lati = i->latitude();
        if(!same(lati,lat)) {
            if(selectedPoints.size())
                mappedList.insert( pair<double,vector<Point> >(lat,selectedPoints) );
            lat = lati;
            selectedPoints.clear();
        }
        selectedPoints.push_back(*i);
    }
    if(selectedPoints.size())
        mappedList.insert( pair<double,vector<Point> >(lat,selectedPoints) );


    long count = 0;
    long mapcount = 1;
//======================================================
    mappedPoints::const_iterator iter = mappedList.begin(), stopMapped = mappedList.end();
    for ( ; iter != stopMapped; ++iter) {
        double latitude = iter->first;
        size_t sizeLons = iter->second.size();
        /*
        cout << ":::::::::::::::::::::::::::::::::::::::::: " << endl;
        cout << "north: " << latitude << endl;
        cout << "west: " << iter->second[0].longitude() << endl;
        cout << "east: " << iter->second[sizeLons-1].longitude() << endl;
        */
        Area area(latitude,iter->second[0].longitude(),latitude,iter->second[sizeLons-1].longitude());
        ReducedGaussian possibleTemp(area,gaussianNumber);
        long calculatedLengthPossible = possibleTemp.calculatedNumberOfPoints();

        if(DEBUG)
            cout << "SpectralToListOfPointsTransformer::transform  Temp Grid values length: " << calculatedLengthPossible <<  endl;

        // CACHING OPPORTUNITY:
        vector<double> values(calculatedLengthPossible);

        SpectralToSpectralTransformer sp2sp;
        vector<comp> dataComplex;
        sp2sp.transform(input,truncation,dataComplex);

        SpectralToGridTransformer::transform(truncation,input.wind(),dataComplex,possibleTemp,*globalGrid,values);

        if(DEBUG) {
            cout << "SpectralToListOfPointsTransformer::transform mapcount: " << mapcount << " List of points values length: " << calculatedLengthList <<  endl;
            cout << "SpectralToListOfPointsTransformer::transform extract sizeLons: " << sizeLons << endl;
            cout << "SpectralToListOfPointsTransformer::transform extract latitude " << latitude << endl;
        }

        if(sizeLons > 1) {
            vector<Point>::const_iterator it = iter->second.begin(), stop = iter->second.end();
            int startI = it->iIndex();
            if(DEBUG)
                cout << "SpectralToListOfPointsTransformer::extractListOfPoints  startI: " << startI << endl;
            long localCount = 0;
            for ( ; it != stop; ++it) {
                int i = it->iIndex();
                if(DEBUG)
                    cout << "SpectralToListOfPointsTransformer::transform  i: " << i << " values index: " << i - startI  << " values: " << values[i - startI] <<  endl;
                ASSERT((i - startI ) <= calculatedLengthPossible);
                listValues[count++] = values[i - startI];
                localCount++;
            }

            ASSERT(localCount == (long)sizeLons);
        } else {
            vector<Point>::const_iterator it = iter->second.begin();
            listValues[count++] = values[0];
        }
        mapcount++;

    }

//======================================================

    return new GridField( possibleGrid, input,output.bitsPerValue(),output.editionNumber(), output.scanningMode(),false, listValues,output.missingValue());

}

void SpectralToListOfPointsTransformer::print(ostream& out) const {
    SpectralToGridTransformer::print(out);
}
