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


#ifndef MIRComplexJob_H
#define MIRComplexJob_H

#include <string>
#include <vector>


#include "eckit/memory/NonCopyable.h"

namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace action {
class Job;
}
namespace api {

class MIRJob;

class MIRComplexJob : private eckit::NonCopyable {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    MIRComplexJob();

    // -- Destructor

    ~MIRComplexJob();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void execute() const;
    bool empty() const;


    void clear();

    MIRComplexJob &add(api::MIRJob *job, input::MIRInput &input, output::MIROutput &output);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members
    // None

    // -- Methods



    // -- Overridden methods


    // -- Class members
    // None

    // -- Class methods
    // None

  private:


    // -- Members

    std::vector<api::MIRJob *> apis_;
    std::vector<action::Job *> jobs_;
    std::vector<output::MIROutput *> outputs_;

    input::MIRInput* input_;

    // -- Methods


    // -- Overridden methods

    // From MIRParametrisation

    virtual void print(std::ostream &) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

};


}  // namespace api
}  // namespace mir
#endif

