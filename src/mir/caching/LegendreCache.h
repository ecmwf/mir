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
/// @date Oct 2013

#ifndef mir_method_LegendreCache_H
#define mir_method_LegendreCache_H


#include "eckit/container/CacheManager.h"

namespace mir {
namespace caching {

//----------------------------------------------------------------------------------------------------------------------

class LegendreCache : public eckit::CacheManager {

  public:  // methods

    LegendreCache();


  protected:

    virtual void print(std::ostream& s) const;

  private:

    virtual const char* version() const;
    virtual const char* extension() const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
