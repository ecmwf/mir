/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_fe_Conservative_h
#define mir_method_fe_Conservative_h

#include <memory>

#include "mir/method/MethodWeighted.h"


namespace eckit {
namespace linalg {
class Vector;
}  // namespace linalg
}  // namespace eckit

namespace mir {
namespace method {
namespace fe {
class FiniteElement;
}  // namespace fe
}  // namespace method
}  // namespace mir


namespace mir {
namespace method {
namespace fe {


class Conservative : public MethodWeighted {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    Conservative(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Conservative();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

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

    std::unique_ptr<FiniteElement> inputMethod_;
    std::unique_ptr<FiniteElement> outputMethod_;

    // -- Methods
    void computeLumpedMassMatrix(eckit::linalg::Vector&, const atlas::Mesh&) const;

    // -- Overridden methods

    // From MethodWeighted
    void hash(eckit::MD5&) const;
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const;
    bool sameAs(const Method&) const;
    void print(std::ostream&) const;
    virtual const char* name() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace fe
}  // namespace method
}  // namespace mir


#endif
