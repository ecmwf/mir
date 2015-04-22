/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "HandleRequest.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Tokenizer_H
#include <eckit/parser/Tokenizer.h>
#endif

#ifndef GribApiInput_H
#include "GribApiInput.h"
#endif

#ifndef AsciiInput_H
#include "AsciiInput.h"
#endif

#ifndef GribApiOutput_H
#include "GribApiOutput.h"
#endif

#ifndef FieldDescriptionGribApi_H
#include "FieldDescriptionGribApi.h"
#endif

#ifndef Factory_H
#include "Factory.h"
#endif

#ifndef HAVE_GETOPT_H
#include <unistd.h>
#else
#include "getopt.h"
#endif

#include "grib_api.h"
#include "ecregrid_api.h"

#include <string>
#include <sstream>
#include <algorithm>

// if we don't have long options then
// we must use only short options
#ifndef HAVE_GETOPT_H
#define KEEP_ALL_SHORT_OPTS
#endif

// to force all short options to be turned on, even if long options are
// also available, then uncomment the following line
#define KEEP_ALL_SHORT_OPTS
//
//
// for clarity, introduce a new define to show whether we have long options
// available
#ifdef HAVE_GETOPT_H
#define HAVE_LONG_OPTS
#endif

/* define various command-line arguments  */
#ifdef KEEP_ALL_SHORT_OPTS
/* If we want all parameters to have short options */
/* then we define them here */
/* parameters here */
#define ARG_POLE_METHOD              'u'
#define ARG_BITMAP_FILE              'b'
#define ARG_FRAME_LINES              'f'
#define ARG_LIST_OF_POINTS           'w'
#define ARG_LSM_SOURCE               'v'
#define ARG_NUMBER_OF_NEAREST_POINTS 'k'
#define ARG_AUTO_RESOLUTION          'z'
#define ARG_GRIB_ACCURACY            'q'
#define ARG_GRIB_EDITION             'e'
#define ARG_SCANNING_MODE            's'
#define ARG_DERIVED_PARAMETER_METHOD 'j'
#define ARG_DISABLE_VECTORS          'd'

#else
/* to remove the short options for these values */
/* these must be given a char value < 97 ('a') */
/* which makes the code function but using characters */
/* that a user cannot type in */
#define ARG_POLE_METHOD              (char)32
#define ARG_BITMAP_FILE              (char)33
#define ARG_FRAME_LINES              (char)34
#define ARG_LIST_OF_POINTS           (char)35
#define ARG_LSM_SOURCE               (char)36
#define ARG_NUMBER_OF_NEAREST_POINTS (char)37
#define ARG_AUTO_RESOLUTION          (char)38
#define ARG_GRIB_ACCURACY            (char)39
#define ARG_GRIB_EDITION             (char)40
#define ARG_SCANNING_MODE            (char)41
#define ARG_DERIVED_PARAMETER_METHOD (char)42
#define ARG_DISABLE_VECTORS          (char)43

#endif // #ifdef KEEP_ALL_SHORT_OPTS

#define THROW_IF_NOT_OF_LENGTH(vec, len, msg) { if (vec.size() != len) { throw BadParameter(msg);} }
#define THROW_IF_NOT_INT(x) { stringstream ss(x), oss; int d; ss >> d; oss << d; if (!ss || oss.str() != x) { throw BadParameter(x);} }

HandleRequest::HandleRequest():
    output_(new FieldDescription),
	inFile_("x"),
    outFile_("y"),
    outFileType_("grib")
{
}

HandleRequest::~HandleRequest()
{
}

