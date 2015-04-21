/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "GridToSpectralTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef Wind_H
#include "Wind.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef LegendreTransformCoefficientsToSpectral_H
#include "LegendreTransformCoefficientsToSpectral.h"
#endif

#ifndef DirectFastFourierTransform_H
#include "DirectFastFourierTransform.h"
#endif

GridToSpectralTransformer::GridToSpectralTransformer(const string& legendrePolynomialsMethod, int fftMax) :
    legendrePolynomialsMethod_(legendrePolynomialsMethod),fftMaxBlockSize_(fftMax)

{
}

GridToSpectralTransformer::~GridToSpectralTransformer() {
}

typedef map<int,int,less<int> > mapBlox;

Field* GridToSpectralTransformer::transform(const Field& in, const Field& out) const {
    const GridField&     input  = dynamic_cast<const GridField&>(in);
    const SpectralField& output = dynamic_cast<const SpectralField&>(out);

// ssp is it neccessary
    Grid* possibleGrid   = input.grid().newGrid(input.grid());

    long  inputLength      = input.dataLength();
    long  calculatedLength = possibleGrid->calculatedNumberOfPoints();
    ASSERT(inputLength == calculatedLength);

    const vector<double>& data = input.data();

    int truncation = output.truncation();
// auresol
//		truncation = input.truncate(truncation);
    Factory f;
    ref_counted_ptr<const LegendrePolynomials> legendrePolynomialsMethod = f.polynomialsMethod(legendrePolynomialsMethod_,truncation,input.grid());

    cout << "GridToSpectralTransformer::transform " << legendrePolynomialsMethod_ << endl;

    vector<long> gridDef;
    size_t latitudeNumberFromDef = possibleGrid->getGridDefinition(gridDef);
    vector<double> latitudes;
    possibleGrid->latitudes(latitudes);
    int southIndex      = possibleGrid->southIndex(0);

// to fill that
    int nfc = (truncation + 1) * 2;
    vector<int> offsets;
    for ( size_t j = 0 ; j < latitudeNumberFromDef*2 ;  j++ )
        offsets.push_back(j * nfc);

    mapBlox blox;
    bool reduced = input.grid().reduced();
    if(reduced) {
        long num = gridDef[0];
        int nn    = 0;
        int ss    = southIndex;
        cout << "southIndex  " << southIndex  << endl;
        for ( int j = 0 ; j <= southIndex ;  j++ ) {
            if(gridDef[j] != num) {
                int ssin = j - 1;
//				cout << "SpectralToGridTransformer::transform num: " << num << " nn: " << nn  <<  "  ss: " << ssin << endl;
                blox.insert( pair<int,int> (nn,ssin) );
                num   = gridDef[j];
                nn    = j;
            }
        }
        blox.insert( pair<int,int> (nn,ss) );
    } else {
        blox.insert( pair<int,int> (0,southIndex) );
    }

    vector<double> fourierCoeff(nfc * latitudeNumberFromDef * 2);
    mapBlox::const_iterator iter = blox.begin(), stop = blox.end();
    for ( ; iter != stop; ++iter) {
        // FFT
        int lonNumber = 0;
        if(reduced)
            lonNumber = gridDef[iter->first];
        else
            lonNumber = possibleGrid->westEastNumberOfPoints();
        int lot = iter->second - iter->first + 1;
        cout << "GridToSpectralTransformer::transform  lot: " << lot << " truncation " << truncation << " lonNumber " << lonNumber <<  endl;
        DirectFastFourierTransform fourier(truncation,lonNumber,lot,iter->first,iter->second);
        fourier.transform(offsets,fourierCoeff,data);
    }

    long calculatedLengthOut = output.calculatedLength();
    vector<double> values(calculatedLengthOut);
    cout << "GridToSpectralTransformer::transform  values length: " << calculatedLengthOut <<  endl;
    // Legendre
    LegendreTransformCoefficientsToSpectral legendre(legendrePolynomialsMethod,truncation,latitudes);
    //int lot1 = legendre.transform(fourierCoeff,values,latitudeNumberFromDef);
// ssp supposed to be SpectralField
    return new GridField( possibleGrid, input,16,1, 0,false, values,input.missingValue());
//	return new GridField( possibleGrid, input->parameter(), input->levelType(), input->level(), 0, calculatedLengthOut,values);
}

Wind* GridToSpectralTransformer::transformVector(const Field& inU, const Field& inV, const Field& req) const {
    if(DEBUG)
        cout << " SpectralToSpectralTransformer::transformWind  ((((( " << endl ;

    Field* uOut = transform(inU,req);
    Field* vOut = transform(inV,req);
    /*
    	if(inU.number() == 138 && inV.number() == 155 && vdConversion_ ) {
    		 SpectralToSpectralTransformer sp2sp;
    		 sp2sp.vorticityDivergenceToUV(inU,inV);

    		auto_ptr<SpectralField> reqUU( dynamic_cast<SpectralField*>(reqU));
    		Wind in(inU,inV);
    		return in.vorticityDivergenceToUV(reqUU->truncation());
    	}
    */

    return new Wind(uOut,vOut);
}

/*
void GridToSpectralTransformer::print(ostream&) const
{
}
*/
