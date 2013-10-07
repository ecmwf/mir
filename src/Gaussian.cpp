/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Gaussian.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Point_H
#include "Point.h"
#endif

#ifndef FieldPoint_H
#include "FieldPoint.h"
#endif

#ifndef Tokenizer_H
#include "Tokenizer.h"
#endif

#include "DefinitionsCache.h"

#include <sstream>
#include <fstream>

Gaussian::Gaussian(int n)
		 : Grid(), gaussianNumber_(n)
{
    latitudes_.resize(n*2);
	gaussianLatitudes(latitudes_);
    
    // where area is not specified, we set it to the extremes of the grid
    // so that calculatedNumberOfPoints works.
    area_ = setGlobalArea();
}

Gaussian:: Gaussian(double north, double west, double south, double east, int n)
		  :  Grid(north,west,south,east), gaussianNumber_(n)
{
    latitudes_.resize(n*2);
	gaussianLatitudes(latitudes_);
}

Gaussian::Gaussian(const Area& other, int n)
	     :  Grid(other), gaussianNumber_ (n)
{
    latitudes_.resize(n*2);
	gaussianLatitudes(latitudes_);
}

Gaussian:: ~Gaussian() 
{ 
}

 
int Gaussian::gaussianNumber() const 
{ 
	return gaussianNumber_; 
}

void Gaussian::northHemisphereLatitudes(vector<double>& lats) const
{
	int i = 0;
	while(latitudes_[i] > 0 || iszero(latitudes_[i])) {
		lats.push_back(latitudes_[i++]);
	}
}

void Gaussian::latitudes(vector<double>& lats) const
{ 
	lats.clear();
	int northLat = northIndex(north());
	int nsNumber = northSouthNumberOfPoints();
	ASSERT((nsNumber + northLat) <= gaussianNumber_ * 2);

	if(DEBUG){
		cout << "Gaussian::latitudes north Index: " << northLat << endl;
		cout << "Gaussian::latitudes number of lats: " << nsNumber << endl;
	}

	lats.reserve(nsNumber);

    for ( int i = northLat ; i < nsNumber + northLat ; i++ ) {
		lats.push_back(latitudes_[i]);
	}
}

string Gaussian::coeffInfo() const
{
	stringstream s;
	s <<"N" << gaussianNumber_;

	return s.str();
}

int Gaussian::matchGaussian() const
{
	return gaussianNumber_;
}

int Gaussian::truncate(int truncationIn) const
{
	const string resol = getShareDir() + "/definitions/resolutions";
    ref_counted_ptr< const vector<string> > resolSpec = DefinitionsCache::get(resol);

    vector<string>::const_iterator it = resolSpec->begin();
	int truncationOut, gaussian;
	double llmin, llmax;

    while (it != resolSpec->end())
    {
        istringstream in(*it);
	    in >> truncationOut >> llmin >> llmax >> gaussian;
		if(gaussian == gaussianNumber_){
			if(truncationOut > truncationIn)
				truncationOut = truncationIn;
			return truncationOut;
		}
        it++;
	}

	return truncationIn;
}

size_t Gaussian::loadGridSpec(vector<long>& rgSpec) const
{
	Tokenizer tokens("  ");
	
	stringstream s;
	s << getShareDir() << "/definitions/reduced_gaussian/rgauss_" << gaussianNumber_;

    string fileName = s.str();
    ref_counted_ptr< const vector<string> > gridSpec = DefinitionsCache::get(fileName);

    vector<string>::const_iterator it = gridSpec->begin();

	int nlat = gaussianNumber_ * 2;
	if((int)rgSpec.size() < nlat)
		rgSpec.resize(nlat);
	int i = 0;

    while (it != gridSpec->end() )
    {
		vector<string> v;
		tokens(*it, v);
		int vsize = v.size();

		for ( int j = 0 ; j < vsize ; j++ ) {
			rgSpec[i] = atol(v[j].c_str());
			rgSpec[nlat-1-i] = rgSpec[i];
			++i;
		}

        it++;
	}
	return nlat;
}



Area Gaussian::fitToGlobalArea(double west) const
{
	return setGlobalArea(west);
}

void Gaussian::setGlobalNorthSouth(double& north, double& south) const
{
	north = latitudes_[0]; 
	south = latitudes_[gaussianNumber_*2-1];
}

Area Gaussian::setGlobalArea(double west) const
{
	if(same(west,-180.0))
		return Area(latitudes_[0], -180.0, latitudes_[gaussianNumber_*2-1],(180.0 - 90./gaussianNumber_));

	return Area(latitudes_[0], 0, latitudes_[gaussianNumber_*2-1],(4*gaussianNumber_-1) * (90./gaussianNumber_));

}