/*! \defgroup tool ecRegrid command line tool

\section intro Overview

ecRegrid offers simple for usage ecregrid UNIX command line tool which has plenty of options. It performs tranformation between spectral or grid fields from spectral or grid on input.
\note Input must be grib messages

\note Output could be GRIB messages or arrays of numbers in binary or ascii format.

USAGE:
./ecregrid -i input_file [optional input file description] -o output_file [output field description]

Full list of possible options could be obtained typing just ecregrid in command line or ecregrid -h

\example ecregrid -i in.grib -o out.grib -l 0.5/0.5 -a 80.0/-40.0/20.0/60.0

*/
void HandleRequest::usage() const
{

// Check whether Long options are available and if not, then restrict usage to
// short options only
//
#define SHORT_OPT(s) ( isalpha(s) ? cout << "\t-" << s << " ": cout << "\t")
#ifdef HAVE_LONG_OPTS
#define LONG_OPT(s)  ( cout << "--" << s << " " )
#else
#define LONG_OPT(s) 0
#endif

    cout << endl   << "DESCRIPTION:" << endl
	     << "\t"   <<     "Transforms spectral, regular and reduced lat/lon and Gaussian grids." << endl
         << "\t"   <<     "Input grid description is read from the GRIB header. Handles multiple GRIB messages." << endl
	     << endl   << "USAGE:" << endl
	     << "\t"   <<     "ecregrid -i input_file -o output_file [OPTIONS]" << endl
         << endl   << "OPTIONS:" << endl;

    SHORT_OPT('x'); LONG_OPT("format"); cout << "name" << endl;
    cout << "\t\t" <<         "Output file format: GRIB (default), binary or ASCII" << endl;

    SHORT_OPT('a'); LONG_OPT("area"); cout << "north/west/south/east" << endl;
	cout << "\t\t" <<         "Grid area" << endl;

    SHORT_OPT('t'); LONG_OPT("truncation"); cout << "T" << endl;
	cout << "\t\t" <<         "Spectral field truncation" << endl;

    SHORT_OPT('l'); LONG_OPT("grid-increments"); cout << "we/ns" << endl;
	cout << "\t\t" <<         "Regular lat/lon grid west-east, north-south increments respectively" << endl;

    SHORT_OPT('n'); LONG_OPT("number-of-grid-points"); cout << "wePoints/nsPoints" << endl;
	cout << "\t\t" <<         "Number of points in regular lat/lon grid for west-east, north-south respectively" << endl;

    SHORT_OPT('r'); LONG_OPT("location-south-pole"); cout << "lat/lon" << endl;
	cout << "\t\t" <<         "Latitude and longitude of the south pole of rotation" << endl;

    SHORT_OPT('g'); LONG_OPT("gaussian-number"); cout << "N" << endl;
	cout << "\t\t" <<         "Lines of latitude between equator and pole for Gaussian grid definition" << endl;

    SHORT_OPT('y'); LONG_OPT("grid-type"); cout << "type" << endl;
	cout << "\t\t" <<         "Grid types:" << endl
	     << "\t\t\t" <<         "regular         - Regular grid (default)" << endl
	     << "\t\t\t" <<         "reduced         - Reduced grid" << endl
	     << "\t\t\t" <<         "cellcentred     - Regular lat/lon cell-centred " << endl;


    SHORT_OPT(ARG_POLE_METHOD); LONG_OPT("pole-method"); cout << "method" << endl;
	cout << "\t\t" <<         "Method for extrapolation on poles:" << endl
	     << "\t\t\t" <<          "average        - From last latitude" << endl
	     << "\t\t\t" <<          "linear" << endl;

    SHORT_OPT(ARG_BITMAP_FILE); LONG_OPT("bitmap-file"); cout << "path/to/file" << endl;
	cout << "\t\t" <<         "Bitmap specification file name" << endl;

    SHORT_OPT(ARG_FRAME_LINES); LONG_OPT("frame-lines"); cout << "N" << endl;
	cout << "\t\t" <<         "Number of lines for frames from rectangular (regular) grids" << endl;

    SHORT_OPT(ARG_LIST_OF_POINTS); LONG_OPT("list-of-points-file"); cout << "/path/to/file" << endl;
	cout << "\t\t" <<         "File name of list of points (lat/lon) specification" << endl;

    SHORT_OPT(ARG_LSM_SOURCE); LONG_OPT("lsm-source"); cout << "source" << endl;
    cout << "\t\t"   <<         "Land-Sea Mask source:" << endl
	     << "\t\t\t" <<         "predefined      - From operational model (default)" << endl
	     << "\t\t\t" <<         "10min           - 10 min USNAVY" << endl
	     << "\t\t\t" <<         "gtopo" << endl
	     << "\t\t\t" <<         "off             - Disable lsm processing " << endl;

    SHORT_OPT('m'); LONG_OPT("interpolation-method"); cout << "method" << endl;
	cout << "\t\t"   <<         "Interpolation methods:" << endl
	     << "\t\t\t" <<         "bilinear         - Bilinear (default)," << endl
	   //  << "\t\t\t" <<         "doublelinear   - Double Linear," << endl
	     << "\t\t\t" <<         "nearestneighbour - Nearest Neighbour," << endl
	     << "\t\t\t" <<         "cubic            - Cubic with 12 points (default for interpolation to rotated grids), " << endl
	     << "\t\t\t" <<         "linear           - Linear" << endl
	     << "\t\t\t" <<         "linearfit        - Linear fit along the two lines of latitudes then linear fit along the line of meridian" << endl
	     << "\t\t\t" <<         "average          - Average of nearest points" << endl
         << "\t\t\t" <<         "averageweighted  - Area Weighted Average" << endl;

    SHORT_OPT(ARG_NUMBER_OF_NEAREST_POINTS); LONG_OPT("number-of-nearest-points"); cout << "N" << endl;
    cout << "\t\t"   <<         "Number of nearest points for Average or Nearest Neighbour  - 4 (default), 16, 36, 64" << endl;

    SHORT_OPT(ARG_AUTO_RESOLUTION); LONG_OPT("auto-resolution"); cout << "on/off" << endl;
    cout << "\t\t"   <<         "Match corresponding truncation with grid resolution: on or off (default)" << endl;

    SHORT_OPT('c'); LONG_OPT("coefficient-method"); cout << "method" << endl;
    cout << "\t\t"   <<         "Preferred handling of Legendre polynomials:" << endl
         << "\t\t\t" <<             "on-fly      - Recalculate each time (default)" << endl
         << "\t\t\t" <<             "fileio      - Read pre-calculated values from named file" << endl
         << "\t\t\t" <<             "shared      - Shared memory" << endl
         << "\t\t\t" <<             "mapped      - From memory map" << endl;


    SHORT_OPT(ARG_DISABLE_VECTORS); LONG_OPT("disable-vectors"); cout << endl;
    cout << "\t\t"   <<         "Disable vorticity/divergence to u/v conversion" << endl;

    SHORT_OPT(ARG_GRIB_ACCURACY); LONG_OPT("grib-accuracy"); cout << "bits" <<  endl;
    cout << "\t\t"   <<         "Bits per value in the output GRIB message" << endl;

    SHORT_OPT(ARG_SCANNING_MODE); LONG_OPT("scanning-mode"); cout << "mode" << endl;
    cout << "\t\t"   <<         "1 - north-south, east-west /  i scans positively, j scans negatively (default)" << endl
         << "\t\t"   <<         "2 - south-north, east-west /  i scans positively, j scans positively" << endl
         << "\t\t"   <<         "3 - north-south, west-east /  i scans negatively, j scans negatively " << endl
         << "\t\t"   <<         "4 - south-north, west-east /  i scans negatively, j scans positively" << endl ;

    SHORT_OPT('p'); LONG_OPT("projection"); cout << "name" << endl;
    cout << "\t\t"   <<         "Projection type:" << endl
         <<	"\t\t\t" <<             "polar_stereographic" << endl;
     //    << "\t\t\t" <<             "lambert_conformal" << endl
     //    << "\t\t\t" <<             "lambert_azimuthal_equal_area" << endl;

    SHORT_OPT(ARG_GRIB_EDITION); LONG_OPT("grib-edition"); cout << "N" << endl;
	cout << "\t\t"   <<         "GRIB edition" << endl;

    SHORT_OPT(ARG_DERIVED_PARAMETER_METHOD); LONG_OPT("derived-parameter-method"); cout << "method" << endl;
    cout << "\t\t"   <<         "Standard Deviation and derived parameters:" << endl
         <<	"\t\t\t" <<             "stddev              - Standard Deviation" << endl
         << "\t\t\t" <<             "anisotropy          - Anisotropy" << endl
         << "\t\t\t" <<             "orientation         - Orientation" << endl
         << "\t\t\t" <<             "slope               - Slope" << endl
         << "\t\t\t" <<             "meridionalderivativ - Meridional Derivatives" << endl
         << "\t\t\t" <<             "zonalderivativ      - Zonal Derivatives" << endl;

    // added version info
    cout << endl;
    showVersionInfo();
}

