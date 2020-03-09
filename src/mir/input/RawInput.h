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


#ifndef mir_input_RawInput_h
#define mir_input_RawInput_h

#include "mir/input/MIRInput.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace input {
class RawMetadata;
}
}  // namespace mir


namespace mir {
namespace input {


class RawInput : public MIRInput, public param::MIRParametrisation {
public:
    // -- Exceptions
    // None

    // -- Constructors
    RawInput(const RawMetadata& metadata, const double* values, size_t count);

    // -- Destructor

    ~RawInput();  // Change to virtual if base class

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

    const RawMetadata& metadata_;
    const double* values_;
    size_t count_;

    // -- Methods

    // -- Overridden methods

    // From MIRInput
    void print(std::ostream&) const;  // Change to virtual if base class
    const param::MIRParametrisation& parametrisation(size_t which) const;
    data::MIRField field() const;
    bool next();
    size_t copy(double* values, size_t size) const;
    bool sameAs(const MIRInput& other) const;

    // From MIRParametrisation
    bool has(const std::string& name) const;

    bool get(const std::string& name, std::string& value) const;
    bool get(const std::string& name, bool& value) const;
    bool get(const std::string& name, int& value) const;
    bool get(const std::string& name, long& value) const;
    bool get(const std::string& name, float& value) const;
    bool get(const std::string& name, double& value) const;

    bool get(const std::string& name, std::vector<int>& value) const;
    bool get(const std::string& name, std::vector<long>& value) const;
    bool get(const std::string& name, std::vector<float>& value) const;
    bool get(const std::string& name, std::vector<double>& value) const;
    bool get(const std::string& name, std::vector<std::string>& value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace input
}  // namespace mir


#endif
