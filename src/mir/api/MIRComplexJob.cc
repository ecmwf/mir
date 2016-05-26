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

#include "mir/api/MIRComplexJob.h"
#include "mir/action/Job.h"
#include "mir/output/MIROutput.h"
#include "mir/api/MIRJob.h"
#include "mir/action/ActionPlan.h"
#include "mir/action/Action.h"


namespace mir {
namespace api {


MIRComplexJob::MIRComplexJob() {
}


MIRComplexJob::~MIRComplexJob() {
    for (std::vector<action::Job *>::iterator j = jobs_.begin(); j != jobs_.end(); ++j) {
        delete (*j);
    }
    for (std::vector<api::MIRJob *>::iterator j = apis_.begin(); j != apis_.end(); ++j) {
        delete (*j);
    }
}

class Node {

    const action::Action &action_;
    std::vector<Node *> kids_;

  public:

    Node(const action::Action &action) : action_(action) {}

    const action::Action &action() const {
        return action_;
    }

    std::vector<Node *> &kids() {
        return kids_;
    }

    void dump(size_t depth) const {
        for (size_t i = 0; i < 3 * depth; i++) {
            std::cout << " ";
        }
        std::cout << action_ << std::endl;
        for (std::vector<Node *>::const_iterator j = kids_.begin(); j != kids_.end(); ++j) {
            (*j)->dump(depth+1);
        }
    }

};

// static void fill(size_t n) {

// }

void MIRComplexJob::execute() const {
    std::cout << "---------------------- " << std::endl;

    for (std::vector<action::Job *>::const_iterator j = jobs_.begin(); j != jobs_.end(); ++j) {
        const action::ActionPlan &plan = (*j)->plan();
        std::cout << std::endl;
        for (size_t i = 0; i < plan.size(); i++) {
            const action::Action &action = plan.action(i);
            std::cout << action << std::endl;
        }
    }

    std::cout << "---------------------- " << std::endl;

    std::vector<Node *> graph;

    for (std::vector<action::Job *>::const_iterator j = jobs_.begin(); j != jobs_.end(); ++j) {
        const action::ActionPlan &plan = (*j)->plan();

        std::vector<Node *> *nodes = &graph;

        size_t i = 0;
        while (i < plan.size()) {
            bool found = false;

            for (std::vector<Node *>::const_iterator k = nodes->begin(); k != nodes->end(); ++k) {
                if (plan.action(i).sameAs((*k)->action())) {
                    nodes = &(*k)->kids();
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
            Node* node = new Node(plan.action(i));
            nodes->push_back(node);
            nodes = &node->kids();
            i++;
        }
    }
    std::cout << ">>>>> " << std::endl;

    for (std::vector<Node *>::const_iterator j = graph.begin(); j != graph.end(); ++j) {
        (*j)->dump(0);
    }
        std::cout << "<<<<< " << std::endl;

}

void MIRComplexJob::print(std::ostream &out) const {
    out << "MIRComplexJob[]";
}

MIRComplexJob &MIRComplexJob::add(api::MIRJob *job, input::MIRInput &input, output::MIROutput &output) {
    apis_.push_back(job); // We keep it becase the Job needs a reference
    jobs_.push_back(new action::Job(*job, input, output));
    return *this;
}


}  // namespace api
}  // namespace mir

