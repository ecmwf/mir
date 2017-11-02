/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_param_RulesFromFile_h
#define mir_param_RulesFromFile_h

#include "mir/param/Rules.h"


namespace mir {
namespace param {


struct RulesFromFile : public Rules {

    // -- Methods

    void initialize();

};


}  // namespace param
}  // namespace mir


#endif

