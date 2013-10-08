/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include <string>

#include "machine.h"
#include "Exception.h"
#include "FieldDescription.h"
#include "FieldDescriptionGribApi.h"
#include "Area.h"
#include "Constrains.h"
#include "Field.h"
#include "Wind.h"
#include "Transformer.h"
#include "Factory.h"
#include "GribApiInput.h"
#include "GribApiOutput.h"
#include "BinaryInput.h"
#include "BinaryOutput.h"
#include "ListOfPoints.h"
#include "GridField.h"

#include <unistd.h>   /* for 'gethostname' */
#include <sstream>

extern "C" {
#include <ecregrid_api.h>

#define THROW_IF_NULL(x) { if (!x) { throw BadParameter("Invalid parameter passed to ecregrid api");} } 


/*!
  \brief Writes logging information for ECMWF users

  \note Designed to work at ECMWF only! Some MetVis Section members are 
   excluded from the list.
  \warning The log file can grow very quickly and needs constantly checking.
*/
void writeLog(const std::string &interface)
{
    if (!DEBUG)
        return;
//  if(magCompare(MAGICS_SITE,"ecmwf"))
//  {
   FILE* filePtr_ = fopen("/vol/netlog/ecregrid/usage_log","a");

   if(filePtr_)
   {
	string id = getenv("USER"); //user-id
	if(! (id=="cgm" || id=="mas" || id=="maf" ) )
	{
		char            host[64];
		char            mytime[24];
		time_t          when;

		time(&when);
		strncpy(mytime,ctime(&when),24); 
		string smytime(mytime,24);      // date time

		string arch = getenv("ARCH");
		if(arch.empty()) arch = "NoArch";
		
		if(gethostname(host, sizeof(host)) == -1) strcpy(host, "NoHost");

		string inter = "-";
/*
		if(magCompare(interface,"tool")) inter = "t"; 
		else if(magCompare(interface,"c")) inter = "c"; 
		else if(magCompare(interface,"fortran")) inter = "f"; 
		else inter = "?";
*/
		ostringstream stream;
		stream << ECREGRID_MAJOR_VERSION << "." << ECREGRID_MINOR_VERSION << "." << ECREGRID_REVISION_VERSION;
		string logline = "-" + id + " " +  smytime + " - "+ stream.str() + " " + host + "-" + arch + " " + interface;
		fprintf(filePtr_,"%s\n",logline.c_str());

		fflush(filePtr_);
		fclose(filePtr_);
	}//endif NOT developers
   }
}


field_description* ecregrid_get_field_description(const char* g, int* err)
{
try{	
    THROW_IF_NULL(g)
    THROW_IF_NULL(err)
	if(DEBUG)
    	cout << "-> ecregrid_get_field_description: " << g << endl;
	return new field_description(g);
}
catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return 0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}

}

