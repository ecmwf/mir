// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/Spectral.h"
#include "mir/util/Domain.h"


namespace mir::repres::sh {


class SphericalHarmonics : public Spectral {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SphericalHarmonics(const param::MIRParametrisation&);
    SphericalHarmonics(size_t truncation);

    // -- Destructor

    ~SphericalHarmonics() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static void truncate(size_t truncation_from, size_t truncation_to, const MIRValuesVector& in, MIRValuesVector& out);

    static size_t number_of_complex_coefficients(size_t truncation) { return (truncation + 1) * (truncation + 2) / 2; }

    static void interlace_spectra(MIRValuesVector& interlaced, const MIRValuesVector& spectra, size_t truncation,
                                  size_t numberOfComplexCoefficients, size_t index, size_t indexTotal);

    // -- Overridden methods

    util::Domain domain() const override { return util::Domain(); }

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    size_t truncation_;

    // -- Methods
    // None

    // -- Overridden methods

    void fillGrib(grib_info&) const override;
    void fillSpec(CustomSpec&) const override;
    std::string factory() const override;

    const Representation* truncate(size_t truncation, const MIRValuesVector&, MIRValuesVector&) const override;
    size_t truncation() const override;

    void comparison(std::string&) const override;

    void validate(const MIRValuesVector&) const override;
    size_t numberOfValues() const override;

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;

    bool isPeriodicWestEast() const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres::sh
