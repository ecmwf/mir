/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "SpectralToSpectralTransformer.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Field_H
#include "Field.h"
#endif

#ifndef SpectralField_H
#include "SpectralField.h"
#endif

#ifndef Wind_H
#include "Wind.h"
#endif


SpectralToSpectralTransformer::SpectralToSpectralTransformer(bool conversion) :
    vdConversion_(conversion) {
}

SpectralToSpectralTransformer::~SpectralToSpectralTransformer() {
}

Field* SpectralToSpectralTransformer::transform(const Field& in, const Field& out) const {
    if(DEBUG)
        cout << " SpectralToSpectralTransformer::transform  ((((( " << endl ;

    const SpectralField& input  = dynamic_cast<const SpectralField&>(in);
    const SpectralField& output = dynamic_cast<const SpectralField&>(out);

    int inTrunc  = input.truncation();
    int outTrunc = output.truncation();
    if(DEBUG) {
        cout << " SpectralToSpectralTransformer::transform  in Trunc: " << inTrunc << " out Trunc: " << outTrunc << endl ;
    }

    if (input == output) {
//      if(DEBUG){
        cout << "---------------------------------------------------"  <<  endl;
        cout << "||| SpectralToSpectralTransformer::transform - Input and Output field are the SAME! |||"  <<  endl;
//            cout << "||| Input and Output field are the SAME! |||"  <<  endl;
        cout << "---------------------------------------------------"  <<  endl;
//     }

        return 0;
    }

    if(outTrunc > inTrunc)
        throw UserError("SpectralToSpectralTransformer::transform Output Truncation "+outTrunc," bigger than Input Truncation"+inTrunc);

    long dataLengthIn     = input.dataLengthComplex();
    vector<comp> valuesIn;
    valuesIn.reserve(dataLengthIn);
    input.getDataAsComplex(valuesIn);

    long  outSize = output.calculatedLength();

    if(DEBUG) {
        cout << " SpectralToSpectralTransformer::transform outSize ((((( " << outSize << endl;
        cout << " SpectralToSpectralTransformer::transform dataLengthIn ((((( " << dataLengthIn << endl;
    }

    int  truncationDiff = inTrunc - outTrunc;
    long inCount =  0, outCount = 0;
    int  toup = outTrunc + 1;

    // CACHING OPPORTUNITY:
    vector<double> values(outSize);

    for ( int j = 0 ; j < toup ;  j++ ) {
        for ( int i = j ; i < toup ;  i++ ) {
            values[outCount++] = valuesIn[inCount].real();
            values[outCount++] = valuesIn[inCount++].imag();
        }
        inCount += truncationDiff;
    }

    return  new SpectralField(outTrunc, input.parameter(), input.units(), input.editionNumber(), input.centre(), input.levelType(), input.level(), input.date(), input.time(), input.stepUnits(), input.startStep(), input.endStep(), output.bitsPerValue(), values);



}

bool SpectralToSpectralTransformer::transform(const SpectralField& input, int outTrunc, vector<double>& values) const {
    if(DEBUG)
        cout << " SpectralToSpectralTransformer::transform  ((((( " << endl ;

    // we assume input has been allocated
    const size_t valuesSize = values.size();

    int inTrunc  = input.truncation();

    if(DEBUG) {
        cout << " SpectralToSpectralTransformer::transform  in Trunc: " << inTrunc << " out Trunc: " << outTrunc << endl ;
    }

    if(outTrunc == inTrunc) {
        if(DEBUG) {
            cout << "---------------------------------------------------"  <<  endl;
            cout << "||| SpectralToSpectralTransformer::transform - Input and Output field are the SAME! |||"  <<  endl;
            cout << "---------------------------------------------------"  <<  endl;
        }
        return false ;
    }

    if(outTrunc > inTrunc)
        throw UserError("SpectralToSpectralTransformer::transform Output Truncation "+outTrunc," bigger than Input Truncation"+inTrunc);

    long dataLengthIn     = input.dataLengthComplex();
    vector<comp> valuesIn;
    valuesIn.reserve(dataLengthIn);
    input.getDataAsComplex(valuesIn);

    if(DEBUG) {
        cout << " SpectralToSpectralTransformer::transform outSize ((((( " << valuesSize << endl;
        cout << " SpectralToSpectralTransformer::transform dataLengthIn ((((( " << dataLengthIn << endl;
    }

    int  truncationDiff = inTrunc - outTrunc;
    long inCount =  0, outCount = 0;
    int  toup = outTrunc + 1;

    for ( int j = 0 ; j < toup ;  j++ ) {
        for ( int i = j ; i < toup ;  i++ ) {
            values[outCount++] = valuesIn[inCount].real();
            values[outCount++] = valuesIn[inCount++].imag();
        }
        inCount += truncationDiff;
    }
    return true;
}

