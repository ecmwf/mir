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

#include <string>

#include "eckit/config/Configured.h"

#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace input {
class MIRInput;
}
namespace output {
class MIROutput;
}
namespace util {
class MIRStatistics;
}
}  // namespace mir


namespace mir::api {

class MIREstimation;

class MIRJob : public param::SimpleParametrisation, public eckit::Configured {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MIRJob();

    // -- Destructor

    ~MIRJob() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void execute(input::MIRInput&, output::MIROutput&) const;
    void execute(input::MIRInput&, output::MIROutput&, util::MIRStatistics&) const;

    void estimate(input::MIRInput&, output::MIROutput&, MIREstimation&) const;

    MIRJob& set(const std::string& name, const std::string& value) override;
    MIRJob& set(const std::string& name, const char* value) override;
    MIRJob& set(const std::string& name, float value) override;
    MIRJob& set(const std::string& name, double value) override;
    MIRJob& set(const std::string& name, int value) override;
    MIRJob& set(const std::string& name, long value) override;
    MIRJob& set(const std::string& name, long long value);
    MIRJob& set(const std::string& name, bool value) override;
    MIRJob& set(const std::string& name, size_t value) override;

    MIRJob& set(const std::string& name, const std::vector<int>& value) override;
    MIRJob& set(const std::string& name, const std::vector<long>& value) override;
    MIRJob& set(const std::string& name, const std::vector<long long>& value);
    MIRJob& set(const std::string& name, const std::vector<size_t>& value) override;
    MIRJob& set(const std::string& name, const std::vector<float>& value) override;
    MIRJob& set(const std::string& name, const std::vector<double>& value) override;
    MIRJob& set(const std::string& name, const std::vector<std::string>& value) override;

    MIRJob& set(const std::string& name, double v1, double v2);
    MIRJob& set(const std::string& name, double v1, double v2, double v3, double v4);

    MIRJob& clear(const std::string& name) override;

    MIRJob& set(const std::string& args);

    MIRJob& representationFrom(const input::MIRInput&);

    // For debugging only
    void mirToolCall(std::ostream&) const;

    void json(eckit::JSON&) const;

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
    // None

    // -- Methods

    template <class T>
    MIRJob& _setScalar(const std::string& name, const T& value);

    template <class T>
    MIRJob& _setVector(const std::string& name, const T& value, size_t outputCount = 4);

    // -- Overridden methods

    // From SimpleParametrisation
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::api
