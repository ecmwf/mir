/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Formula.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016


#include "mir/util/Formula.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

Formula::Formula(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

}

Formula::~Formula() {

}


//----------------------------------------------------------------------------------------------------------------------

} // namespace util
} // namespace mir
