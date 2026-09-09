// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/output/MIROutput.h"
#include "mir/util/Types.h"


namespace mir::data {
class MIRField;
}  // namespace mir::data


namespace mir::output {


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
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
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

protected:
    PNGEncoderFactory(const std::string&);
    virtual ~PNGEncoderFactory();

public:
    PNGEncoderFactory(const PNGEncoderFactory&) = delete;
    PNGEncoderFactory(PNGEncoderFactory&&)      = delete;
    void operator=(const PNGEncoderFactory&)    = delete;
    void operator=(PNGEncoderFactory&&)         = delete;

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


}  // namespace mir::output
