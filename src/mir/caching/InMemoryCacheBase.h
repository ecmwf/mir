/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Oct 2016


#ifndef mir_caching_InMemoryCacheBase_h
#define mir_caching_InMemoryCacheBase_h

#include <string>
#include <map>

#include "eckit/memory/NonCopyable.h"
#include "mir/caching/InMemoryCacheUsage.h"


namespace eckit {
class Channel;
}


namespace mir {
namespace caching {


class InMemoryCacheStatistics;

class InMemoryCacheBase : public eckit::NonCopyable {

public:  // methods


    InMemoryCacheBase();

    virtual ~InMemoryCacheBase();

    static InMemoryCacheUsage totalFootprint();

protected:

    void checkTotalFootprint();
    static eckit::Channel& log();

private:

    virtual InMemoryCacheUsage footprint() const = 0;
    virtual InMemoryCacheUsage capacity() const = 0;
    virtual InMemoryCacheUsage purge(const InMemoryCacheUsage&, bool force = false) = 0;
    virtual const std::string& name() const = 0;

};


}  // namespace caching
}  // namespace mir


#endif
