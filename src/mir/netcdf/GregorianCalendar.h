/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_netcdf_GregorianDateCodec
#define mir_netcdf_GregorianDateCodec

#include "eckit/types/DateTime.h"
#include "mir/netcdf/Calendar.h"

namespace mir {
namespace netcdf {


class GregorianCalendar : public Calendar {
public:
    GregorianCalendar(const Variable& variable);
    virtual ~GregorianCalendar();

private:
    eckit::DateTime reference_;
    long long offset_;
    mutable long long zero_;

    std::string units_;
    std::string calendar_;

    // -- Methods

    virtual void print(std::ostream& s) const;

    template <class T>
    void _decode(std::vector<T>& v) const;

    virtual void decode(std::vector<double>&) const;
    virtual void decode(std::vector<float>&) const;
    virtual void decode(std::vector<long>&) const;
    virtual void decode(std::vector<short>&) const;
    virtual void decode(std::vector<unsigned char>&) const;
    virtual void decode(std::vector<long long>&) const;

    virtual void encode(std::vector<double>&) const;
    virtual void encode(std::vector<float>&) const;
    virtual void encode(std::vector<long>&) const;
    virtual void encode(std::vector<short>&) const;
    virtual void encode(std::vector<unsigned char>&) const;
    virtual void encode(std::vector<long long>&) const;

    virtual void addAttributes(Variable&) const;
    virtual void updateAttributes(int nc, int varid, const std::string& path);
};

}  // namespace netcdf
}  // namespace mir
#endif
