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


#ifndef mir_caching_legendre_FileLoader_h
#define mir_caching_legendre_FileLoader_h

#include "eckit/io/Buffer.h"

#include "mir/caching/legendre/LegendreLoader.h"


namespace mir {
namespace caching {
namespace legendre {


class FileLoader : public LegendreLoader {
public:
    FileLoader(const param::MIRParametrisation&, const eckit::PathName&);

    virtual ~FileLoader();

    static bool shared();

protected:
    virtual void print(std::ostream&) const;

private:
    virtual const void* address() const;
    virtual size_t size() const;
    virtual bool inSharedMemory() const;

    eckit::Buffer buffer_;
};


}  // namespace legendre
}  // namespace caching
}  // namespace mir


#endif
