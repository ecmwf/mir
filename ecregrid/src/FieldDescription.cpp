/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Area_H
#include "Area.h"
#endif

#ifndef Parameter_H
#include "Parameter.h"
#endif

#ifndef Input_H
#include "Input.h"
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

#ifndef PolarStereographic_H
#include "PolarStereographic.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef RegularLatLonellCentered_H
#include "RegularLatLonCellCentered.h"
#endif


FieldDescription::FieldDescription() :
	id_(),ft_()
{
}
FieldDescription::FieldDescription(const string& composedName) :
	id_(composedName),ft_()
{
}

FieldDescription::~FieldDescription()
{
}

FieldDescription& FieldDescription::operator=(const FieldDescription& f)
{
	if (this == &f)
		return *this;

	id_ = f.id_;
	ft_ = f.ft_;

	return *this;
}

Field* FieldDescription::defineField() const
{
	Grid* grid;
	string       gridType = id_.gridType_;
	if(DEBUG)
		cout << "FieldDescription::defineField -- grid Type " << gridType <<  endl;

	if(gridType == "sh")
       return new SpectralField(ft_.truncation_, ft_.parameter_, ft_.units_, ft_.editionNumber_, ft_.centre_, ft_.levelType_, ft_.level_, ft_.date_, ft_.time_, ft_.stepUnits_, ft_.startStep_, ft_.endStep_, ft_.bitsPerValue_, vector<double>());

	if(gridType == "latlon") {
		if(!id_.reduced_) {
			if(!ft_.area_.empty()) {
				if(id_.rotated_){
					grid = new RotatedRegularLatLon(ft_.area_,ft_.ns_,ft_.we_,ft_.southPole_.latitude(),ft_.southPole_.longitude());
				}
				else{
				//-----------------------------------------------------------
					if(ft_.cellCentered_){
						if(DEBUG)
							cout << "FieldDescription::defineField -- Regular LatLon CellCentered Area" << endl;
						if(ft_.nptsNS_ > 0 && ft_.nptsWE_ > 0)
							grid = new RegularLatLonCellCentered(ft_.area_,ft_.nptsNS_,ft_.nptsWE_);
						else{			
							if(ft_.ns_ > 0 && ft_.we_ > 0){
								grid = new RegularLatLonCellCentered(ft_.area_,ft_.ns_,ft_.we_);
							}
							else {
								throw UserError("FieldDescription::defineField -- RegularLatLonCellCentered inrements has to be specified");
							}
						}
					}
					else {
						if(ft_.nptsNS_ > 0 && ft_.nptsWE_ > 0){
							grid = new RegularLatLon(ft_.area_,ft_.nptsNS_,ft_.nptsWE_,ft_.globalWestEast_);
						}
						else{			
							if(ft_.ns_ > 0 && ft_.we_ > 0){
								grid = new RegularLatLon(ft_.area_,ft_.ns_,ft_.we_);
							}
							else {
								throw UserError("FieldDescription::defineField -- RegularLatLon inrements has to be specified");
							}
						}
					}
				}

				//-----------------------------------------------------------
			}
			else {
				if(id_.rotated_){
					grid = new RotatedRegularLatLon(ft_.ns_,ft_.we_,ft_.southPole_.latitude(),ft_.southPole_.longitude());
				}
				else {
					if(ft_.cellCentered_){
						if(DEBUG)
							cout << "FieldDescription::defineField -- Regular LatLon CellCentered No Area" << endl;
						if(ft_.nptsNS_ > 0 && ft_.nptsWE_ > 0)
							grid = new RegularLatLonCellCentered(ft_.nptsNS_,ft_.nptsWE_,ft_.shifted_);
						else{			
							if(ft_.ns_ > 0 && ft_.we_ > 0){
								grid = new RegularLatLonCellCentered(ft_.ns_,ft_.we_,ft_.shifted_);
							}
							else {
								throw UserError("FieldDescription::defineField -- RegularLatLonCellCentered inrements has to be specified");
							}
						}
					}
					else{
						if(ft_.nptsNS_ > 0 && ft_.nptsWE_ > 0){
							grid = new RegularLatLon(ft_.nptsNS_,ft_.nptsWE_);
						}
						else{			
							if(ft_.ns_ > 0 && ft_.we_ > 0){
								grid = new RegularLatLon(ft_.ns_,ft_.we_);
							}
							else {
								throw UserError("FieldDescription::defineField -- RegularLatLon inrements has to be specified");
							}
						}
					}
				}
			}
		}
		else {
			cout << "FieldDescription::defineField -- Reduced LatLon " << endl;
			throw UserError("FieldDescription::defineField -- Reduced LatLon");
		}
	}
	else if(gridType == "list") {
			if(DEBUG)
				cout << "FieldDescription::defineField -- List Of Points " << ft_.listOfPointsFileType_ << endl;
            Factory factory;
            auto_ptr<Input> f( factory.getInput(ft_.listOfPointsFile_,ft_.listOfPointsFileType_));
            vector<Point> points;
            f->getLatLonValues(points);
            grid = new ListOfPoints(points);
	}
	else if(gridType == "gaussian") {
			if(DEBUG)
				cout << "FieldDescription::defineField -- is Reduced Gaussian " << id_.reduced_ << endl;
			if(!id_.reduced_) {
				if(!ft_.area_.empty()) {
					grid = new RegularGaussian(ft_.area_, ft_.gaussianNumber_);
				}
				else {
					grid = new RegularGaussian(ft_.gaussianNumber_);
				}
			}
			else {
				if(!ft_.area_.empty()) {
					grid = new ReducedGaussian(ft_.area_, ft_.gaussianNumber_);
				}
				else {
					grid = new ReducedGaussian(ft_.gaussianNumber_);
				}
			}
	}
	else if(gridType == "projection") {
			if(id_.composedName_ == "polar_stereographic"){
				if(DEBUG)
					cout << "FieldDescription::defineField -- Polar Stereographic " << endl;
				grid = new PolarStereographic(ft_.projectionPredefines_);
			}
			else{
				throw UserError("FieldDescription::defineField -- UNKNOWN Projection");
			}
	}
	else {
		cout << "FieldDescription::defineField -- Not DECIDED " << endl;
		throw UserError("FieldDescription::defineField -- Not DECIDED");
	}

	if(DEBUG)
		cout << "FieldDescription::defineField -- Grid => " <<  *grid << endl;

	return new GridField(grid,ft_.scanningMode_,ft_.frameNumber_,ft_.bitmap_,ft_.bitmapFile_,ft_.bitsPerValue_,ft_.editionNumber_,ft_.missingValue_);
}

