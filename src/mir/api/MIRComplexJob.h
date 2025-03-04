/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include <iosfwd>
#include <vector>


namespace mir {
namespace action {
class Job;
}
namespace api {
class MIRJob;
class MIRWatcher;
}  // namespace api
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace util {
class MIRStatistics;
}


namespace api {


class MIRComplexJob {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MIRComplexJob();

    MIRComplexJob(const MIRComplexJob&) = delete;
    MIRComplexJob(MIRComplexJob&&)      = delete;

    // -- Destructor

    virtual ~MIRComplexJob();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const MIRComplexJob&) = delete;
    void operator=(MIRComplexJob&&)      = delete;

    // -- Methods

    void execute(util::MIRStatistics&) const;
    bool empty() const;
    void clear();

    MIRComplexJob& add(MIRJob*, input::MIRInput&, output::MIROutput&, MIRWatcher* = nullptr);

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::vector<MIRJob*> apis_;
    std::vector<action::Job*> jobs_;
    std::vector<output::MIROutput*> outputs_;
    std::vector<MIRWatcher*> watchers_;

    input::MIRInput* input_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIRParametrisation

    virtual void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace api
}  // namespace mir
