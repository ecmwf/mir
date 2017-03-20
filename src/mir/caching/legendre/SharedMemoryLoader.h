/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Apr 2015

#ifndef mir_caching_SharedMemoryLoader_H
#define mir_caching_SharedMemoryLoader_H

#include "mir/caching/legendre/LegendreLoader.h"

namespace mir {
namespace caching {
namespace legendre {

class SharedMemoryLoader : public LegendreLoader {
public:
    SharedMemoryLoader(const param::MIRParametrisation&, const eckit::PathName& path);

    ~SharedMemoryLoader(); // Change to virtual if base class

    static void unloadSharedMemory(const eckit::PathName& path);
    static void loadSharedMemory(const eckit::PathName& path);

protected:
    void print(std::ostream&) const; // Change to virtual if base class

private:

    void* address_;
    size_t size_;
    bool unload_;

    virtual const void* address() const;
    virtual size_t size() const;
};

} // namespace legendre
} // namespace caching
} // namespace mir

#endif
