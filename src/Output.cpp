/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Output.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
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

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef RegularLatLon_H
#include "RegularLatLon.h"
#endif

#ifndef RegularLatLonellCentered_H
#include "RegularLatLonCellCentered.h"
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

#ifndef ListOfPoints_H
#include "ListOfPoints.h"
#endif

#ifndef Factory_H
#include "Factory.h"
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

#ifndef Input_H
#include "Input.h"
#endif


Output::Output():
	fileName_("none")
{
}

Output::Output(const string& name) :
	fileName_(name)
{
}

Output::~Output()
{
}
/*
Field* Output::defineField(const FieldDescription& dc) const
{
	Grid* grid;

	FieldIdentity id = dc.id_;
	FieldFeatures ft = dc.ft_;

	string       gridType = id.gridType_;

	if(gridType == "sh")
		return new SpectralField(ft.truncation_);

	if(gridType == "latlon") {
		if(!id.reduced_) {
			if(!ft.area_.empty()) {
				if(id.rotated_)
					grid = new RotatedRegularLatLon(ft.area_,ft.ns_,ft.we_,ft.southPole_.latitude(),ft.southPole_.longitude());
				else {
					if(DEBUG)
						cout << "Output::defineField -- Regular LatLon " << endl;
					if(ft.cellCentered_){
						if(ft.nptsNS_ > 0 && ft.nptsWE_ > 0)
							grid = new RegularLatLonCellCentered(ft.area_,ft.nptsNS_,ft.nptsWE_);
						else			
							if(ft.nptsNS_ > 0 && ft.nptsWE_ > 0){
								grid = new RegularLatLonCellCentered(ft.area_,ft.ns_,ft.we_);
							}
							else {
								throw UserError("Output::defineField -- RegularLatLonCellCentered inrements has to be specified");
							}
					}
					else {
						if(ft.nptsNS_ > 0 && ft.nptsWE_ > 0)
							grid = new RegularLatLon(ft.area_,ft.nptsNS_,ft.nptsWE_,ft.globalWestEast_);
						else			
							grid = new RegularLatLon(ft.area_,ft.ns_,ft.we_);
					}
				}
			}
			else {
				if(id.rotated_)
					grid = new RotatedRegularLatLon(ft.ns_,ft.we_,ft.southPole_.latitude(),ft.southPole_.longitude());
				else{
					if( ft.nptsNS_ > 0 && ft.nptsWE_ > 0)
						throw UserError("Output::defineField -- RegularLatLon with number of point north-sout, west-east works only if area provided");
					grid = new RegularLatLon(ft.ns_,ft.we_);
				}
			}
		}
		else {
				if(DEBUG)
					cout << "Output::defineField -- Reduced LatLon " << endl;
				throw UserError("Output::defineField -- Reduced LatLon");
		}
	}
	else if(gridType == "list") {
				cout << "Output::defineField -- List Of Points " << ft.listOfPointsFileType_ << endl;
					Factory factory;
					auto_ptr<Input> f( factory.getInput(ft.listOfPointsFile_,ft.listOfPointsFileType_));
					vector<Point> points;
					f->getLatLonValues(points);
					grid = new ListOfPoints(points);
	}
	else if(gridType == "gaussian") {
			if(!id.reduced_) {
				if(DEBUG)
					cout << "Output::defineField -- Regular Gaussian " << endl;
				if(!ft.area_.empty()) {
					grid = new RegularGaussian(ft.area_, ft.gaussianNumber_);
				}
				else {
					grid = new RegularGaussian(ft.gaussianNumber_);
				}
			}
			else {
				if(DEBUG)
					cout << "Output::defineField -- Reduced Gaussian " << endl;
				if(!ft.area_.empty()) {
					grid = new ReducedGaussian(ft.area_, ft.gaussianNumber_);
				}
				else {
					grid = new ReducedGaussian(ft.gaussianNumber_);
				}
			}
	}
	else {
		cout << "Output::defineField -- Not DECIDED " << endl;
		throw UserError("Output::defineField -- Not DECIDED");
	}

	if(DEBUG)
		cout << "Output::defineField -- Grid => "<< *grid << endl;

	return new GridField(grid,ft.scanningMode_,ft.frame_,ft.bitmap_,ft.bitmapFile_,ft.bitsPerValue_, ft.editionNumber_);
}
*/

void Output::deliverData(const Field& field, std::vector<double>& data) const
{
	if(field.nameOfField() == "sh"){
		const SpectralField& sf  = dynamic_cast<const SpectralField&>(field);
        data = sf.data();
        return;
	}
	else {
		  const GridField& gf   = dynamic_cast<const GridField&>(field);
		  int scanningMode      = gf.scanningMode();
		if(scanningMode == 1)
        {
            data = gf.data();
            return;
		}
		else {
			if(DEBUG)
				cout << "Output::deliverData Different Scanning Mode " << scanningMode << endl;

            gf.grid().reOrderNewData(gf.data(), data, scanningMode);
            return;
		}
		throw WrongValue("Output::deliverData scanning Mode ", scanningMode);
	}

	throw BadParameter("Output::deliverData");
}


void Output::print(ostream& out) const
{
	out << "Output[ file = "<< fileName_ << ']';
}
