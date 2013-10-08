/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GridToGridTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef Wind_H
#include "Wind.h"
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

#ifndef Interpolator_H
#include "Interpolator.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef DerivedSubgridParameters_H
#include "DerivedSubgridParameters.h"
#endif

#ifndef SubArea_H
#include "SubArea.h"
#endif

#ifndef Extraction_H
#include "Extraction.h"
#endif

#ifndef PartialDerivatives_H
#include "PartialDerivatives.h"
#endif

GridToGridTransformer::GridToGridTransformer(const string& intMethod, const string& lsmMethod, int nptsInt, const string& type, const string& extrapolate, double missingValue) :
    interpolationMethod_(intMethod), 
    lsmMethod_(lsmMethod), 
    pointsForInterpolation_(nptsInt), 
    type_(type), 
    extrapolate_(extrapolate), 
    missingValue_(missingValue)
{
}

GridToGridTransformer::~GridToGridTransformer()
{
}

Field* GridToGridTransformer::transform(const Field& in, const Field& out) const
{
	const GridField& input  = dynamic_cast<const GridField&>(in);
	const GridField& output = dynamic_cast<const GridField&>(out);

	if ((input == output) && (type_ != "zonalderivativ" && type_ != "meridionalderivativ")) {
		cout << "---------------------------------------------------"  <<  endl;
		cout << "||| GridToGridTransformer::transform Input and Output field are the SAME! |||"  <<  endl;
		cout << "---------------------------------------------------"  <<  endl;
		if(IS_SET("ECREGRID_FORCE_INTERPOLATION")){
			cout << "GridToGridTransformer::transform FORCE INTERPOLATION" << endl;
		}
		else{
			return 0;
		}
	}

	double missingValue = output.missingValue();
	if(input.bitmap())
		missingValue = input.missingValue();

	Factory factory;
	auto_ptr<Extraction> extraction(factory.multiExtraction(output,missingValue));

	Grid* possibleGrid   = output.grid().newGrid(input.grid());

	if( *possibleGrid == output.grid() && !output.grid().area().empty()) {
		if(DEBUG){
			cout << "************************************************************************************************************" << endl;
			cout << "*** GridToGridTransformer::transform - Required  Output Area is NOT changed => "  << output.grid().area() <<   endl;
			cout << "************************************************************************************************************" << endl;
		}
	}
	else {
		if(DEBUG){
			cout << "************************************************************************************************************" << endl;
			cout << "*** GridToGridTransformer::transform  - Output Area changed to => "  << possibleGrid->area() <<   endl;
			cout << "************************************************************************************************************" << endl;
		}
	}

    const long valuesSize = possibleGrid->calculatedNumberOfPoints();
    // CACHING OPPORTUNITY:
    vector<double> values(valuesSize);

	if(DEBUG)
		cout << "GridToGridTransformer::transform possible Grid " << *possibleGrid << endl;


	bool bitmap = input.bitmap();
	if(input.grid().equals(*possibleGrid) && (type_ != "zonalderivativ" && type_ != "meridionalderivativ")){
		SubArea subarea(possibleGrid->area());
//		SubArea subarea(output.grid().area());
		subarea.extract(input,values);		
		if(DEBUG){
			cout << "****************************************************" << endl;
			cout << "GridToGridTransformer::transform Sub Area extraction " << endl;
			cout << "****************************************************" << endl;
		}
		if(extraction.get()){	
			bitmap = true;
			extraction->extract(*possibleGrid, values);
			if(DEBUG)
				cout << "GridToGridTransformer::transform Extraction on top of Sub Area extraction" << *extraction << endl;
		}
	}
	else{
		bitmap = transform(input, *possibleGrid, extraction.get(), values);
	}

	return new GridField(possibleGrid, input, output.bitsPerValue(), output.editionNumber(), output.scanningMode(), bitmap, values, missingValue);
}

bool GridToGridTransformer::transform(const GridField& input, const Grid& outGrid, const Extraction* extraction, vector<double>& values) const
{
	Factory factory;
	bool bitmap = input.bitmap();
    // We assume the input vector has been sized

	if(input.grid() == outGrid) {
        const vector<double>& data = input.data();
        values = data;

		if(extraction){	
			bitmap = true;
            extraction->extract(input.grid(), values);
			if(DEBUG)
				cout << "GridToGridTransformer::transform Extraction without Interplation - " << *extraction << endl;
		}
	}
	else{
		if(DEBUG)
			cout << "GridToGridTransformer::transform Requested Interpolation method: "  << interpolationMethod_ <<  endl;

		bool applyLsm = input.lsm();
		if (DISABLE_LSM || lsmMethod_ == "off")
       		applyLsm = false;
		string lsmSource = lsmMethod_;
    	char* lsmMethod = getenv("ECREGRID_LSM_SOURCE");
    	if(lsmMethod){
			lsmSource = string(lsmMethod);
    	}


		if( type_ == "meridionalderivativ"){
			meridionalPartialDerivatives(input,values);
			return false;
		}

		if( type_ == "zonalderivativ"){
			zonalPartialDerivatives(input,values);
			return false;
		}
		if(DEBUG)
			cout << "GridToGridTransformer::transform Lsm Source: "  << lsmSource <<  endl;
		auto_ptr<Interpolator>method(factory.interpolationMethod(interpolationMethod_, pointsForInterpolation_,input,outGrid,applyLsm,lsmSource,extrapolate_,missingValue_,bitmap));
		if(type_ != "interpolation") {
			if( type_ == "stddev") {
				method->standardDeviation(input, outGrid, values);
			}
			else{
				auto_ptr<DerivedSubgridParameters> derived(factory.selectDerivedSubgridParameter(type_));
				method->derivedSubgridParameters(input, outGrid, values, *derived);
			}
		}
		else{
			method->interpolate(input, outGrid, values);
		}

		if(extraction) {
			bitmap = true;
			extraction->extract(outGrid, values);
		}
	}
	return bitmap;
}

