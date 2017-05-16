/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date Mar 2017


#ifndef mir_action_transform_mapping_Mapping_h
#define mir_action_transform_mapping_Mapping_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace action {
namespace transform {
namespace mapping {


class Mapping : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors
    Mapping() {}

    // -- Destructor
    virtual ~Mapping() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    virtual long getTruncationFromPointsPerLatitude(const long&) const;
    virtual long getPointsPerLatitudeFromTruncation(const long&) const;
    virtual void print(std::ostream&) const = 0;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Mapping& p) {
        p.print(s);
        return s;
    }
};


class MappingFactory {
    std::string name_;
    virtual Mapping *make() = 0;
protected:
    MappingFactory(const std::string&);
    virtual ~MappingFactory();
public:
    static Mapping *build(const std::string&);
    static void list(std::ostream&);
};


template <class T> class MappingBuilder : public MappingFactory {
    virtual Mapping *make() {
        return new T();
    }
public:
    MappingBuilder(const std::string& name) : MappingFactory(name) {}
};


} // namespace mapping
} // namespace transform
} // namespace action
} // namespace mir


#endif
