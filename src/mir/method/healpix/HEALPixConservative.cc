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


#include "mir/method/healpix/HEALPixConservative.h"

#include <numeric>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/proxy/HEALPix.h"
#include "mir/repres/unsupported/HEALPixNested.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::method::healpix {


namespace {


struct healpix_t {
    const size_t Nside;
    const repres::proxy::HEALPix::Ordering ordering;
    size_t size() const { return 12 * Nside * Nside; }

    static healpix_t make(const repres::Representation& rep) {
        try {
            auto& h = dynamic_cast<const repres::proxy::HEALPix&>(rep);
            return {h.Nside(), h.ordering()};
        }
        catch (const std::bad_cast&) {
        }

        try {
            auto& h = dynamic_cast<const repres::unsupported::HEALPixNested&>(rep);
            return {h.Nside(), h.ordering()};
        }
        catch (const std::bad_cast&) {
        }

        throw exception::UserError("HEALPixConservative: supports only HEALPix ring or nested representations");
    }
};


struct matrix_t : protected std::vector<double> {
    matrix_t(size_t rows, size_t cols) : vector(rows * cols), rows_(rows), cols_(cols) { ASSERT(!empty()); }

    const value_type& operator()(size_t r, size_t c) const {
        ASSERT(r < rows() && c < cols());
        return at(r * cols() + c);
    }

    value_type& operator()(size_t r, size_t c) {
        ASSERT(r < rows() && c < cols());
        return at(r * cols() + c);
    }

    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

private:
    size_t rows_;
    size_t cols_;
};


}  // namespace


bool HEALPixConservative::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const HEALPixConservative*>(&other);
    return (o != nullptr) && name() == o->name() && MethodWeighted::sameAs(*o);
}


void HEALPixConservative::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                                   const repres::Representation& out) const {
    auto& log = Log::info();  // Log::debug();
    log << "HEALPixConservative::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    // Pixel 1D

    auto Hin  = healpix_t::make(in);
    auto Hout = healpix_t::make(out);
    ASSERT(Hin.size() == in.numberOfPoints());
    ASSERT(Hout.size() == out.numberOfPoints());

    // TODO

    using f = std::pair<size_t, size_t>;
    f inc{std::gcd(Hin.Nside, Hout.Nside), std::lcm(Hin.Nside, Hout.Nside)};


    // Pixel 2D

    // matrix_t pixel()

    // TODO


    // 12 Pixels + reordering
    WeightMatrix M(Hout.size(), Hin.size());
    W.swap(M);
}


void HEALPixConservative::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    std::ostringstream str;
    print(str);
    md5.add(str.str());
}


void HEALPixConservative::json(eckit::JSON& j) const {
    j.startObject();
    MethodWeighted::json(j);
    j.endObject();
}


void HEALPixConservative::print(std::ostream& out) const {
    out << "HEALPixConservative[";
    MethodWeighted::print(out);
    out << "]";
}


bool HEALPixConservative::validateMatrixWeights() const {
    return false;
}


const char* HEALPixConservative::name() const {
    return "healpix-conservative";
}


static const MethodBuilder<HEALPixConservative> __builder("healpix-conservative");


}  // namespace mir::method::healpix
