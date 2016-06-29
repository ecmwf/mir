/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/action/plan/SimpleExecutor.h"
#include "mir/action/plan/Action.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/action/context/Context.h"
#include "mir/log/MIR.h"
#include "mir/api/MIRWatcher.h"
#include "mir/action/plan/ActionNode.h"

#include "eckit/exception/Exceptions.h"



namespace mir {
namespace action {


SimpleExecutor::SimpleExecutor() {
}


SimpleExecutor::~SimpleExecutor() {

}

void SimpleExecutor::print(std::ostream& out) const {
    out << "SimpleExecutor[]";
}


void SimpleExecutor::wait() {

}

void SimpleExecutor::execute(context::Context& ctx, const ActionNode& node) {
    node.execute(ctx, *this);
}
}  // namespace action
}  // namespace mir

