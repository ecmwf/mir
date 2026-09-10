// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cmath>

#include "mir/util/Exceptions.h"
#include "mir/util/SpectralOrder.h"


namespace mir::util {


template <int ORDER>
class SpectralOrderT : public SpectralOrder {
public:
    // -- Exceptions
    // None

    // -- Constructors
    SpectralOrderT() { ASSERT(ORDER > 0); }

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    long getTruncationFromGaussianNumber(long N) const override {
        ASSERT(N > 0);

        const auto T = static_cast<long>(std::ceil(4. / static_cast<double>(ORDER + 1) * static_cast<double>(N)) - 1);
        ASSERT(T > 0);

        return T;
    }

    long getGaussianNumberFromTruncation(long T) const override {
        ASSERT(T > 0);

        const auto N = static_cast<long>(static_cast<double>(T + 1) * static_cast<double>(ORDER + 1) / 4.);
        ASSERT(N > 0);

        return N;
    }

    void print(std::ostream& out) const override { out << "SpectralOrderT<ORDER=" << ORDER << ">[]"; }

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::util