Area Gaussian::setGlobalArea() const
{
	return Area(latitudes_[0], 0, latitudes_[gaussianNumber_*2-1],(4*gaussianNumber_-1) * (90./gaussianNumber_));
}

bool Gaussian::isGlobalNorthSouth() const
{
// ssp
    return (fabs(south() - latitudes_[gaussianNumber_*2-1]))<= AREA_FACTOR && fabs( north() - latitudes_[0]) <= AREA_FACTOR;
}

bool Gaussian::isGlobalWestEast() const
{
/// AREA_FACTOR is added because grib has precision for 3 dec places.
/// For instance east for N640 is 359.8593750 intstead of 359.859 
	double res = east() - west() + 90.0 / gaussianNumber_ + AREA_FACTOR;
	return res > EQUATOR || same(res,EQUATOR);
}

void Gaussian::adjustAreaWestEastMars( double& west, double& east, double increment) const
{
	Grid::adjustAreaWestEastMars(west, east, increment);

//  Try to catch special case: global west-east with wrap-around.
	if (east - west > 360.0)
		east = west + (4. * gaussianNumber_ - 1.) * increment;
}

Area Gaussian::adjustArea(const Area& area,bool globalWE, bool globalNS) const
{
	double west = area.west();
	double east = area.east();

	if(DEBUG){
		cout << "Gaussian::adjustArea west: " << west << " east " << east << endl;
		cout << "Gaussian::adjustArea is global west-east: " << globalWE << " is global north-south " << globalNS << endl;
	}

	if(globalWE)
		setGlobalWestEast(west,east,90.0/gaussianNumber_);
	else
		adjustAreaWestEast(west,east,90.0/gaussianNumber_);

    double north = 0, south = 0;

    if(globalNS){
        setGlobalNorthSouth(north,south);
	}
	else{
		north = latitudes_[northIndex(area.north())];
		south = latitudes_[southIndex(area.south())];
	}

	return Area(north,west,south,east);
}

int Gaussian::westEastNumberOfPoints() const
{
	if(isGlobalWestEast())
		return 4 * gaussianNumber_;

    // NB do we need different logic for EMOS_SIMULATION? 
    return (int)((east() - west() + 90.0/gaussianNumber_) / (90.0/gaussianNumber_) + ROUNDING_FACTOR);
}

int Gaussian::northSouthNumberOfPoints() const
{
	if(isGlobalNorthSouth())
    {
		return gaussianNumber_ * 2;
    }
// ssp  
	return southIndex(south()) - northIndex(north()) + 1;
}

int Gaussian::northIndex(double north) const
{
#if ECREGRID_EMOS_SIMULATION
        return northIndexMars(north);
#else

    // Round north boundary to nearest Latitude southwards
    for ( int i = 0 ; i < gaussianNumber_ * 2 ; i++ ) {
         // unit test added AREA_FACTOR because grib has limited precisison
		double res = north + AREA_FACTOR;
		if(res > latitudes_[i] || same(res,latitudes_[i]))
        {
			return i;
		}
	}
	throw WrongValue("Gaussian::northIndex - North is not within Area",north);
#endif // ECREGRID_EMOS_SIMULATION
}

int Gaussian::southIndex(double south) const
{
#if ECREGRID_EMOS_SIMULATION
        return southIndexMars(south);
#else
    // Round south boundary to nearest FieldPoint northwards
	int i;
    for ( i = 0 ; i < gaussianNumber_ * 2 ; i++ ) {
		if(same(south,latitudes_[i]))
			return i ;
		else if(south > latitudes_[i])
			return i - 1;
	}
	throw WrongValue("Gaussian::southIndex - South is not within Area",south);
#endif // ECREGRID_EMOS_SIMULATION
}

int Gaussian::northIndexMars(double north) const
{
    // Round north boundary to nearest Latitude northwards
    for ( int i = 0 ; i < gaussianNumber_ * 2 ; i++ ) {
        if (same(north, latitudes_[i]))
        {
            return i;
        }
        else if(north > latitudes_[i] ) {            
            return i > 0 ? i - 1 : 0;
		}
	}

	throw WrongValue("Gaussian::northIndexMars - North is not within Area",north);
}

int Gaussian::southIndexMars(double south) const
{
    // Round south boundary to nearest FieldPoint southwards
    int last_latitude = 0; 
    for (int i = 0 ; i < gaussianNumber_ * 2 ; i++ ) {
		if(south > latitudes_[i] || same(south,latitudes_[i]))
        {
			return i;
        }
        last_latitude=i;
	}
	
    // if we are here, then we were not able to find a south index within the 
    // south area bound. return the most southern value we can

    return last_latitude;
}