field_description* ecregrid_get_field_description_general(int* err)
{
try{	
    THROW_IF_NULL(err)
	return new field_description;
}
catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return 0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

field_description* ecregrid_get_field_description_from_grib_handle(grib_handle* handle, int* err)
{
try{	
    THROW_IF_NULL(handle)
    THROW_IF_NULL(err)
	return new field_description_grib_handle(handle);
}
catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return 0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

void ecregrid_field_description_destroy(field_description* f)
{
    if (f)
	    delete f;
}

double* ecregrid_process_grib_handle_to_unpacked(grib_handle* inHandle, field_description* output, size_t* outLength, int* err)
{
writeLog("grib_to_unpacked");
try{	
    THROW_IF_NULL(inHandle)
    THROW_IF_NULL(output)
    THROW_IF_NULL(outLength)
    THROW_IF_NULL(err)

	GribApiInput  in;
	BinaryOutput out;
	Factory factory;

    size_t valuesLength = 0;
	GRIB_CHECK(grib_get_size(inHandle,"values",&valuesLength),0);

    vector<double> values(valuesLength);

	auto_ptr<field_description_grib_handle>  input(new field_description_grib_handle(inHandle));
	auto_ptr<Field> inputField(in.defineField(inHandle,values));

	if(DEBUG)
		cout  << "ecregrid_process_grib_handle_to_unpacked: -- Input Field => " << *inputField << endl;
	auto_ptr<field_description> newOutput ( inputField->makeOutput(*output));
	*output = *newOutput;

	newOutput->ifAvailableCopyBasics(*input);

	Constrains wonder(inputField->nameOfField(), newOutput->id_.composedName_);
    bool globalOutput = newOutput->ft_.area_.empty();
    // we pass explicit global flag as we may wish to alter the way
    // that output subareas are handled. For example in the C++ API we 
    // may handle areas differently
	wonder.isPossible(newOutput.get(), globalOutput);
		
	auto_ptr<Field> outputField ( newOutput->defineField());
	if(DEBUG)
		cout  << "*** ecregrid_process_grib_handle_to_unpacked: -- Output Field " << *outputField << endl;
	auto_ptr<Transformer> trans(factory.getTransformer(*inputField,*outputField,*newOutput));
	auto_ptr<Field> end (trans->transform(*inputField, *outputField));

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL){
		end->dump2file(dump);
	}
		
	if(DEBUG){
		cout  << "*** ecregrid_process_grib_handle_to_unpacked -- Interpolated Field " << *end << endl;
	}
	*err = 0;
	end->setOutputAreaAndBasics(*output);

    // for bwd compatibility, return a new double* memory block
    vector<double> data;
    out.deliverData(*end, data);

    double* rtn = 0;
    if (data.size() > 0)
    {
        rtn = new double[data.size()];
        std::copy(data.begin(), data.end(), rtn);
    }
    *outLength = data.size();
    return rtn;

}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

grib_handle* ecregrid_process_grib_handle_to_grib_handle(grib_handle* inHandle, const field_description* output, int* err)
{
    try {

    THROW_IF_NULL(inHandle)
    THROW_IF_NULL(output)
    THROW_IF_NULL(err)

    auto_ptr<grib_values>  grib_set(0);
    int grib_set_count = 0;
	return ecregrid_process_grib_handle_to_grib_handle_grib_set(inHandle,output,grib_set.get(),grib_set_count,err);
    }
catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

grib_handle* ecregrid_process_grib_handle_to_grib_handle_grib_set(grib_handle* inHandle, const field_description* output, const grib_values* grib_set, int grib_set_count, int* err)
{
  writeLog("grib_to_grib_set");
try{	
    THROW_IF_NULL(inHandle)
    THROW_IF_NULL(output)
    //THROW_IF_NULL(grib_set)
    THROW_IF_NULL(err)
	if(DEBUG)
	   cout  << "****** INPUT ****** " << endl;
	GribApiInput in;
	Factory factory;

    size_t valuesLength = 0;		
	GRIB_CHECK(grib_get_size(inHandle,"values",&valuesLength),0);
    vector<double> values(valuesLength);
    
	auto_ptr<Field> inputField ( in.defineField(inHandle,values));
    
	if(DEBUG){
	   cout  << "ecregrid_process_grib_handle_to_grib_handle_grib_set: -- Input Field => " << *inputField << endl;
	  }

	if(DEBUG){
	   cout  << "****** OUTPUT ****** " << endl;
	}
	auto_ptr<field_description> newOutput ( inputField->makeOutput(*output));
	newOutput->isAvailable();

	Constrains wonder(inputField->nameOfField(), newOutput->id_.composedName_);
    bool globalOutput = newOutput->ft_.area_.empty();
    // we pass explicit global flag as we may wish to alter the way
    // that output subareas are handled. For example in the C++ API we 
    // may handle areas differently
	wonder.isPossible(newOutput.get(), globalOutput);
		
	auto_ptr<Field> outputField ( newOutput->defineField());
	auto_ptr<GribApiOutput> out(factory.getGribApiOutput(outputField->nameOfField()));

    
	if(DEBUG){
	   cout  << "-----------------------------------------------" << endl;
	   cout  << "*** ecregrid_process_grib_handle_to_grib_handle_grib_set: -- Output Field => " << *outputField << endl;
	   }

	if(DEBUG){
	   cout  << "****** TRANSFORMER ****** " << endl;
	}
	auto_ptr<Transformer> trans(factory.getTransformer(*inputField,*outputField,*newOutput));

	auto_ptr<Field> end( trans->transform(*inputField, *outputField));
	if(!end.get()){
		return NULL;
	}

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL){
		end->dump2file(dump);
	}

	if(DEBUG)
		cout  << "*** ecregrid_process_grib_handle_to_grib_handle_grib_set -- Interpolated Field " << *end << endl;

	grib_handle* hend = out->createSetCopyGribHandle(*end, inHandle, grib_set, grib_set_count);
	ASSERT(hend);

	*err = 0;

	return hend;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

vector_grib_handle* ecregrid_process_vector_grib_handle_to_grib_handle(grib_handle* inHandleU, grib_handle* inHandleV, const field_description* output, int* err)
{
    try {
    THROW_IF_NULL(inHandleU)
    THROW_IF_NULL(inHandleV)
    THROW_IF_NULL(output)
    THROW_IF_NULL(err)
    grib_values*  grib_set = 0;
    int grib_set_count = 0;


	return ecregrid_process_vector_grib_handle_to_grib_handle_grib_set(inHandleU,inHandleV,output,grib_set,grib_set_count,err);
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

vector_grib_handle* ecregrid_process_vector_grib_handle_to_grib_handle_grib_set(grib_handle* inHandleU, grib_handle* inHandleV, const field_description* output, const grib_values* grib_set, int grib_set_count, int* err)
{
writeLog("vector_grib_to_grib_set");
try{	
    THROW_IF_NULL(inHandleU)
    THROW_IF_NULL(inHandleV)
    THROW_IF_NULL(output)
    //THROW_IF_NULL(grib_set)
    THROW_IF_NULL(err)

	GribApiInput  inU;
	GribApiInput  inV;
	Factory factory;

    size_t valuesLengthU         = 0;		
    size_t valuesLengthV         = 0;		
	GRIB_CHECK(grib_get_size(inHandleU,"values",&valuesLengthU),0);
	GRIB_CHECK(grib_get_size(inHandleV,"values",&valuesLengthV),0);
    
    vector<double> valuesU(valuesLengthU);
    vector<double> valuesV(valuesLengthV);

	auto_ptr<Field> inputU ( inU.defineField(inHandleU,valuesU));
	if(DEBUG)
	 cout  << "-------------------------------- " <<  endl;
	auto_ptr<Field> inputV ( inV.defineField(inHandleV,valuesV));

	if(DEBUG){
	   cout  << "ecregrid_process_vector_grib_handle_to_grib_handle_grib_set: -- Input Field U => " << *inputU << endl;
	   cout  << "ecregrid_process_vector_grib_handle_to_grib_handle_grib_set: -- Input Field V => " << *inputV << endl;
	 }

	auto_ptr<field_description> newOutput ( inputU->makeOutput(*output));
	newOutput->isAvailable();

	Constrains wonder(inputU->nameOfField(), newOutput->id_.composedName_);
    bool globalOutput = newOutput->ft_.area_.empty();
    // we pass explicit global flag as we may wish to alter the way
    // that output subareas are handled. For example in the C++ API we 
    // may handle areas differently
	wonder.isPossible(newOutput.get(), globalOutput);
		
	auto_ptr<Field> required ( newOutput->defineField());

	auto_ptr<GribApiOutput> out(factory.getGribApiOutput(required->nameOfField()));

	auto_ptr<Transformer> trans(factory.getTransformer(*inputU,*required,*newOutput));

	auto_ptr<Wind> outWind ( trans->transformVector(*inputU,*inputV,*required));

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL){
		const string dump2(dump);
		outWind->getU().dump2file(dump2+"U");
		outWind->getV().dump2file(dump2+"V");
	}

	if(outWind->isSame()){
		if(DEBUG){
	   		cout  << "ecregrid_process_vector_grib_handle_to_grib_handle_grib_set: -- SAME " << endl;
		}
		*err = 0;
		return NULL;
	}

	vector_grib_handle* vectorParam = new vector_grib_handle;

//	cout  << "*** ecregrid_process_wind_grib_handle_to_grib_handle: -- Output Field U" << outWind->getU() << endl;
//	cout  << "*** ecregrid_process_wind_grib_handle_to_grib_handle: -- Output Field V" << outWind->getV() << endl;

	grib_handle* outHandleU = out->createSetCopyGribHandle(outWind->getU(), inHandleU, grib_set, grib_set_count);
	ASSERT(outHandleU);
	grib_handle* outHandleV = out->createSetCopyGribHandle(outWind->getV(), inHandleV, grib_set, grib_set_count);
	ASSERT(outHandleV);


	vectorParam->one = outHandleU;
	vectorParam->two = outHandleV;

	*err = 0;
	return vectorParam;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

double* ecregrid_process_unpacked_to_unpacked(const field_description* input, double* inData, size_t inLength, field_description* output, size_t* outLength, int* err)
{
writeLog("unpacked_to_unpacked");
try{	
    THROW_IF_NULL(input)
    THROW_IF_NULL(inData)
    THROW_IF_NULL(output)
    THROW_IF_NULL(err)

	BinaryInput  in;
	BinaryOutput out;
	Factory factory;

    // maintain compatibility with this method
    vector<double> values(inLength);
    if (values.size() > 0 && inData)
        std::copy(inData, inData + inLength, values.begin());

	auto_ptr<Field> inputField ( in.defineField(values, *input));

	auto_ptr<field_description> newOutput ( inputField->makeOutput(*output));
	*output = *newOutput;
	newOutput->ifAvailableCopyBasics(*input);

	Constrains wonder(inputField->nameOfField(), newOutput->id_.composedName_);
    bool globalOutput = newOutput->ft_.area_.empty();
    // we pass explicit global flag as we may wish to alter the way
    // that output subareas are handled. For example in the C++ API we 
    // may handle areas differently
	wonder.isPossible(newOutput.get(), globalOutput);
		
	auto_ptr<Field> outputField ( newOutput->defineField());
	/* cout  << "*** ecregrid_process_unpacked_to_unpacked: -- Output Field " << *outputField << endl; */
	auto_ptr<Transformer> trans(factory.getTransformer(*inputField,*outputField,*newOutput));

	auto_ptr<Field> end(trans->transform(*inputField, *outputField));
	ASSERT(end.get());

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL){
		end->dump2file(dump);
	}

	*err = 0;
	end->setOutputAreaAndBasics(*output);
    // for bwd compatibility, return block of data
    vector<double> data;
    out.deliverData(*end, data);

    double* rtn = 0;
    if (data.size() > 0)
    {
        rtn = new double[data.size()];
        std::copy(data.begin(), data.end(), rtn);
    }
    *outLength = data.size();
    return rtn;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return 0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

grib_handle* ecregrid_process_unpacked_to_grib_handle(const field_description* input, double* inData, size_t inLength, const field_description* output, int* err)
{
writeLog("unpacked_to_grib");
try{	
    THROW_IF_NULL(input)
    THROW_IF_NULL(inData)
    THROW_IF_NULL(output)
    THROW_IF_NULL(err)
    
	BinaryInput  in;
	Factory factory;

    // copy the double* of existing api to std::vector
    vector<double> values(inLength);
    if (inLength > 0 && inData)
        std::copy(inData, inData + inLength, values.begin());

	auto_ptr<Field> inputField ( in.defineField(values, *input));
	if(DEBUG)
		cout  << "*** ecregrid_process_unpacked_to_grib_handle: -- Input Field " << *inputField << endl;

	auto_ptr<field_description> newOutput ( inputField->makeOutput(*output));
	newOutput->isAvailable();

	Constrains wonder(inputField->nameOfField(), newOutput->id_.composedName_);
    bool globalOutput = newOutput->ft_.area_.empty();
    // we pass explicit global flag as we may wish to alter the way
    // that output subareas are handled. For example in the C++ API we 
    // may handle areas differently
	wonder.isPossible(newOutput.get(), globalOutput);
		
	auto_ptr<Field> outputField ( newOutput->defineField());

	auto_ptr<GribApiOutput> out(factory.getGribApiOutput(outputField->nameOfField()));

	if(DEBUG)
		cout  << "*** ecregrid_process_unpacked_to_grib_handle: -- Output Field " << *outputField << endl;
	auto_ptr<Transformer> trans(factory.getTransformer(*inputField,*outputField,*newOutput));

	auto_ptr<Field> end ( trans->transform(*inputField, *outputField));

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL){
		end->dump2file(dump);
	}

	grib_handle* hend = NULL;
	if(end.get())
		hend = out->createSetGribHandle(*end);
	else
		hend = out->createSetGribHandle(*inputField);

	ASSERT(hend);

	*err = 0;

	return hend;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

grib_handle* ecregrid_process_read_from_file_to_grib_handle(const field_description* input, const char* fileName, const field_description* output, int* err)
{
writeLog("from_file_to_grib");
try{	
    THROW_IF_NULL(input)
    THROW_IF_NULL(fileName)
    THROW_IF_NULL(output)
    THROW_IF_NULL(err)

	Factory factory;

	auto_ptr<Input> in(factory.getInput(fileName,input->fileType()));

	auto_ptr<Field> inputField ( in->defineField(*input));
	if(DEBUG)
		cout  << "*** ecregrid_process_read_from_file_to_grib_handle: -- Input Field " << *inputField << endl;

	auto_ptr<field_description> newOutput ( inputField->makeOutput(*output));
	newOutput->isAvailable();

	Constrains wonder(inputField->nameOfField(), newOutput->id_.composedName_);
    bool globalOutput = newOutput->ft_.area_.empty();
    // we pass explicit global flag as we may wish to alter the way
    // that output subareas are handled. For example in the C++ API we 
    // may handle areas differently
	wonder.isPossible(newOutput.get(), globalOutput);
		
	auto_ptr<Field> outputField ( newOutput->defineField());

	auto_ptr<GribApiOutput> out(factory.getGribApiOutput(outputField->nameOfField()));

	if(DEBUG)
		cout  << "*** ecregrid_process_read_from_file_to_grib_handle: -- Output Field " << *outputField << endl;
	auto_ptr<Transformer> trans(factory.getTransformer(*inputField,*outputField,*newOutput));

	auto_ptr<Field> end ( trans->transform(*inputField, *outputField));
	ASSERT(end.get());

	char *dump = getenv("ECREGRID_DUMP_TO_FILE");
	if(dump!=NULL){
		end->dump2file(dump);
	}

//	grib_handle* hend = out->createSetCopyGribHandle(*end, 0, grib_set, grib_set_count);
	grib_handle* hend = out->createSetCopyGribHandle(*end, 0, 0, 0);
	ASSERT(hend);

	*err = 0;

	return hend;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	*err = 1;
	return  0;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	*err = 2;
	return 0;
}
}

int ecregrid_set_file_type(field_description* f, const char* n)
{

try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(n)
	f->fileType(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_area(field_description* f, double n, double w, double s, double e)
{

try{	
    THROW_IF_NULL(f)
	f->area(n,w,s,e);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_parameter_id(field_description* f, int paramId)
{
try{	
    THROW_IF_NULL(f)
	f->parameterId(paramId);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_table(field_description* f, int tab)
{
try{	
    THROW_IF_NULL(f)
	f->table(tab);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_number_of_points(field_description* f, long ns, long we)
{

try{	
    THROW_IF_NULL(f)
	f->numberOfPoints(ns,we);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_south_pole(field_description* f, double lat, double lon)
{

try{	
    THROW_IF_NULL(f)
	f->southPole(lat,lon);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_increments(field_description* f, double we, double ns)
{

try{	
    THROW_IF_NULL(f)
	f->increments(we,ns);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_truncation(field_description* f, int t)
{

try{	
    THROW_IF_NULL(f)
	f->truncation(t);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_time(field_description* f, int t)
{

try{	
    THROW_IF_NULL(f)
	f->date(t);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_date(field_description* f, int d)
{

try{	
    THROW_IF_NULL(f)
	f->date(d);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_number_of_nearest_points(field_description* f, int n)
{

try{	
    THROW_IF_NULL(f)
	f->numberOfNearestPoints(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_gaussian_number(field_description* f, int n)
{

try{	
    THROW_IF_NULL(f)
	f->gaussianNumber(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
	return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
	return 2;
}
}

int ecregrid_set_scanning_mode(field_description* f, int n)
{
try{	
    THROW_IF_NULL(f)
	f->scanningMode(n);
    return 0;
}
catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_is_reduced(field_description* f, int n)
{

try{	
    THROW_IF_NULL(f)
	if(n)
		f->isReduced(true);
	else	
		f->isReduced(false);

    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}


int ecregrid_set_reduced_grid_definition(field_description* f, long* rgridDef, size_t size)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(rgridDef)
	f->reducedGridDefinition(rgridDef,size);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_frame_number(field_description* f, int n)
{
try{	
    THROW_IF_NULL(f)
	f->frameNumber(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_bitmap_file(field_description* f, const char* n)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(n)
	f->bitmapFile(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_list_of_points_file(field_description* f, const char* n)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(n)
	f->listOfPointsFile(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_list_of_points_file_type(field_description* f, const char* n)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(n)
	f->listOfPointsFileType(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_interpolation_method(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(m)
	f->interpolationMethod(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_lsm_method(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(m)
	f->lsmMethod(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_edition_number(field_description* f, int n)
{
try{	
    THROW_IF_NULL(f)
	f->editionNumber(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}
/*
int ecregrid_set_level_type(field_description* f, const char* l)
{
try{	
	f->levelType(l);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_level(field_description* f, int l)
{
try{	
	f->level(l);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_units(field_description* f, const char* m)
{
try{	
	f->units(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_centre(field_description* f, int n)
{
try{	
	f->centre(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}


int ecregrid_set_step_units(field_description* f, const char* m)
{
try{	
	f->stepUnits(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}
*/

int ecregrid_set_start_step(field_description* f, int n)
{
try{	
    THROW_IF_NULL(f)
	f->startStep(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_end_step(field_description* f, int n)
{
try{	
    THROW_IF_NULL(f)
	f->endStep(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_auresol(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(m)
	f->auresol(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_global_west_east(field_description* f, const char* yesno)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(yesno)
	f->globalWestEast(yesno);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_derived_parameter(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(m)
	f->grid2gridTransformationType(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_legendre_polynomials_method(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(m)
	f->legendrePolynomialsMethod(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_fft_max_block_size(field_description* f, int m)
{
try{	
    THROW_IF_NULL(f)
	f->fftMax(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_grid_type(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(m)
	f->gridSpec(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_extrapolate_on_pole(field_description* f, const char* m)
{
try{	
    THROW_IF_NULL(f)
	f->extrapolateOnPole(m);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_set_missing_value(field_description* f, double value)
{
try{	
    THROW_IF_NULL(f)
	f->missingValue(value);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

// Get Information 

int ecregrid_get_area_north(const field_description* f, double* north)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(north)
    *north = f->north();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_area_west(const field_description* f, double* west)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(west)
    *west = f->west();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_area_south(const field_description* f, double* south)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(south)
    *south = f->south();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_area_east(const field_description* f, double* east)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(east)
    *east = f->east();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_south_pole_lat(const field_description* f, double* lat)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(lat)
    *lat = f->southPoleLat();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_south_pole_lon(const field_description* f, double* lon)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(lon)
    *lon = f->southPoleLon();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_parameter_number(const field_description* f, int* number)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(number)
    return f->parameterNumber();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_parameter_table(const field_description* f, int* table)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(table)
    *table = f->parameterTable();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_date(const field_description* f, int* date)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(date)
    *date = f->date();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_time(const field_description* f, int* time)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(time)
    *time = f->time();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_scanning_mode(const field_description* f, int* scan)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(scan)
    *scan =  f->scanningMode();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_gaussian_number(const field_description* f, int* number)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(number)
    *number = f->gaussianNumber();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_truncation(const field_description* f, int* truncation)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(truncation)
    *truncation =  f->truncation();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_west_east_increment(const field_description* f, double* we)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(we)
	*we = f->westEastInc();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_west_east_number_of_points(const field_description* f, long* we)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(we)
	*we = f->westEastNumberOfPoints();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_get_north_south_number_of_points(const field_description* f, long* we)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(we)
	*we = f->northSouthNumberOfPoints();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}
int ecregrid_get_north_south_increment(const field_description* f, double* ns)
{
try{	
    THROW_IF_NULL(f)
    THROW_IF_NULL(ns)
    *ns = f->northSouthInc();
	return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

long ecregrid_get_version(){
	return ECREGRID_VERSION;
}

int ecregrid_one_point(grib_handle* inHandle, double* lat, double* lon , double* value)
{
    try {
    THROW_IF_NULL(inHandle)
    THROW_IF_NULL(lat)
    THROW_IF_NULL(lon)
    THROW_IF_NULL(value)
writeLog("ecregrid_one_point");
	long length = 1;
	return ecregrid_list_of_points(inHandle,lat,lon,value,&length);
    }
catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

int ecregrid_list_of_points(grib_handle* inHandle, double* lats, double* lons, double* outValues, long* length)
{
    writeLog("ecregrid_list_of_points");
    try{
        THROW_IF_NULL(lats);
        THROW_IF_NULL(lons);
        THROW_IF_NULL(outValues);
        THROW_IF_NULL(length);
        int err = 0;
        auto_ptr<field_description> f_output ( ecregrid_get_field_description("list",&err));
        if(err != 0){
            printf("ERROR: ecregrid_list_of_points - unable to create field description \n");
            return 1;
        }

        vector<Point> points;
        for (long i = 0 ; i < *length ; i++) {
            points.push_back(Point(lats[i],lons[i]));
        }
        FieldDescriptionGribApi process;
        vector<double> values;
        process.extractListOfPoints(inHandle, points, *f_output, values);

        // convert to double* for bwd compatibility
        // write what we can to the output buffer
        for (unsigned int i = 0; i < values.size() && i < *length; i++)
            outValues[i] = values[i];

        // we also report what we have written to the caller...
        *length = values.size();

        return 0;
    }
    catch(Exception& e){
        cout << e ;
        cout << "-> ecRegrid exception" << endl;
        return  1;
    }
    catch(...){
        cout << "-> Unknown exception" << endl;
        return 2;
    }
}

int ecregrid_set_bits_per_value(field_description* f, int n)
{
try{	
    THROW_IF_NULL(f)
	f->bitsPerValue(n);
    return 0;
}

catch(Exception& e){
    cout << e ;
    cout << "-> ecRegrid exception" << endl;
    return 1;
}
catch(...){
    cout << "-> Unknown exception" << endl;
    return 2;
}
}

} // end of extern "C"

