// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <vector>


namespace eckit {
class MD5;
}
namespace mir::lsm {
class Mask;
}  // namespace mir::lsm
namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}  // namespace mir


namespace mir::lsm {


class LandSeaMasks {
public:
    // -- Exceptions
    // None

    // -- Constructors

    LandSeaMasks(const Mask& input, const Mask& output);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void hash(eckit::MD5&) const;
    std::string cacheName() const;

    bool active() const;
    bool cacheable() const;

    const std::vector<bool>& inputMask() const;
    const std::vector<bool>& outputMask() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static LandSeaMasks lookup(const param::MIRParametrisation&, const repres::Representation& in,
                               const repres::Representation& out);

    static bool sameLandSeaMasks(const param::MIRParametrisation&, const param::MIRParametrisation&);

private:
    // -- Members

    const Mask& input_;
    const Mask& output_;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const LandSeaMasks& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::lsm
