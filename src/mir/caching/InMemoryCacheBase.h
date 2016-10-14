/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @date May 2015

#ifndef mir_caching_InMemoryCacheBase_H
#define mir_caching_InMemoryCacheBase_H

#include <string>
#include <map>

#include "eckit/memory/NonCopyable.h"
// #include "eckit/memory/ScopedPtr.h"
// #include "eckit/thread/AutoLock.h"
// #include "eckit/thread/Mutex.h"
// #include "eckit/config/Resource.h"

namespace mir {
class InMemoryCacheStatistics;

//----------------------------------------------------------------------------------------------------------------------

class InMemoryCacheBase : public eckit::NonCopyable {

public:  // methods


    InMemoryCacheBase();

    virtual ~InMemoryCacheBase();

    static unsigned long long totalFootprint();

protected:

    void checkTotalFootprint();

private:

    virtual unsigned long long footprint() const = 0;
    virtual unsigned long long capacity() const = 0;
    virtual void purge(size_t amount) = 0;
    virtual const std::string& name() const = 0;

};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir


#endif
