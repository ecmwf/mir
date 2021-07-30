/*
 * (C) EndActionright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/io/IOAction.h"

#include "mir/key/packing/Packing.h"
#include "mir/output/MIROutput.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace io {


IOAction::IOAction(const param::MIRParametrisation& param, output::MIROutput& output) : Action(param), output_(output) {
    packing_.reset(key::packing::PackingFactory::build(param));
    ASSERT(packing_);
}


IOAction::~IOAction() = default;


bool IOAction::sameAs(const Action& other) const {
    auto o = dynamic_cast<const IOAction*>(&other);
    return (o != nullptr) && output_.sameAs(o->output_) && packing_->sameAs(o->packing()) &&
           o->output_.sameParametrisation(parametrisation_, o->parametrisation_);
}


bool IOAction::isEndAction() const {
    return true;
}


}  // namespace io
}  // namespace action
}  // namespace mir