void Gaussian::gaussianLatitudesFirstGuess(vector<double>& vals) const
{
// Computes initial approximations for Gaussian latitudes
// return zeros of the bessel function
	unsigned long i = 0;
	double gvals[] =  {     2.4048255577E0,   5.5200781103E0,
		8.6537279129E0,   11.7915344391E0,  14.9309177086E0,
		18.0710639679E0,  21.2116366299E0,  24.3524715308E0,
		27.4934791320E0,  30.6346064684E0,  33.7758202136E0,
		36.9170983537E0,  40.0584257646E0,  43.1997917132E0,
		46.3411883717E0,  49.4826098974E0,  52.6240518411E0,
		55.7655107550E0,  58.9069839261E0,  62.0484691902E0,
		65.1899648002E0,  68.3314693299E0,  71.4729816036E0,
		74.6145006437E0,  77.7560256304E0,  80.8975558711E0,
		84.0390907769E0,  87.1806298436E0,  90.3221726372E0,
		93.4637187819E0,  96.6052679510E0,  99.7468198587E0,
		102.8883742542E0, 106.0299309165E0, 109.1714896498E0,
		112.3130502805E0, 115.4546126537E0, 118.5961766309E0,
		121.7377420880E0, 124.8793089132E0, 128.0208770059E0,
		131.1624462752E0, 134.3040166383E0, 137.4455880203E0,
		140.5871603528E0, 143.7287335737E0, 146.8703076258E0,
		150.0118824570E0, 153.1534580192E0, 156.2950342685E0, };

		for( i = 0; i < (unsigned long)gaussianNumber_; i++)
		{
			if(i < NUMBER(gvals))
				vals[i] = gvals[i];
			else
				vals[i] = vals[i-1] + M_PI;
		}
}

void Gaussian::weightsY(vector<double>& weights) const
{
// Compute weights for Gaussian integration
    const int MAXITER = 10;
	long jlat, iter, legi;
	double rad2deg, convval;
	double conv;

	double precision = 1.0E-14;
//	double precision = 1.0E-15;
	long  nlat = gaussianNumber_ * 2;

    // The below is an educated guess when converting from double* to vector
    // (previously it was assumed they were allocated);
    weights.resize(nlat);

	rad2deg = 180.0/M_PI;

	convval   = (1.0 - ((2.0 / M_PI)*(2.0 / M_PI)) * 0.25);

//	gaussianLatitudesFirstGuess(weights); 

// Loop over the desired roots
	for (jlat = 0; jlat < gaussianNumber_; jlat++)
	{

		//   First approximation for root
		double root = cos(M_PI*((jlat+1) - 0.25) / (nlat + 0.5));
		double root1 = 0.;
		//Starting with the above approximation to the Ith root, we enter the main loop of refinement by Newton's method.

		iter = 0;
		conv = 1.;
		double deriv = 0.;

		while(fabs(root - root1) > precision )
		{
			double mem1 = 1.0;
			double mem2 = 0.;

			// Loop up the recurrence relation to get the Legendre polynomials evaluated
			for(legi = 0; legi < nlat; legi++)
			{
				double mem3 = mem2;
				mem2 = mem1;
				mem1 = ( (2.0 * (legi+1) - 1.0) * root * mem2 - (legi * mem3)) / ((double)(legi+1.));
			}
			// mem1 is now the desired Legendre polynomial. We next compute deriv, its derivative, by a standard relation involving also mem2, the polynomial of one lower order.
			deriv = nlat * (root * mem1 - mem2) / (root * root - 1.);
			root1 = root;
			root  = root1 - mem1 / deriv ;

			//  Routine fails if no convergence after JPMAXITER iterations.
			ASSERT( iter++ < MAXITER );
		}

		//   Set North and South values using symmetry.
		// gaussian lats
		//lats[i] = root;
		//lats[nlat-1-jlat] = root;

		weights[jlat] = 2./((1. - root*root) * deriv*deriv); 
		weights[nlat-1-jlat] = weights[jlat];

	}

}

