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
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/action/plan/SimpleExecutor.h"
#include "mir/action/plan/Action.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/api/MIRWatcher.h"
#include "mir/action/plan/ActionNode.h"

#include "eckit/exception/Exceptions.h"



namespace mir {
namespace action {


SimpleExecutor::SimpleExecutor(const std::string& name):
    Executor(name) {
}


SimpleExecutor::~SimpleExecutor() = default;


void SimpleExecutor::print(std::ostream& out) const {
    out << "SimpleExecutor[]";
}


void SimpleExecutor::wait() const {

}

void SimpleExecutor::execute(context::Context& ctx, const ActionNode& node) const {
    node.execute(ctx, *this);
}

void SimpleExecutor::parametrisation(const param::MIRParametrisation &parametrisation) {

}



namespace {
static SimpleExecutor executor("simple");
}

}  // namespace action
}  // namespace mir

