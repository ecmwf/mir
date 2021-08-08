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

#include "mir/output/MIROutput.h"
#include "mir/util/Types.h"


namespace mir {
namespace data {
class MIRField;
}
}  // namespace mir


namespace mir {
namespace output {


class PNGOutput : public MIROutput {
public:
    // -- Types

    struct PNGEncoder;

    // -- Exceptions
    // None

    // -- Constructors

    PNGOutput(std::string path);

    // -- Destructor

    ~PNGOutput() override;

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

    const std::string path_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    size_t copy(const param::MIRParametrisation&, context::Context&) override;
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void prepare(const param::MIRParametrisation&, action::ActionPlan&, output::MIROutput&) override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


class PNGEncoderFactory {
    std::string name_;
    virtual PNGOutput::PNGEncoder* make(const param::MIRParametrisation&, const data::MIRField&) = 0;

    PNGEncoderFactory(const PNGEncoderFactory&) = delete;
    PNGEncoderFactory& operator=(const PNGEncoderFactory&) = delete;

protected:
    PNGEncoderFactory(const std::string&);
    virtual ~PNGEncoderFactory();

public:
    // This is 'const' as the representation uses reference counting
    // Represention should always be immutable
    static const PNGOutput::PNGEncoder* build(const param::MIRParametrisation&, const data::MIRField&);
    static void list(std::ostream&);
};


template <class T>
class PNGEncoderBuilder : public PNGEncoderFactory {
    PNGOutput::PNGEncoder* make(const param::MIRParametrisation& param, const data::MIRField& field) override {
        return new T(param, field);
    }

public:
    PNGEncoderBuilder(const std::string& name) : PNGEncoderFactory(name) {}
};


}  // namespace output
}  // namespace mir
