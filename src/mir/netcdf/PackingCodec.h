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


#ifndef mir_netcdf_PackingCodec_h
#define mir_netcdf_PackingCodec_h

#include "mir/netcdf/Codec.h"


namespace mir {
namespace netcdf {


class PackingCodec : public Codec {
public:
    PackingCodec(const Variable&);
    virtual ~PackingCodec();

private:
    double scale_factor_;
    double add_offset_;

    // -- Methods
    virtual void print(std::ostream&) const;
    virtual void decode(std::vector<double>&) const;
};


}  // namespace netcdf
}  // namespace mir


#endif
