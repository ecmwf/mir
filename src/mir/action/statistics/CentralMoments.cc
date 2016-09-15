/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016
//

#include "mir/action/statistics/CentralMoments.h"

//#nclude <map>
//#nclude <ostream>
//#nclude "eckit/exception/Exceptions.h"
//#nclude "eckit/log/Log.h"
//#nclude "eckit/log/Plural.h"
//#nclude "eckit/thread/AutoLock.h"
//#nclude "eckit/thread/Mutex.h"
//#nclude "eckit/thread/Once.h"
//#nclude "mir/action/context/Context.h"
//#nclude "mir/config/LibMir.h"
//#nclude "mir/input/MIRInput.h"


namespace mir {
namespace action {
namespace statistics {


CentralMoments::CentralMoments(const param::MIRParametrisation& parametrisation) :
    Statistics(parametrisation) {
}


bool CentralMoments::sameAs(const action::Action& other) const {
    const CentralMoments* o = dynamic_cast<const CentralMoments*>(&other);
    return o; //(o && options_ == o->options_);
}


void CentralMoments::calculate(const data::MIRField&, Results&) const {
}


namespace {
static StatisticsBuilder<CentralMoments> statistics("CentralMoments");
}


}  // namespace statistics
}  // namespace action
}  // namespace mir

