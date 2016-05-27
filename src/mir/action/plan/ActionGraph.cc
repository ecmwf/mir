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

#include <iostream>

#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/ActionNode.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/action/plan/Action.h"


namespace mir {
namespace action {


ActionGraph::ActionGraph() {
}


ActionGraph::~ActionGraph() {
    for (std::vector<ActionNode *>::const_iterator j = nodes_.begin(); j != nodes_.end(); ++j) {
        delete (*j);
    }
}

void ActionGraph::add(const ActionPlan& plan) {
    action::ActionGraph *current = this;

    size_t i = 0;
    while (i < plan.size()) {
        bool found = false;

        for (std::vector<action::ActionNode *>::const_iterator k = current->nodes_.begin(); k != current->nodes_.end(); ++k) {
            if (plan.action(i).sameAs((*k)->action())) {
                current = &(*k)->graph();
                found = true;
                break;
            }
        }

        if (!found) {
            break;
        }
        i++;
    }

    while (i < plan.size()) {
        action::ActionNode* node = current->add(plan.action(i));
        current = &node->graph();
        i++;
    }
}

ActionNode* ActionGraph::add(const Action& action) {
    nodes_.push_back(new ActionNode(action));
    return nodes_.back();
}


void ActionGraph::dump(std::ostream& out, size_t depth) const {
    for (std::vector<ActionNode *>::const_iterator j = nodes_.begin(); j != nodes_.end(); ++j) {
        (*j)->dump(out, depth);
    }
}


void ActionGraph::print(std::ostream &out) const {
    out << "ActionGraph[]";
}

}  // namespace action
}  // namespace mir

