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

#include <memory>

#include "mir/input/MIRInput.h"
#include "mir/param/FieldParametrisation.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::input {


class ArtificialInput : public MIRInput, protected param::FieldParametrisation {
public:
    // -- Constructors

    ArtificialInput(const ArtificialInput&) = delete;
    ArtificialInput(ArtificialInput&&)      = delete;

    // -- Destructor

    ~ArtificialInput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void operator=(const ArtificialInput&) = delete;
    void operator=(ArtificialInput&&)      = delete;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors

    ArtificialInput();

    // -- Members
    // None

    // -- Methods

    param::SimpleParametrisation& parametrisation(size_t which = 0);

    // -- Overridden methods

    // From MIRInput
    bool next() override;
    size_t dimensions() const override;
    const param::MIRParametrisation& parametrisation(size_t which = 0) const override;
    void setAuxiliaryInformation(const util::ValueMap&) override;
    void print(std::ostream&) const override;
    bool sameAs(const MIRInput&) const override;

    // From FieldParametrisation
    bool get(const std::string& name, long& value) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    param::SimpleParametrisation parametrisation_;
    std::unique_ptr<param::MIRParametrisation> inputParametrisation_;
    size_t calls_;

    // For unstructured grids
    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods
    // None

    // -- Overridden methods

    // From FieldParametrisation
    void latitudes(std::vector<double>&) const override;
    void longitudes(std::vector<double>&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ArtificialInput& p) {
        p.print(s);
        return s;
    }
};


class ArtificialInputFactory {
    virtual ArtificialInput* make() = 0;
    const std::string name_;

protected:
    explicit ArtificialInputFactory(const std::string& name);
    virtual ~ArtificialInputFactory();

public:
    ArtificialInputFactory(const ArtificialInputFactory&) = delete;
    ArtificialInputFactory(ArtificialInputFactory&&)      = delete;
    void operator=(const ArtificialInputFactory&)         = delete;
    void operator=(ArtificialInputFactory&&)              = delete;

    static ArtificialInput* build(const std::string&);
    static void list(std::ostream&);
};


template <class T>
class ArtificialInputBuilder : public ArtificialInputFactory {
    ArtificialInput* make() override { return new T; }

public:
    explicit ArtificialInputBuilder(const std::string& name) : ArtificialInputFactory(name) {}
};


}  // namespace mir::input
