/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef GribApiOutput_H
#define GribApiOutput_H

#ifndef Output_H
#include "Output.h"
#endif

#include "grib_api.h"

// Forward declarations
class Field;
class GribApiInput;

class GribApiOutput : public Output {
  public:

    // -- Contructors

    GribApiOutput();
    GribApiOutput(const string &name);

    // -- Destructor

    ~GribApiOutput();

    // --  Methods
    void    writeToFileGrib(grib_handle *h) const;
    void    writeToFileGrib(FILE *out, grib_handle *h) const;

    grib_handle *createSetCopyGribHandle(const Field &out, grib_handle *hin, const grib_values *grib_set, int grib_set_count) const;
    grib_handle *createSetGribHandle(const Field &out) const;

    virtual void         setGrib(grib_handle *h, const Field &out, const grib_values *grib_set, int grib_set_count) const;

    virtual grib_handle *setGrib(const Field &out, const grib_values *grib_set, int grib_set_count) const;
    void                 setGrib(grib_handle *handle, const Field &out) const;
    grib_handle         *setGrib(const Field &out) const;

    void createGribAndWriteToFile(const Field &out) const;
    void createGribAndWriteToFile(FILE *outfile, const Field &out) const;
    // enrico GribApiInput* to be const
    void createGribAndWriteToFile(FILE *outfile, const Field &out, grib_handle *in) const;
    void writeToFileGribInChunks(FILE *outfile, grib_handle *h) const;
    void writeToFileGribInChunks(grib_handle *h) const;
    void writeToMultiFileGrib(grib_multi_handle *mh, grib_handle *h) const;

    void scanningMode(const GridField &out, long &iScansNegatively, long &jScansPositively) const;


    // Overriden methods
    void     write(FILE *out, const vector<double> &values) const;
    void     write(const Field &field) const;
    void     write(FILE *out, const Field &field) const;
    string   typeOf()  const {
        return "grib";
    }

  private:

    // No copy allowed
    GribApiOutput(const GribApiOutput &);
    GribApiOutput &operator=(const GribApiOutput &);

    // -- Friends

};

#endif