Wind* GridToGridTransformer::transformVector(const Field& inU, const Field& inV, const Field& out) const
{
	const GridField& inputU  = dynamic_cast<const GridField&>(inU);
	const GridField& inputV  = dynamic_cast<const GridField&>(inV);
	const GridField& output  = dynamic_cast<const GridField&>(out);

	if (inputU == output && inputV == output) {
			cout << "---------------------------------------------------"  <<  endl;
			cout << "||| GridToGridTransformer::transformVector Input and Output U,V field are the SAME! |||   "  <<  endl;
			cout << "---------------------------------------------------"  <<  endl;
		return new Wind();
	}

	double missingValue = output.missingValue();
	if(inputU.bitmap())
		missingValue = inputU.missingValue();
	Factory factory;
	auto_ptr<Extraction> extraction(factory.multiExtraction(output,missingValue));

	Grid* possibleGrid   = output.grid().newGrid(inputU.grid());
	Grid* possibleGridV   = output.grid().newGrid(inputV.grid());

	if( *possibleGrid == output.grid() && !output.grid().area().empty()) {
		if(DEBUG){
			cout << "************************************************************************************************************" << endl;
			cout << "*** GridToGridTransformer::transformVector - Required  Output Area is NOT changed => "  << output.grid().area() <<   endl;
			cout << "************************************************************************************************************" << endl;
		}
	}
	else {
		if(DEBUG){
			cout << "************************************************************************************************************" << endl;
			cout << "*** GridToGridTransformer::transformVector  - Output Area changed to => "  << possibleGrid->area() <<   endl;
			cout << "************************************************************************************************************" << endl;
		}
	}

    // CACHING OPPORTUNITY:
    const size_t valuesSize = possibleGrid->calculatedNumberOfPoints();
    vector<double> valuesU(valuesSize);
    vector<double> valuesV(valuesSize);


	if(DEBUG)
		cout << "GridToGridTransformer::transform possible Grid " << *possibleGrid << endl;
	bool bitmapU = transform(inputU, *possibleGrid, extraction.get(), valuesU);
	bool bitmapV = transform(inputV, *possibleGridV, extraction.get(), valuesV);
	 if(output.isRotated()){
		vector<double> orientationAngles;
		orientationAngles.reserve(valuesSize);	
		possibleGrid->angularChange(orientationAngles);
		if(DEBUG)
			cout << "GridToGridTransformer::transformWind Rotated output apply angular Change " << endl;
		for (size_t i = 0 ; i < valuesSize ; i++) {
			double radian = -(orientationAngles[i] * 0.0174532925);
			double c = cos(radian);
			double s = sin(radian);
//			cout << "orientationAngles " << i << "  " << orientationAngles[i]<< endl;
			double utemp = valuesU[i];
			double vtemp = valuesV[i];
			valuesU[i] = utemp*c - vtemp*s;
			valuesV[i] = utemp*s + vtemp*c;
		}
//	cout << "GridToGridTransformer::transformWind valuesSize " << valuesSize << endl;
	}
	GridField* uOut = new GridField(possibleGrid,inputU,output.bitsPerValue(),output.editionNumber(),output.scanningMode(),bitmapU,valuesU, missingValue);
	ASSERT(uOut);
	GridField* vOut = new GridField(possibleGridV,inputV, output.bitsPerValue(), output.editionNumber(),output.scanningMode(),bitmapV, valuesV, missingValue);
	ASSERT(vOut);
//	uOut->dump2file("./u.txt");
//	vOut->dump2file("./v.txt");
	if(DEBUG){
		cout << "GridToGridTransformer::transformWind Interpolated field U " << *uOut << endl;
		cout << "GridToGridTransformer::transformWind Interpolated field V " << *vOut << endl;
	}

	return new Wind(uOut,vOut);
}

void GridToGridTransformer::zonalPartialDerivatives(const GridField& input, vector<double>& values) const
{
    const vector<double>&  data = input.data();

    int inScMode = input.scanningMode();
    double inMissingValue = input.missingValue();

    if(DEBUG){
        cout << "****************************************************" << endl;
        cout << "*** GridToGridTransformer::zonalDerivatives - " <<   endl;
        cout << "****************************************************" << endl;
        cout << " GridToGridTransformer::zonalDerivatives input Scanning Mode:  " << inScMode << endl;
    }

    vector<Point> inputPoints;
    input.grid().generateGrid1D(inputPoints);
    size_t inSize = inputPoints.size();

    PartialDerivatives derivatives;

    derivatives.zonalDerivatives(inputPoints,input.grid(),inScMode,data,values,inMissingValue);
}

void GridToGridTransformer::meridionalPartialDerivatives(const GridField& input, vector<double>& values) const
{
    const vector<double>&  data = input.data();

    int inScMode = input.scanningMode();
    double inMissingValue = input.missingValue();

    if(DEBUG){
        cout << "****************************************************" << endl;
        cout << "*** GridToGridTransformer::meridionalPartialDerivatives - " <<   endl;
        cout << "****************************************************" << endl;
        cout << " GridToGridTransformer::meridionalPartialDerivatives input Scanning Mode:  " << inScMode << endl;
    }

    vector<Point> inputPoints;
    input.grid().generateGrid1D(inputPoints);
    size_t inSize = inputPoints.size();

    PartialDerivatives derivatives;

    derivatives.meridionalDerivatives(inputPoints,input.grid(),inScMode,data,values,inMissingValue);
}