/* API */

void FieldDescription::grid2gridTransformationType(const string& s)
{
	ft_.grid2gridTransformationType_ = s;
	if(DEBUG)
		cout << "Set-> Grid To Grid Transformation Type => " << s << endl;
}

void FieldDescription::parameter(const Parameter& param)
{
	ft_.parameter_ = param;
	if(DEBUG)
		cout << "Set->  Parameter => " << param << endl;
}

void FieldDescription::area(double north, double west, double south, double east)
{
	ft_.area_ = Area(north,west,south,east);
	if(DEBUG)
		cout << "Set->  Area => North: " << north << " West: " << west << " South: " << south << " East: " << east << endl;
}

void FieldDescription::projectionPredefines(int number)
{
	ft_.projectionPredefines_ = number;

	if(DEBUG)
		cout << "Set->  Projection Predefines =>" << number << endl;
}

void FieldDescription::numberOfPoints(long ns, long we)
{

	ft_.nptsWE_       = we;
	ft_.nptsNS_       = ns;
	ft_.decide_         = true;
	bool   gridOrSpectral = true;
	string gridType       = "latlon";
	id_ = FieldIdentity(gridOrSpectral, gridType, id_.reduced_, id_.stretched_, id_.rotated_);
/*
*/
	if(DEBUG)
		cout << "Set->  Number Of Points => West-East: " << we << " North-South: " << ns << endl;
}

