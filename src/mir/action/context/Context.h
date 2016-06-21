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


#ifndef Context_H
#define Context_H

#include <string>

#include "eckit/memory/ScopedPtr.h"
#include "eckit/memory/NonCopyable.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/action/context/Context.h"

namespace mir {
namespace input {
class MIRInput;
}

namespace util {
class MIRStatistics;
}

namespace data {
class MIRField;
}

namespace context {


class Context : private eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Context(Context*);

    Context(input::MIRInput &input,
            util::MIRStatistics& statistics);

    Context(data::MIRField &field,
            util::MIRStatistics& statistics);
    // -- Destructor

    ~Context();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    util::MIRStatistics& statistics();
    data::MIRField& field();
    input::MIRInput& input();

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

    input::MIRInput &input_;
    util::MIRStatistics& statistics_;

    eckit::ScopedPtr<data::MIRField> field_;

    // -- Methods


    // -- Overridden methods

    // From MIRParametrisation


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

};

}  // namespace action
}  // namespace mir
#endif

