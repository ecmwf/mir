/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Input.h"

#ifndef Exception_H
#include "Exception.h"
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

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
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

#ifndef FieldIdentity_H
#include "FieldIdentity.h"
#endif

#ifndef FieldFeatures_H
#include "FieldFeatures.h"
#endif

Input::Input():
    fileName_("none") {
}

Input::Input(const string& name):
    fileName_(name) {
}

Input::~Input() {
}

bool Input::exist(const string& dir) const {
    /*
        fstream  file;
        file.open((dir + fileName_).c_str());

        if (file.is_open()) {
            file.close();
            return true;
        }

        return false;
    */

    string fileName = dir + "/" + fileName_;

    if(DEBUG)
        cout << "Input::exist : " << fileName << endl;

    FILE* f = fopen(fileName.c_str(), "r");
    if(f) {
        fclose(f);
        return true;
    }
    return false;
}

Field* Input::defineField(const FieldDescription& dc) const {
    vector<double> values;
    getDoubleValues("", values);
    return defineField(values, dc);

}

Field* Input::defineField(const vector<double>& values, const FieldDescription& dc) const {
    Grid*        inputGrid=0;

    FieldIdentity id = dc.id_;
    FieldFeatures ft = dc.ft_;

    string gridType = id.gridType_;

    if(DEBUG)
        cout << "Input::defineGrid -- !!!!!!!!!!!  gridType= " << gridType << endl;

    if(!id.gridOrSpectral_) {
        // Spherical harmonics
        cout << "Input::defineGrid -- Spectral Truncation = " << ft.truncation_ << endl;
        return new SpectralField(ft.truncation_,ft.parameter_,ft.units_,ft.editionNumber_,ft.centre_,string(ft.levelType_),ft.level_,ft.date_,ft.time_,ft.stepUnits_,ft.startStep_,ft.endStep_,ft.bitsPerValue_,values);
    } else {
        if(gridType == "gaussian") {

            // Regular or Reduced
            if(id.reduced_) {
                if(ft.area_.empty()) {
                    if(ft.global_) {
                        ReducedGaussian temp(ft.gaussianNumber_);
                        Area area = temp.setGlobalArea();
                        inputGrid = new ReducedGaussian(area,ft.gaussianNumber_);
                    } else
                        inputGrid = new ReducedGaussian(ft.area_,ft.gaussianNumber_);
                }
            } else {
                if(ft.area_.empty()) {
                    if(ft.global_) {
                        RegularGaussian temp(ft.gaussianNumber_);
                        Area area = temp.setGlobalArea();
                        inputGrid = new RegularGaussian(area,ft.gaussianNumber_);
                    } else
                        inputGrid = new RegularGaussian(ft.gaussianNumber_);
                } else
                    inputGrid = new RegularGaussian(ft.area_,ft.gaussianNumber_);
                if(DEBUG)
                    cout << "Input::defineGrid -- Regular Gaussian Grid = " << *inputGrid << endl;
            }
        } else if(gridType == "latlon") {

            if(id.rotated_) {
                if(ft.area_.empty()) {
                    Area area(90,0,-90,360);
                    inputGrid = new RotatedRegularLatLon(area,ft.ns_,ft.we_);
                } else
                    inputGrid = new RotatedRegularLatLon(ft.area_,ft.ns_,ft.we_);
            } else {
                // Regular or Reduced
                if(id.reduced_) {
                    if(ft.area_.empty())
                        inputGrid = new ReducedLatLon(ft.ns_,ft.we_,ft.reducedGridDefinition_,ft.reducedGridLatNumber_);
                    else
                        inputGrid = new ReducedLatLon(ft.north(),ft.west(),ft.south(),ft.east(),ft.ns_,ft.we_,ft.reducedGridDefinition_,ft.reducedGridLatNumber_);

                } else {
                    if(ft.area_.empty()) {
                        if(ft.global_) {
                            RegularLatLon temp(ft.ns_,ft.we_);
                            Area area = temp.setGlobalArea();
                            inputGrid = new RegularLatLon(area,ft.ns_,ft.we_);
                        } else
                            inputGrid = new RegularLatLon(ft.ns_,ft.we_);
                    } else
                        inputGrid = new RegularLatLon(ft.area_,ft.ns_,ft.we_);
                }
            }
        } else if(gridType == "list") {
            vector<Point> points;
            getLatLonValues(points);
            inputGrid = new ListOfPoints(points);

        } else
            throw NotImplementedFeature("Input::defineField This Grid is not handled->  " + gridType);
    }

    if(DEBUG) {
        cout << "Input::defineGrid -- Grid characteristics => " << *inputGrid << endl;
        cout << "Input::defineGrid -- Parameter => " << ft.parameterId_ << endl;
    }

    return new GridField(inputGrid,Parameter(ft.parameterId_,ft.table_,string(ft.levelType_)),ft.units_,ft.editionNumber_,ft.centre_,string(ft.levelType_),ft.level_,ft.date_,ft.time_,ft.stepUnits_,ft.startStep_,ft.endStep_,ft.bitsPerValue_,ft.scanningMode_,ft.bitmap_,values,ft.missingValue_);
}


void Input::print(ostream& out) const {
    out << "Input[ file = "<<  fileName_ << ']';
}