void FieldDescription::globalWestEast(const string& str)
{
	if(str == "y" || str == "yes"){
		ft_.globalWestEast_ = true;
		if(DEBUG)
			cout << "Set->  Grid is global West-East " << endl;
	}
	else{
		ft_.globalWestEast_ = false;
		if(DEBUG)
			cout << "Set->  Grid is NOT global West-East " << endl;
	}
	ft_.isSetglobalWestEast_ = true;
}

void FieldDescription::increments(double we, double ns)
{

	ft_.decide_         = true;
	ft_.we_             = we;
	ft_.ns_             = ns;

	bool   gridOrSpectral = true;
	string gridType       = "latlon";
	id_ = FieldIdentity(gridOrSpectral, gridType, id_.reduced_, id_.stretched_, id_.rotated_);

//	cout << "Set->  id_ " << id_ << endl;
	if(DEBUG)
		cout << "Set->  Lat/Lon Increments => West-East: " << we << " North-South: " << ns << endl;
}

void FieldDescription::truncation(int number)
{
	ft_.decide_         = true;
	ft_.truncation_     = number;

	bool   gridOrSpectral = false;
	string gridType       = "sh";
	id_ = FieldIdentity(gridOrSpectral, gridType, id_.reduced_, id_.stretched_, id_.rotated_);
	if(DEBUG)
		cout << "Set->   Spectral truncation: " << number << endl;
}

void FieldDescription::gaussianNumber(int number)
{
	ft_.decide_         = true;
	ft_.gaussianNumber_ = number;

	bool   gridOrSpectral = true;
	string gridType       = "gaussian";
	id_ = FieldIdentity(gridOrSpectral, gridType, id_.reduced_, id_.stretched_, id_.rotated_);

	if(DEBUG)
		cout << "Set->  Gaussian Number: " << number << endl;
}

void FieldDescription::numberOfNearestPoints(int number)
{
	ft_.numberOfNearestPoints_  = number;

	if(DEBUG)
		cout << "Set->  Number of Neighbouring Points: " << number << endl;
}

void FieldDescription::southPole(double lat, double lon)
{
	ft_.southPole_ = Point(lat,lon);
    bool rotated = (!iszero(lat) || !iszero(lon));
	id_ = FieldIdentity(id_.gridOrSpectral_, id_.gridType_, id_.reduced_, id_.stretched_, rotated);
	if(DEBUG)
		cout << "Set->  South Pole of Rotation =>  Latitude: " << lat << " Longitude: " << lon << endl;
}

void FieldDescription::centre(int c)
{
	ft_.centre_ = c;
	if(DEBUG)
		cout << "Set-> Centre: " << c << endl;
}

void FieldDescription::editionNumber(int c)
{
	ft_.editionNumber_ = c;
	if(DEBUG)
		cout << "Set-> editionNumber: " << c << endl;
}

void FieldDescription::units(const string& u)
{
	ft_.units_ = u;
	if(DEBUG)
		cout << "Set-> units: " << u << endl;
}

void FieldDescription::stepUnits(const string& step)
{
	ft_.stepUnits_ = step;
	if(DEBUG)
		cout << "Set-> Step Units: " << step << endl;
}

void FieldDescription::startStep(int step)
{
	ft_.startStep_ = step;
	if(DEBUG)
		cout << "Set-> Start Step: " << step << endl;
}

void FieldDescription::endStep(int step)
{
	ft_.endStep_ = step;
	if(DEBUG)
		cout << "Set-> End Step: " << step << endl;
}

void FieldDescription::scanningMode(int scanningMode)
{
	ft_.scanningMode_ = scanningMode;
	if(DEBUG)
		cout << "Set-> Scanning Mode: " << scanningMode << endl;
}

void FieldDescription::isAvailable()
{
	id_.isAvailable();
	if(DEBUG)
		cout << "Set-> isAvailable" << endl;
}

void FieldDescription::ifAvailableCopyBasics(const FieldDescription& other)
{
	id_.isAvailable();
	if(DEBUG)
		cout << "Set-> isAvailable" << endl;

	ft_.copyBasics(other.ft_);
}