void HandleRequest::showVersionInfo() const
{
    cout << "  - ecRegrid library version " << ECREGRID_MAJOR_VERSION << "." << ECREGRID_MINOR_VERSION << "." << ECREGRID_REVISION_VERSION << endl;
	long grib_api_version = grib_get_api_version();
	cout << "  - Using grib_api version " << grib_api_version  / 10000 << "." << (grib_api_version / 100) % 100 << "." << grib_api_version % 100 <<  endl;
}

int HandleRequest::request(int argc, char* argv[])
{

#ifdef HAVE_LONG_OPTS

    static struct option long_options[] =
     {
       /* We set up long names for options with a short name.
          NB Where the long name is given, getopt_long will return
             the short name for use in our switch statement */
       {"format",                            required_argument,     0, 'x'},
       {"area",                              required_argument,     0, 'a'},
       {"truncation",                        required_argument,     0, 't'},
       {"grid-increments",                   required_argument,     0, 'l'},
       {"number-of-grid-points",             required_argument,     0, 'n'},
       {"location-south-pole",               required_argument,     0, 'r'},
       {"gaussian-number",                   required_argument,     0, 'g'},
       {"grid-type",                         required_argument,     0, 'y'},
       {"interpolation-method",              required_argument,     0, 'm'},
       {"coefficient-method",                required_argument,     0, 'c'},
       {"projection",                        required_argument,     0, 'p'},
       {"input",                             required_argument,     0, 'i'},
       {"output",                            required_argument,     0, 'o'},
       {"help",                              no_argument,           0, 'h'},
       /* the following have no short options. We use an integer earlier than
        * the first ascii letter 'a' so we can recognise the option in a
        * switch statement, but disallow user to specify option via the
        * command- line*/
       {"pole-method",                       required_argument,     0, ARG_POLE_METHOD},
       {"bitmap-file",                       required_argument,     0, ARG_BITMAP_FILE},
       {"frame-lines",                       required_argument,     0, ARG_FRAME_LINES},
       {"list-of-points-file",               required_argument,     0, ARG_LIST_OF_POINTS},
       {"lsm-source",                        required_argument,     0, ARG_LSM_SOURCE},
       {"number-of-nearest-points",          required_argument,     0, ARG_NUMBER_OF_NEAREST_POINTS},
       {"auto-resolution",                   required_argument,     0, ARG_AUTO_RESOLUTION},
       {"grib-accuracy",                     required_argument,     0, ARG_GRIB_ACCURACY},
       {"grib-edition",                      required_argument,     0, ARG_GRIB_EDITION},
       {"scanning-mode",                     required_argument,     0, ARG_SCANNING_MODE},
       {"derived-parameter-method",          required_argument,     0, ARG_DERIVED_PARAMETER_METHOD},
       {"disable-vectors",                   no_argument,           0, ARG_DISABLE_VECTORS},
       {0, 0, 0, 0}
     };

#endif // HAVE_LONG_OPTS

    eckit::Tokenizer tokens("/");

	if( argc < 2 ) {
		usage();
		return 1;
	}

    showVersionInfo();

	int c;

	opterr=0;

	vector<string> area;
	vector<string> southPole;
	vector<string> increments;
	vector<string> points;
	int number, bits, edition;
	long wePoints, nsPoints;
	double north, west, south, east;
	double ns, we, lat, lon;
    int option_index;
    std::string opt;

#ifndef KEEP_ALL_SHORT_OPTS
	static char optstring[] = "hi:o:t:r:a:l:g:m:x:y:c:p:n:";
#else
	static char optstring[] = "hi:o:t:r:a:l:g:m:x:y:c:p:n:u:b:f:w:v:k:z:q:e:s:j:d";
#endif

#ifdef HAVE_LONG_OPTS
	while ((c=getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
#else
	while ((c=getopt(argc, argv, optstring)) != -1)
#endif
	{
		switch(c)
		{
		 case 'h':
			usage();
			return 1;
			break;
		 case 'i':
			inFile_ = optarg;
			cout << "<<< INPUT  <<< File Name: " << optarg << endl;
			break;
		 case 'o':
			outFile_ = optarg;
			cout << ">>> OUTPUT >>> File Name: " << optarg << endl;
			break;
		 case 'x':
			cout << "  - Type of Output File: ";
			outFileType_ = optarg;
			cout <<  optarg << endl;
			break;
         case ARG_LIST_OF_POINTS:
			cout << "  - List of Points File: ";
			output_->listOfPointsFile(optarg);
			outFileType_ = "ascii";
			cout << optarg << endl;
			break;
         case 't':
			cout << "  - Spectral field Truncation: ";
            THROW_IF_NOT_INT(optarg);
			number = atoi(optarg);
			output_->truncation(number);
			cout << number << endl;
			break;
         case 'm':
			output_->interpolationMethod(optarg);
			cout << "  - Interpolation method: " << optarg << endl;
			break;
         case ARG_DERIVED_PARAMETER_METHOD:
			output_->grid2gridTransformationType(optarg);
			cout << "  - Grid to Grid as: " << optarg << endl;
			break;
         case 'e':
            THROW_IF_NOT_INT(optarg);
			edition = atoi(optarg);
			output_->editionNumber(edition);
			cout << "  - Edition Number: " << edition << endl;
                        break;
         case ARG_GRIB_ACCURACY:
            THROW_IF_NOT_INT(optarg);
			bits = atoi(optarg);
			output_->bitsPerValue(bits);
			cout << "  - Bits Per Value: " << bits << endl;
			break;
         case 'r':
			cout << "  - South Pole of Rotation: ";
			southPole.clear();
			tokens(optarg, southPole);

            THROW_IF_NOT_OF_LENGTH(southPole, 2, optarg);
			lat = atof(southPole[0].c_str());
			lon = atof(southPole[1].c_str());
			output_->southPole(lat,lon);
			cout << "lat: " << lat << " lon: " << lon << endl;
			break;
         case 'g':
            THROW_IF_NOT_INT(optarg);
			number = atoi(optarg);
			cout << "  - Gaussian Number: " << number << endl;
			output_->gaussianNumber(number);
			break;
         case 'l':
			cout << "  - Lat/Lon increments: ";
			increments.clear();
			tokens(optarg, increments);

            THROW_IF_NOT_OF_LENGTH(increments, 2, optarg);
			we = atof(increments[0].c_str());
			ns = atof(increments[1].c_str());
			output_->increments(we,ns);
			cout << " west-east: " << format_increments_value(we) << " north-south: " << format_increments_value(ns) << endl;
			break;
         case 'n':
			cout << "  - Number of points: ";
			points.clear();
           	tokens(optarg, points);

            THROW_IF_NOT_OF_LENGTH(points, 2, optarg);
           	wePoints = atol(points[0].c_str());
            nsPoints = atol(points[1].c_str());
            output_->numberOfPoints(nsPoints, wePoints);
            cout << " west-east: " << wePoints << " north-south: " << nsPoints << endl;
            break;
         case ARG_LSM_SOURCE:
            cout << "  - Lsm Method: "<< optarg << endl;
            output_->lsmMethod(optarg);
            break;
         case 'y':
            cout << "  - Grid type: "<< optarg << endl;
         	output_->gridSpec(optarg);
            break;
         case ARG_DISABLE_VECTORS:
            cout << "  - Disable vort,div to u,v Conversion;" << endl;
			output_->vdConversion(false);
            break;
         case ARG_BITMAP_FILE:
			output_->bitmapFile(optarg);
            cout << "  - Bitmap file: " << optarg << endl;
            break;
         case ARG_FRAME_LINES:
            THROW_IF_NOT_INT(optarg);
			number = atoi(optarg);
			output_->frameNumber(number);
			cout << "  - Frame number: " << number << endl;
			break;
         case 'p':
            // replace dashes with underscores
            // so that polar-stereographic and polar_stereographic work
            opt = std::string(optarg);
            std::replace(opt.begin(), opt.end(), '-', '_');
			output_->gridType(opt);
            cout << "  - Projection: " << optarg << endl;
			break;
         case 'a':
			cout << "  - Area: ";
			area.clear();
			tokens(optarg, area);

            THROW_IF_NOT_OF_LENGTH(area, 4, optarg);
			north = atof(area[0].c_str());
			west  = atof(area[1].c_str());
			south = atof(area[2].c_str());
			east  = atof(area[3].c_str());
			cout << " north: " << north<< " west: " << west << " south: " << south << " east: " << east << endl;
			output_->area(north,west,south,east);
			break;
         case 'c':
            cout << "  - legendre Polynomials Method: "<< optarg << endl;
            output_->legendrePolynomialsMethod(optarg);
            break;
         case ARG_POLE_METHOD:
            cout << "  - Extrapolation on Pole method: "<< optarg << endl;
            output_->extrapolateOnPole(optarg);
            break;
		 case ARG_AUTO_RESOLUTION:
            cout << "  - Auresol on/off: "<< optarg << endl;
            output_->auresol(optarg);
            break;
         case ARG_SCANNING_MODE:
            cout << "  - Scanning mode: "<< optarg << endl;
            THROW_IF_NOT_INT(optarg);
			number = atoi(optarg);
			output_->scanningMode(number);
			break;
         case ARG_NUMBER_OF_NEAREST_POINTS:
            THROW_IF_NOT_INT(optarg);
			number = atoi(optarg);
			output_->numberOfNearestPoints(number);
			cout << "  - Number of Nearests Points For Average or Nearest Neighbour: " << number << endl;
			/*
			cout << "  - FFT max block size: "<< optarg << endl;
			blockSize = atoi(optarg);
			output_->fftMax(blockSize);
			*/
            break;
         case '?':
			cout << "Found an option that was not in optstring "  <<  endl;
			//cout << argv[optind] << " " << optopt << endl;
			if (isprint(optopt))
				cout << "-" << (char)optopt << " " << argv[optind] << endl;
			cout <<  "No output produced" << endl;
			return 1;
		}
	}

	return 0;
}

void HandleRequest::processing() const
{
    FILE* infile = 0;
    FILE* outfile = 0;
    grib_handle* windTempInput = 0;
	grib_handle* inputHandle   = 0;
	grib_handle* outputHandle  = 0;

    try {

        GribApiInput  in(inFile_);
        Factory factory;

        /* Open Input file */
        infile = fopen(inFile_.c_str(),"r");
        if (! infile) {
            throw CantOpenFile(inFile_);
        }


        int count1 = 1;
        int count2 = 1;
        bool windProcess = false;
        int windNumber = 0;
		bool windPairFind = false;

        while((inputHandle = in.next(infile)) ) {

            /* open outfile now we have valid input */
            if (!outfile)
            {
                outfile = fopen(outFile_.c_str(),"wa");
                if (! outfile) throw CantOpenFile(outFile_);
            }

            long param = 0, table = 0;
            char levelType[80];
            size_t sizeType = sizeof(levelType);
            GRIB_CHECK(grib_get_long(inputHandle,"paramId",&param),0);
            GRIB_CHECK(grib_get_string(inputHandle,"levelType",levelType,&sizeType),0);
            if(param > 1000) {
                table = param / 1000;
                param = param % 1000;
            }
            else
                table = 128;

            Parameter parameter(param,table,levelType);
        // Wind----------------------------------------------------
            if(parameter.wind() && (isVortDivConversion(param) || output_->isRotated())){
                    if(!windProcess){
                        windTempInput = grib_handle_clone(inputHandle);
                        windNumber    = parameter.number();
                        windProcess = true;
                    }
                    else {
                        if(DEBUG){
                            cout  << "****** HandleRequest::processing  WIND  ******** 1. " << windNumber << "  2. " <<  parameter.number() << endl;
                        }
                        if(isWindPair(windNumber, parameter.number())){
							windPairFind = true;
                            if(DEBUG)
                                cout  << "** WIND PAIR **** " << endl;
                            vector_grib_handle* wind;
                            int err = 0;
                            // Check order - has to be u then v
                            if(pairOrder(windNumber,parameter.number())){
                                wind = ecregrid_process_vector_grib_handle_to_grib_handle(windTempInput,inputHandle,output_.get(),&err);
                                if(err)
                                    throw Failed("ECREGRID: Processing vector grib handle to grib handle");
                                }
                            else{
                                wind = ecregrid_process_vector_grib_handle_to_grib_handle(inputHandle,windTempInput,output_.get(),&err);
                                if(err)
                                    throw Failed("ECREGRID: Processing vector grib handle to grib handle");
                            }
                            GribApiOutput outW;
                            if(outFileType_ == "grib"){
                                outW.writeToFileGrib(outfile,wind->one);
                                outW.writeToFileGrib(outfile,wind->two);
                            }
                            else{
                                throw UserError("Output must be grib");
                                    //out->write(outfile,uv->getU());
                                    //out->write(outfile,uv->getV());
                            }
                            windProcess = false;
                            windNumber  = 0;
                            if(inputHandle) {grib_handle_delete(inputHandle); inputHandle=NULL;}
                            if(windTempInput) {grib_handle_delete(windTempInput); windTempInput=NULL;}
                            if (wind)
                            {
                                if (wind->one)
                                    grib_handle_delete(wind->one);
                                if (wind->two)
                                    grib_handle_delete(wind->two);
                            }
                        }
                        else{
                            throw BadParameter("There is no pair of wind params!");
                        }
                    }
                    continue;
            }
            if(outFileType_ == "grib"){
                GribApiOutput out;
                int err = 0;
                outputHandle = ecregrid_process_grib_handle_to_grib_handle(inputHandle,output_.get(),&err);
                if(err)
                    throw Failed("ECREGRID: Processing grib handle to grib handle failed");

                if(outputHandle){
                    if(getenv("ECREGRID_WRITE_CHUNKS"))
                        out.writeToFileGribInChunks(outfile,outputHandle);
                    else
                        out.writeToFileGrib(outfile,outputHandle);
                }
                else{
                    if(getenv("ECREGRID_WRITE_CHUNKS"))
                        out.writeToFileGribInChunks(outfile,inputHandle);
                    else
                        out.writeToFileGrib(outfile,inputHandle);
                }
            }
            else if(outFileType_ == "ascii"){
				AsciiInput asciiOut(output_->ft_.listOfPointsFile_);
                vector<Point> points;
                asciiOut.getLatLonValues(points);
				long length = points.size();

                // CACHING OPPORTUNITY:
                vector<double> values(length);

				FieldDescriptionGribApi process;
				process.extractListOfPoints(inputHandle,points,*output_,values);
                length = values.size();
				for(long i = 0; i < length; i++){
		       		 if(SMALL_NUMBERS)
		           		 fprintf(outfile,"%f  %f  %8.15f \n",points[i].latitude(),points[i].longitude(), values[i]);
        			else
            			fprintf(outfile,"%f  %f  %f \n",points[i].latitude(),points[i].longitude(),values[i]);
				}
			}
            else{
                size_t outputSize = 0;
                int err = 0;
                auto_ptr<double> temp(ecregrid_process_grib_handle_to_unpacked(inputHandle,output_.get(),&outputSize,&err));
                if(err)
                    throw Failed("ECREGRID: Processing grib_handle  to unpacked failed");
                auto_ptr<Output> out_diff(factory.getOutputBinTxt(outFile_,outFileType_));

                // need to copy to stl vector to write
                //
                vector<double> dat(outputSize);
                std::copy(temp.get(), temp.get() + outputSize, dat.begin());
                out_diff->write(outfile, dat);
            }
            if(DEBUG){
                cout << "HandleRequest::processing -- Message - " << count1++ << " Well Done!" << endl ;
                cout << "-------------------------------------------------------------- " << endl ;
            }
                cout << "ECREGRID: Interpolate product number - " << count2++ << endl ;

            if(inputHandle) {
                grib_handle_delete(inputHandle); inputHandle=NULL;
            }
            if(outputHandle) {

                grib_handle_delete(outputHandle); outputHandle=NULL;}
            if(windTempInput) {
                grib_handle_delete(windTempInput); windTempInput=NULL;}
       }

		if(windTempInput && !windPairFind)
          throw Failed("Processing grib handle to grib handle failed. Probably  Wind pair is not Found. Please try to set -d option");


       if(inputHandle) {grib_handle_delete(inputHandle); inputHandle=NULL;}
       if(outputHandle) {grib_handle_delete(outputHandle); outputHandle=NULL;}
       if(windTempInput) {grib_handle_delete(windTempInput); windTempInput=NULL;}
       if (infile)
           fclose(infile);
       if (outfile)
           fclose(outfile);

    }
    catch(...)
    {
        /* tidy up after ourselves */
       if(inputHandle) {grib_handle_delete(inputHandle); inputHandle=NULL;}
       if(outputHandle) {grib_handle_delete(outputHandle); outputHandle=NULL;}
       if(windTempInput) {grib_handle_delete(windTempInput); windTempInput=NULL;}
       if (infile)
           fclose(infile);
       if (outfile)
       {
           fclose(outfile);
           // if outfile has been opened, we delete what we have put there
           // otherwise, leave any existing files as they are
           remove(outFile_.c_str());
       }
       throw; // to caller
    }

}

bool HandleRequest::isWindPair(int u, int v) const
{
	if( u == 131 && v == 132)
		return true;
	if( u == 132 && v == 131)
		return true;
	if( u == 165 && v == 166)
		return true;
	if( u == 166 && v == 165)
		return true;
	if( u == 138 && v == 155)
		return true;
	if( u == 155 && v == 138)
		return true;
	return false;
}

bool HandleRequest::pairOrder(int u, int v) const
{
	if(DEBUG)
		cout << "HandleRequest::pairOrder -- u " << u <<  " v  " << v << endl ;

	if( u == 131 && v == 132)
		return true;
	if( u == 165 && v == 166)
		return true;
	if( u == 138 && v == 155)
		return true;
	if( u == 132 && v == 131)
		return false;
	if( u == 166 && v == 165)
		return false;
	if( u == 155 && v == 138)
		return false;
	return false;
}


void HandleRequest::print(ostream& out) const
{
	out << "HandleRequest{ Input file: " << inFile_ << "   Output file: " << outFile_ << " }" ;
}