void SpectralToSpectralTransformer::transform(const SpectralField& input, int outTrunc, vector<comp>& compOut) const {
    compOut.clear();
    if(DEBUG)
        cout << " SpectralToSpectralTransformer::transform  ((((( " << endl ;

    int inTrunc  = input.truncation();

    if(DEBUG) {
        cout << " SpectralToSpectralTransformer::transform  in Trunc: " << inTrunc << " out Trunc: " << outTrunc << endl ;
    }

    if(outTrunc == inTrunc) {
        if(DEBUG) {
            cout << "---------------------------------------------------"  <<  endl;
            cout << "||| SpectralToSpectralTransformer::transform - Input and Output field are the SAME! |||"  <<  endl;
//            cout << "||| Input and Output field are the SAME! |||"  <<  endl;
            cout << "---------------------------------------------------"  <<  endl;
        }
        long dataLengthIn     = input.dataLengthComplex();
        compOut.reserve(dataLengthIn);
        input.getDataAsComplex(compOut);
        return;
    }

    if(outTrunc > inTrunc)
        throw UserError("SpectralToSpectralTransformer::transform Output Truncation "+outTrunc," bigger than Input Truncation"+inTrunc);

    long dataLengthIn     = input.dataLengthComplex();
    vector<comp> valuesIn;
    valuesIn.reserve(dataLengthIn);
    input.getDataAsComplex(valuesIn);

//	long  outSize = output.calculatedLength();
    int  toup = outTrunc + 1;
    long  outSize = ((toup + 1) * (toup + 2))/2;
    compOut.reserve(outSize);

    if(DEBUG) {
        cout << " SpectralToSpectralTransformer::transform Input Size: " << dataLengthIn << " Output Size: " << outSize  << endl;
    }

    int  truncationDiff = inTrunc - outTrunc;
    long inCount =  0;
    for ( int j = 0 ; j < toup ;  j++ ) {
        for ( int i = j ; i < toup ;  i++ ) {
            compOut.push_back(valuesIn[inCount++]);
        }
        inCount += truncationDiff;
    }

    ASSERT(compOut.size());
}

Wind* SpectralToSpectralTransformer::transformVector(const Field& inU, const Field& inV, const Field& req) const {
    if(DEBUG)
        cout << " SpectralToSpectralTransformer::transformVector  ((((( " << endl ;

    const SpectralField& inputU = dynamic_cast<const SpectralField&>(inU);
    const SpectralField& inputV = dynamic_cast<const SpectralField&>(inV);

    const SpectralField& requested = dynamic_cast<const SpectralField&>(req);

    long outSize =  requested.calculatedLength();

    // CACHING OPPORTUNITY:
    vector<double> valuesU(outSize);
    vector<double> valuesV(outSize);

    int outTruncation = requested.truncation();

    if(inU.number() == 138 && inV.number() == 155 && vdConversion_) {
        vector<comp> vort;
        vector<comp> dive;
        transform(inputU,outTruncation - 1,vort);
        transform(inputV,outTruncation - 1,dive);
        if(DEBUG)
            cout << " SpectralToSpectralTransformer::transformVector VORT,DIV TO U,V  " << outTruncation << endl ;
//		cout << " Vort size " << vort.size() << " dive size " << dive.size() << endl;
        ASSERT(vort.size() && dive.size());
        vector<comp> ucomp;
        vector<comp> vcomp;
        vorticityDivergenceToUV(vort,dive,outTruncation,ucomp,vcomp);
        //----------------------- U & V -----------------------------
        long complexOutSize = outSize / 2;
        long j = 0, jj = 0;
        for ( int i = 0 ; i < complexOutSize ;  i++ ) {
            valuesU[j++]  = ucomp[i].real();
            valuesU[j++]  = ucomp[i].imag();
            valuesV[jj++] = vcomp[i].real();
            valuesV[jj++] = vcomp[i].imag();
        }
    } else {
        bool sameU = transform(inputU,outTruncation,valuesU);
        bool sameV = transform(inputV,outTruncation,valuesV);

        if(!sameU && !sameV) {
            return new Wind();
        }

    }

    SpectralField* u =  new SpectralField(outTruncation, Parameter(131,128,inputU.levelType()), inputU.units(), inputU.editionNumber(), inputU.centre(), inputU.levelType(), inputU.level(), inputU.date(), inputU.time(), inputU.stepUnits(), inputU.startStep(), inputU.endStep(), requested.bitsPerValue(), valuesU);

    SpectralField* v =  new SpectralField(outTruncation, Parameter(132,128,inputV.levelType()), inputV.units(), inputV.editionNumber(), inputV.centre(), inputV.levelType(), inputV.level(), inputV.date(), inputV.time(), inputV.stepUnits(), inputV.startStep(), inputV.endStep(), requested.bitsPerValue(), valuesV);

    return new Wind(u,v);

//		throw UserError("SpectralToSpectralTransformer::transformVector vector treatment applied just for vort,div ");

//	return 0;
}