void FieldDescription::isReduced(bool isReduced)
{
//	ft_.decide_         = true;
	ft_.isReduced_      = isReduced;

	bool   gridOrSpectral = true;
	id_ = FieldIdentity(gridOrSpectral, id_.gridType_, isReduced, id_.stretched_, id_.rotated_);

	if(isReduced == true)
    {
		if(DEBUG)
            cout << "Set->  Grid is REDUCED" << endl;
    }
	else
    {
		if(DEBUG)
			cout << "Set->  Grid is REGULAR" << endl;
    }
}

void FieldDescription::gridSpec(const string& spec)
{
	if(spec == "regular")
    {
		ft_.isReduced_ = false;
		if(DEBUG)
			cout << "Set->  Grid is REGULAR" << endl;
	}
	else if(spec == "reduced")
    {
		ft_.isReduced_ = true;
		if(DEBUG)
			cout << "Set->  Grid is REDUCED" << endl;
	}
	else if(spec == "cell-centred" || spec == "cellcentred" )
    {
		ft_.isReduced_ = false;
		bool shifted = true;
		cellCentered (true, shifted);
		id_.gridType_ = "latlon";
		ft_.decide_   = true;
		if(DEBUG)
			cout << "Set-> Regular Cell Centered Field;" << endl;
	}
	else
    {
		if(DEBUG)
			cout << "Set->  Grid is REGULAR as default" << endl;
		ft_.isReduced_ = false;
	}

    // reinstating this as we are in the situation where we end up with
    // undetermined grids - e.g. if we allow setting "reduced" without
    // saying what sort of reduced grid it is.
    //
	ft_.decide_         = true;

	bool   gridOrSpectral = true;
	id_ = FieldIdentity(gridOrSpectral, id_.gridType_, ft_.isReduced_, id_.stretched_, id_.rotated_);

}

void FieldDescription::vdConversion(bool conversion)
{
//	ft_.decide_         = true;
//	bool   gridOrSpectral = true;
//	id_ = FieldIdentity(gridOrSpectral, id_.gridType_, isReduced, id_.stretched_, id_.rotated_);

	ft_.vdConversion_   = conversion;

	if(conversion == true)
    {
		if(DEBUG)
			cout << "Set->  Conversion from vort,div to u,v ENABLED" << endl;
    }
    else
    {
		if(DEBUG)
			cout << "Set->  Conversion from vort,div to u,v DISABLED" << endl;
    }
}

void FieldDescription::gridType(const string& type)
{
	id_.setFromTypeOfGrid(type);
	ft_.decide_ = true;
	if(DEBUG)
		cout << "Set->  Grid Type: " << type << endl;
}

void FieldDescription::levelType(const string& levelType)
{
	ft_.levelType_ = levelType;
	if(DEBUG)
		cout << "Set->  Level Type: " << levelType << endl;
}

void FieldDescription::level(int level)
{
	ft_.level_ = level;
	if(DEBUG)
		cout << "Set->  Level: " << level << endl;
}

void FieldDescription::date(int date)
{
	ft_.date_ = date;
	if(DEBUG)
		cout << "Set->  Date: " << date << endl;
}

void FieldDescription::bitsPerValue(int b)
{
	ft_.bitsPerValue_ = b;
	if(DEBUG)
		cout << "Set->  Bits Per Value: " << b << endl;
}

void FieldDescription::time(int time)
{
	ft_.time_ = time;
	if(DEBUG)
		cout << "Set->  Time: " << time << endl;
}

void FieldDescription::reducedGridDefinition(long* rgridDef, size_t size)
{
	ft_.reducedGridLatNumber_ = size;
	ft_.reducedGridDefinition_ = rgridDef;
}

void FieldDescription::bitmapFile(const string& file)
{
	ft_.bitmapFile_ = file;
	ft_.bitmap_     = true;
	if(DEBUG)
		cout << "Set->  Bitmap File: " << file << endl;
}

