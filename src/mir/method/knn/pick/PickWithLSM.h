/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_knn_distance_PickWithLSM_h
#define mir_method_knn_distance_PickWithLSM_h

#include "mir/method/knn/pick/Pick.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace lsm {
class LandSeaMasks;
}
}


namespace mir {
namespace method {
namespace knn {
namespace pick {


struct PickWithLSM : Pick {
    PickWithLSM(const param::MIRParametrisation&, const lsm::LandSeaMasks& lsm);

protected:
    const std::vector<bool>& imask_;
    const std::vector<bool>& omask_;
    virtual bool sameAs(const Pick&) const;

private:
    virtual void hash(eckit::MD5&) const;
};


class PickWithLSMFactory {
private:
    std::string name_;
    virtual PickWithLSM* make(const param::MIRParametrisation&, const lsm::LandSeaMasks&) = 0;
protected:
    PickWithLSMFactory(const std::string& name);
    virtual ~PickWithLSMFactory();
public:
    static const PickWithLSM* build(const std::string& name, const param::MIRParametrisation&, const lsm::LandSeaMasks&);
    static void list(std::ostream&);
    static bool has(const std::string& name);
};


template<class T>
class PickWithLSMBuilder : public PickWithLSMFactory {
    virtual PickWithLSM* make(const param::MIRParametrisation& param, const lsm::LandSeaMasks& lsm) {
        return new T(param, lsm);
    }
public:
    PickWithLSMBuilder(const std::string& name) : PickWithLSMFactory(name) {}
};


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