inline double dd(double pm, double pn) {
    return -sqrt((pn*pn - pm*pm) / (4.*pn*pn - 1)) / pn;
}

inline double ss(double pm, double pn) {
    return -pm / (pn*(pn + 1));
}

void SpectralToSpectralTransformer::vorticityDivergenceToUV(const vector<comp>& vor, const vector<comp>& div, int toup, vector<comp>& uu, vector<comp>& vv) const {
    /*
         Calculate spherical harmonic coefficients of U*cos(lat) and
         V*cos(lat) at triangular truncation toup, given the coefficients
         of divergence and vorticity at truncation toup-1.
    */
    long  outSize    = ((toup + 1) * (toup + 2)) / 2;

    uu.clear();
    uu.reserve(outSize);
    vv.clear();
    vv.reserve(outSize);

    if(DEBUG) {
        cout << "********************************************" << endl;
        cout << "******** Vor,Div to U,V  Conversion ********" << endl;
        cout << "********************************************" << endl;
        cout << "SpectralToSpectralTransformer::vorticityDivergenceToUV out Size: " << outSize << endl;
    }

    comp       zi (0.0,1.0);
    double rEarth = 6.371e6;
    long        k = 0;
    long      imn = 0;

    /* Handle coefficients for m < toup; n = m */
    for ( int j = 0 ; j < toup ;  j++ ) {
        double zm = j ;
        double zn = zm;
        if (j) {
            uu.push_back( (-dd(zm,zn+1.)*vor[imn+1] + zi*ss(zm,zn)*div[imn]) * rEarth );
            vv.push_back( ( dd(zm,zn+1.)*div[imn+1] + zi*ss(zm,zn)*vor[imn]) * rEarth );

        } else {
            uu.push_back( (-dd(zm,zn+1) * vor[imn+1]) * rEarth ) ;
            vv.push_back( ( dd(zm,zn+1) * div[imn+1]) * rEarth );
        }
        ++imn;
        ++k;
        int  jmp = j + 1;

        /* When n < toup - 1 */
        if (jmp < toup - 1 ) {
            for ( int i = jmp ; i < toup - 1 ;  i++ ) {
                zn = i;
                uu.push_back( ( dd(zm,zn)*vor[imn-1] - dd(zm,zn+1)*vor[imn+1] + zi*ss(zm,zn)*div[imn]) * rEarth );
                vv.push_back( (-dd(zm,zn)*div[imn-1] + dd(zm,zn+1)*div[imn+1] + zi*ss(zm,zn)*vor[imn]) * rEarth );
                ++k;
                ++imn;
            }
            /* When n == toup - 1 */
            zn = toup - 1;
            uu.push_back( ( dd(zm,zn)*vor[imn-1] + zi*ss(zm,zn)*div[imn]) * rEarth );
            vv.push_back( (-dd(zm,zn)*div[imn-1] + zi*ss(zm,zn)*vor[imn]) * rEarth );
            ++k;
            ++imn;
        }
        /* When n == toup */
        zn = toup;
        uu.push_back( ( dd(zm,zn)*vor[imn-1]) * rEarth );
        vv.push_back( ( -dd(zm,zn)*div[imn-1]) * rEarth );
        ++k;
        /* When n == toup + 1 */
        /* IMN  = IMN + 1 + KTIN-ITOUT */
        /* KTIN-ITOUT = -1 */
        imn = imn;
    }
    ASSERT(uu.size() && vv.size());

    /* Handle coefficients for m = toup */
    /* When n == toup */
    uu.push_back(0);
    vv.push_back(0);
    uu.push_back(0);
    vv.push_back(0);
    /*
    	cout.precision(15);
    	for ( int j = 0; j < outSize ;  j++ ){
    		cout << "vorticityDivergenceToUV uu " << j << "   "  << uu[j] << endl;
    		cout << "vorticityDivergenceToUV vv " << j << "   "  << vv[j] << endl;
    	}
    o*/
}