void FieldDescription::frameNumber(int number)
{
	ft_.frameNumber_ = number;
	ft_.frame_       = true;
	if(DEBUG)
		cout << "Set->  Frame Number: " << number << endl;
}

void FieldDescription::parameterId(int number)
{
	ft_.parameterId_ = number;
	if(DEBUG)
		cout << "Set->  parameterId: " << number << endl;
}

void FieldDescription::table(int number)
{
	ft_.table_ = number;
	if(DEBUG)
		cout << "Set->  table: " << number << endl;
}

void FieldDescription::global(bool b)
{
	ft_.global_ = b;
	if(DEBUG){
		if(b)
			cout << "Set-> field is global: "<< endl;
		else
			cout << "Set-> field is not global: "<< endl;
	}
}

void FieldDescription::fftMax(int number)
{
	ft_.fftMax_ = number;
	if(DEBUG)
		cout << "Set->  FFT max block size: " << number << endl;
}

void FieldDescription::listOfPointsFileType(const string& listFileType)
{
	ft_.listOfPointsFileType_ = listFileType;
	if(DEBUG)
		cout << "Set->  List of Points File Type: " << listFileType << endl;
}

void FieldDescription::listOfPointsFile(const string& file)
{
	ft_.listOfPointsFile_ = file;

	id_.gridType_     = "list";
	id_.composedName_ = "list";
	ft_.decide_       = true;
	if(DEBUG)
		cout << "Set->  List of Points File: " << file << endl;
}

void FieldDescription::interpolationMethod(const string& method)
{
	ft_.interpolationMethod_ = method;

	if(DEBUG)
		cout << "Set->  Interpolation Method: " << method << endl;
}

void FieldDescription::lsmMethod(const string& method)
{
	ft_.lsmMethod_ = method;

	if(DEBUG)
		cout << "Set->  Lsm Method: " << method << endl;
}

void FieldDescription::auresol(const string& wether)
{
	
	bool yn = true;
	if(wether == "off")
		yn = false;

	ft_.auresol_ = yn;

	if(DEBUG)
		cout << "Set->  Auresol to: " << yn << endl;
}

void FieldDescription::legendrePolynomialsMethod(const string& method)
{
	ft_.legendrePolynomialsMethod_ = method;

	if(DEBUG)
		cout << "Set->  Legendre Polynomials Method: " << method << endl;
}

void FieldDescription::cellCentered(bool a,bool b)
{
	ft_.cellCentered_ = a;
	ft_.shifted_      = b;

	if(DEBUG){
		if(b)
			cout << "Set->  Grid is cell Centered " << endl;
		else
			cout << "Set->  Grid is NOT cell Centered " << endl;
	}
}

void FieldDescription::pseudoGaussian(bool b)
{
	ft_.pseudoGaussian_ = b;

	if(DEBUG){
		if(b)
			cout << "Set->  Grid is Pseudo Gaussian" << endl;
		else
			cout << "Set->  Grid is NOT Pseudo Gaussian" << endl;
	}
}

void FieldDescription::shifted(bool b)
{
	ft_.shifted_ = b;

	if(DEBUG){
		if(b)
			cout << "Set->  Grid is shifted " << endl;
		else
			cout << "Set->  Grid is NOT shifted " << endl;
	}
}

void FieldDescription::extrapolateOnPole(const string& s)
{
	ft_.extrapolateOnPole_ = s;
	if(DEBUG)
		cout << "Set-> Extrapolate values on Poles  => " << s << endl;
}

void FieldDescription::fileType(const string& s)
{
	ft_.fileType_ = s;
	if(DEBUG)
		cout << "Set-> File Type => " << s << endl;
}

void FieldDescription::missingValue(double value )
{
	ft_.missingValue_ = value;
	ft_.bitmap_       = true;
	if(DEBUG)
		cout << "Set-> Missing value => " << value << endl;
}

/* end API */


void FieldDescription::print(ostream&) const
{
	if(DEBUG)
		cout << "["<< id_ << "]  [," << ft_ << endl;
}
