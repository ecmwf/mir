// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/plan/SimpleExecutor.h"

#include <ostream>

#include "mir/action/plan/ActionNode.h"


namespace mir::action {


SimpleExecutor::SimpleExecutor(const std::string& name) : Executor(name) {}


void SimpleExecutor::print(std::ostream& out) const {
    out << "SimpleExecutor[]";
}


void SimpleExecutor::wait() const {}


void SimpleExecutor::execute(context::Context& ctx, const ActionNode& node) const {
    node.execute(ctx, *this);
}


void SimpleExecutor::parametrisation(const param::MIRParametrisation& /*unused*/) {}


static const SimpleExecutor executor("simple");


}  // namespace mir::action