/*
void Gaussian::weightsY(double* weights) const
{
// Compute weights for Gaussian integration
    const int MAXITER = 10;
	long jlat, iter, legi;
	double rad2deg, convval, root, legfonc = 0;
	double mem1, mem2, conv;

	double precision = 1.0E-14;
//	double precision = 1.0E-15;
	long  nlat = gaussianNumber_ * 2;

	rad2deg = 180.0/M_PI;

	convval   = (1.0 - ((2.0 / M_PI)*(2.0 / M_PI)) * 0.25);

	gaussianLatitudesFirstGuess(weights); 

	for (jlat = 0; jlat < gaussianNumber_; jlat++)
	{

		//   First approximation for root

		root = cos(weights[jlat] / sqrt( ((((double)nlat)+0.5)*(((double)nlat)+0.5)) + convval) );

		//   Perform loop of Newton iterations

		iter = 0;
		conv = 1;
		double deriv = 0;

		while(fabs(conv) >= precision )
//		while(fabs(conv) <= precision )
		{
			mem2 = 1.0;
			mem1 = root;

			//  Compute Legendre polynomial

			for(legi = 1; legi < nlat; legi++)
			{
				legfonc = ( (2.0 * (legi+1) - 1.0) * root * mem1 - legi * mem2) / ((double)(legi+1));
				mem2 = mem1;
				mem1 = legfonc;  
			}

			//  Perform Newton iteration


            deriv = ((((double)nlat) * (mem2 - root * legfonc) ) / (1.0 - (root *root)));
			conv = legfonc / deriv; 
			root -= conv;

			//  Routine fails if no convergence after JPMAXITER iterations.

			ASSERT( iter++ < MAXITER );
		}
		//   Set North and South values using symmetry.
		weights[jlat] = (2.*((double)nlat)+1.)/(deriv*deriv); 
		weights[nlat-1-jlat] = weights[jlat];

	}

	if( nlat != (gaussianNumber_*2) ) {
	//	lats[gaussianNumber_ + 1] = 0.0;
		double temp = 2 / (gaussianNumber_*gaussianNumber_);
		for( int i = 1; i < gaussianNumber_; i += 2){
			int j = i + 1;
			temp = (temp * j * j) / (i * i); 
		}
		weights[gaussianNumber_ + 1] = temp;
	}
}
*/

void Gaussian::gaussianLatitudes(vector<double>& lats) const
{

    const int MAXITER = 10;
    long jlat, iter, legi;
    double rad2deg, convval, root, legfonc = 0.;
    double mem1, mem2, conv;

    double precision = 1.0E-14;
//  double precision = 1.0E-15;
    long  nlat = gaussianNumber_ * 2;

    rad2deg = 180.0/M_PI;

    convval   = (1.0 - ((2.0 / M_PI)*(2.0 / M_PI)) * 0.25);

    gaussianLatitudesFirstGuess(lats); 

    for (jlat = 0; jlat < gaussianNumber_; jlat++)
    {
		//   First approximation for root
		root = cos(lats[jlat] / sqrt( ((((double)nlat)+0.5)*(((double)nlat)+0.5)) + convval) );

		//   Perform loop of Newton iterations
		iter = 0;
		conv = 1.;

//		while(fabs(conv) > precision || same(fabs(conv),precision) )
		while(fabs(conv) > precision )
		{
			mem2 = 1.0;
			mem1 = root;

			//  Compute Legendre polynomial
			for(legi = 0; legi < nlat; legi++)
			{
				legfonc = ( (2.0 * (legi+1.) - 1.0) * root * mem1 - legi * mem2) / ((double)(legi+1.));
				mem2 = mem1;
				mem1 = legfonc;
			}

			//  Perform Newton iteration
			conv = legfonc / ((((double)nlat) * (mem2 - root * legfonc) ) / (1.0 - (root *root)));
			root -= conv;

			//  Routine fails if no convergence after JPMAXITER iterations.
			ASSERT( iter++ < MAXITER );
		}

		//   Set North and South values using symmetry.

		lats[jlat] = asin(root) * rad2deg;

		lats[nlat-1-jlat] = -lats[jlat];
	}

	if( nlat != (gaussianNumber_*2) )
		lats[gaussianNumber_ + 1] = 0.0;
}   


void Gaussian::dump() const
{
	double lat = south();
	double lon = west();
	
	cout << "     ";
	while(lon < east())
	{
		cout << std::setw(5) << lon << " ";
		lon = east();
	}
	cout << endl;
	
	lon = west();
	
	cout << std::setw(4) << lat;
	
	while(lat <= north())
	{
//		cout << " " << std::setw(5) << data_[i++];
		lon += east();
		if(lon >= east())
		{
			cout << endl;
			lat = north();
			if(lat <= north())
				cout << std::setw(4) << lat;
			lon = west();
		}
	}
	
}
void Gaussian::printLatitudes() const
{
		cout << " Lat: -----------------------------" << endl;
	cout.precision(16);
	for( int i = 0; i < gaussianNumber_*2 ; i++) {
		cout << " Lat: " << latitudes_[i] << endl;
	}
		cout << " Lat: -----------------------------" << endl;

}

void Gaussian::print(ostream& out) const
{
	out << "Gaussian:" ; Grid::print(out); 
	out << ", Gaussian number=[" << gaussianNumber_ << "]";
//	printLatitudes();

}
