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


#include "mir/method/Matrix.h"

#include <ostream>

#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Mutex.h"


namespace mir::method {


static const MethodBuilder<Matrix> __builder("matrix");


Matrix::Matrix(const param::MIRParametrisation& param) :
    MethodWeighted(param), path_([&param]() {
        if (std::string path; param.get("interpolation-matrix", path)) {
            return path;
        }
        throw exception::UserError("Matrix: option interpolation-matrix missing");
    }()) {
    if (!path_.exists()) {
        throw exception::UserError("Matrix: path does not exist '" + path_ + "'");
    }
}


void Matrix::json(eckit::JSON& j) const {
    j.startObject();
    j << "interpolation-matrix" << path_;
    MethodWeighted::json(j);
    j.endObject();
}


const char* Matrix::name() const {
    return "matrix";
}


void Matrix::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation&,
                      const repres::Representation&) const {
    static util::recursive_mutex MUTEX;
    util::lock_guard<util::recursive_mutex> lock(MUTEX);

    W.load(path_);  // statistics and sizes are checked at a higher level
}

void Matrix::hash(eckit::MD5& h) const {
    h.add(path_);
    MethodWeighted::hash(h);
}


bool Matrix::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const Matrix*>(&other);
    return (o != nullptr) && path_ == o->path_ && MethodWeighted::sameAs(other);
}


void Matrix::print(std::ostream& out) const {
    out << "Matrix["
        << "InterpolationMatrix=" << path_ << ",";
    MethodWeighted::print(out);
    out << "]";
}


}  // namespace mir::method
