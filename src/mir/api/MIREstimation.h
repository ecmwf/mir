/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date Aug 2019


#ifndef MIREstimation_H
#define MIREstimation_H



namespace mir {

namespace api {


class MIREstimation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    MIREstimation();

    // -- Destructor

    ~MIREstimation();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void startField();

    void numberOfGridPoints(size_t count);
    void accuracy(size_t count);
    void edition(size_t count);
    void packing(const std::string& packing);

    void endField();

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

    size_t totalNumberOfFields_;
    size_t totalNumberOfGridPoints_;
    size_t totalNumberOfBits_;

    size_t numberOfGridPoints_;
    size_t accuracy_;
        size_t edition_;

    size_t bitsPerValue_;
    std::string packing_;

    // -- Methods


    // -- Overridden methods

    // From MIRParametrisation

    void print(std::ostream &) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const MIREstimation &p) {
        p.print(s);
        return s;
    }
};


}  // namespace api
}  // namespace mir
#endif

