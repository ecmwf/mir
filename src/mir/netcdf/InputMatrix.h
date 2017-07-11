/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#ifndef mir_netcdf_InputMatrix
#define mir_netcdf_InputMatrix

#include "mir/netcdf/Matrix.h"

namespace mir {
namespace netcdf {

class NCFile;


class InputMatrix : public Matrix {
public:
    InputMatrix(Type &type, int varid, const std::string &name, size_t size, NCFile &file);
    virtual ~InputMatrix();

private:
    int varid_;
    NCFile &file_;

    // Methods
    virtual void read(std::vector<double> &) const ;
    virtual void read(std::vector<float> &) const ;
    virtual void read(std::vector<long> &) const ;
    virtual void read(std::vector<short> &) const ;
    virtual void read(std::vector<unsigned char> &) const ;
    virtual void read(std::vector<long long> &) const ;

    virtual void read(std::vector<double> &, const std::vector<size_t>& start, const std::vector<size_t>& count) const ;
    virtual void read(std::vector<float> &, const std::vector<size_t>& start, const std::vector<size_t>& count) const ;
    virtual void read(std::vector<long> &, const std::vector<size_t>& start, const std::vector<size_t>& count) const ;
    virtual void read(std::vector<short> &, const std::vector<size_t>& start, const std::vector<size_t>& count) const ;
    virtual void read(std::vector<unsigned char> &, const std::vector<size_t>& start, const std::vector<size_t>& count) const ;
    virtual void read(std::vector<long long> &, const std::vector<size_t>& start, const std::vector<size_t>& count) const ;


    template<class T> void _fill(Mapper<T> &v) const;

    // Overridden methods
    virtual void fill(Mapper<double> &) const;
    virtual void fill(Mapper<float> &) const;
    virtual void fill(Mapper<long> &) const;
    virtual void fill(Mapper<short> &) const;
    virtual void fill(Mapper<unsigned char> &) const;
    virtual void fill(Mapper<long long> &) const;

    virtual void print(std::ostream &out)  const;


};

}
}
#endif
