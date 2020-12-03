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


#ifndef mir_key_style_CustomParametrisation_h
#define mir_key_style_CustomParametrisation_h

#include <map>
#include <vector>

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace key {
namespace style {


class CustomParametrisation : public param::MIRParametrisation {
public:
    // -- Exceptions
    // None

    // -- Constructors

    CustomParametrisation(const std::string& name, const std::map<std::string, std::vector<std::string> >& params,
                          const param::MIRParametrisation& parametrisation);

    // -- Destructor

    virtual ~CustomParametrisation() override;  // Change to virtual if base class

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

    template <class T>
    bool _get(const std::string& name, T& value) const;

    // -- Overridden methods

    // From MIRParametrisation
    virtual const MIRParametrisation& userParametrisation() const override;
    virtual const MIRParametrisation& fieldParametrisation() const override;

    virtual bool has(const std::string& name) const override;

    virtual bool get(const std::string& name, std::string& value) const override;
    virtual bool get(const std::string& name, bool& value) const override;
    virtual bool get(const std::string& name, int& value) const override;
    virtual bool get(const std::string& name, long& value) const override;
    virtual bool get(const std::string& name, float& value) const override;
    virtual bool get(const std::string& name, double& value) const override;

    virtual bool get(const std::string& name, std::vector<int>& value) const override;
    virtual bool get(const std::string& name, std::vector<long>& value) const override;
    virtual bool get(const std::string& name, std::vector<float>& value) const override;
    virtual bool get(const std::string& name, std::vector<double>& value) const override;
    virtual bool get(const std::string& name, std::vector<std::string>& value) const override;

    virtual void print(std::ostream&) const override;  // Change to virtual if base class

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string name_;
    std::map<std::string, std::vector<std::string> > params_;
    const param::MIRParametrisation& parametrisation_;

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


}  // namespace style
}  // namespace key
}  // namespace mir


#endif
