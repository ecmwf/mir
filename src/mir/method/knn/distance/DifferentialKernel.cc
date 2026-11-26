// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/*
 * Signed (zero-sum) kernels, for differential operators and edge detection, on the local tangent plane:
 *
 * - derivative:  directional derivative along "kernel-angle" (degrees, counter-clockwise from east: 0 is d/dx east, 90
 *                is d/dy north), in field units per metre; exact for linear fields (a Sobel operator generalised to
 *                any grid)
 * - laplacian:   Laplacian (d2/dx2 + d2/dy2), in field units per square metre; exact for quadratic fields
 * - difference-of-gaussians: gaussian of radius "kernel-radius" minus gaussian of radius "kernel-radius-outer"
 *                (default 1.6 times larger), both normalised (a band-pass, for edge detection)
 *
 * The derivative and laplacian weights come from a weighted least-squares fit of a polynomial (linear or quadratic)
 * to the neighbours, weighted by the radial kernel "kernel" (default gaussian; see KernelProfile) of radius
 * "kernel-radius" (default "distance"). The neighbour search should include a few kernel radii (compact kernels: the
 * kernel radius).
 *
 * The weights add up to zero, so they are not validated as interpolation weights, and not pruned. Missing values
 * should be handled with non-linear treatment "missing-if-any-missing" (others re-normalise weights to add up to one).
 */


#include <array>
#include <cmath>
#include <ostream>
#include <sstream>
#include <string>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/Kernel.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/CompensatedSum.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::distance {


namespace {


std::string profile_name(const param::MIRParametrisation& param) {
    std::string name = "gaussian";
    param.get("kernel", name);
    return name;
}


/// Solve A x = b (A symmetric positive semi-definite, N x N) by Gaussian elimination with partial pivoting
template <size_t N>
bool solve(std::array<std::array<double, N>, N> A, std::array<double, N>& b) {
    double scale = 0.;
    for (size_t i = 0; i < N; ++i) {
        scale = std::max(scale, std::abs(A[i][i]));
    }
    if (!(scale > 0.)) {
        return false;
    }

    for (size_t c = 0; c < N; ++c) {
        size_t p = c;
        for (size_t r = c + 1; r < N; ++r) {
            if (std::abs(A[r][c]) > std::abs(A[p][c])) {
                p = r;
            }
        }
        if (std::abs(A[p][c]) < 1e-12 * scale) {
            return false;  // singular: not enough (or degenerate) neighbours
        }
        std::swap(A[p], A[c]);
        std::swap(b[p], b[c]);

        for (size_t r = c + 1; r < N; ++r) {
            const auto f = A[r][c] / A[c][c];
            for (size_t k = c; k < N; ++k) {
                A[r][k] -= f * A[c][k];
            }
            b[r] -= f * b[c];
        }
    }

    for (size_t c = N; c-- > 0;) {
        for (size_t k = c + 1; k < N; ++k) {
            b[c] -= A[c][k] * b[k];
        }
        b[c] /= A[c][c];
    }
    return true;
}


/**
 * @brief Weighted least-squares differential operator: w_j = K_j b(x_j) . z / h^order, with M z = e, M = sum_j K_j
 * b(x_j) b(x_j)^T, b a polynomial basis of the offsets scaled by the kernel radius (x = (dx, dy) / h), and e the
 * operator coefficients (on the basis)
 */
template <size_t N>
class DifferentialKernel : public DistanceWeighting {
public:
    using vector_t = std::array<double, N>;

    DifferentialKernel(const param::MIRParametrisation& param, int order) :
        profile_(profile_name(param)), radius_(kernel_radius(param)), order_(order) {}

    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override {
        ASSERT(!neighbours.empty());

        const TangentPlane plane(point);

        // kernel weights and basis at each neighbour, and the normal equations matrix
        std::vector<double> K;
        std::vector<vector_t> B;
        K.reserve(neighbours.size());
        B.reserve(neighbours.size());

        std::array<vector_t, N> M{};
        for (const auto& n : neighbours) {
            const auto k        = profile_.at_distance2(Point3::distance2(point, n.point()), radius_);
            const auto [dx, dy] = plane(n.point());
            const auto b        = basis(dx / radius_, dy / radius_);

            for (size_t r = 0; r < N; ++r) {
                for (size_t c = 0; c < N; ++c) {
                    M[r][c] += k * b[r] * b[c];
                }
            }

            K.push_back(k);
            B.push_back(b);
        }

        auto z = coefficients();
        if (!solve(M, z)) {
            throw exception::UserError(name() + ": not enough neighbours around point " + std::to_string(ip) +
                                       " (increase the search distance or the kernel radius)");
        }

        const auto scale = std::pow(radius_, order_);

        triplets.clear();
        triplets.reserve(neighbours.size());
        for (size_t j = 0; j < neighbours.size(); ++j) {
            double w = 0.;
            for (size_t r = 0; r < N; ++r) {
                w += B[j][r] * z[r];
            }
            triplets.emplace_back(ip, neighbours[j].payload(), K[j] * w / scale);
        }
    }

    WeightMatrix::Check validateMatrixWeights() const override { return {true, false, false}; }
    bool pruneMatrixWeights() const override { return false; }

protected:
    virtual const std::string& name() const                   = 0;
    virtual vector_t basis(double x, double y) const          = 0;
    virtual vector_t coefficients() const                     = 0;
    virtual void json_extra(eckit::JSON&) const               = 0;
    virtual void print_extra(std::ostream&) const             = 0;
    virtual bool sameAs_extra(const DistanceWeighting&) const = 0;

private:
    bool sameAs(const DistanceWeighting& other) const override {
        const auto* o = dynamic_cast<const DifferentialKernel*>(&other);
        return (o != nullptr) && name() == o->name() && profile_.sameAs(o->profile_) &&
               eckit::types::is_approximately_equal(radius_, o->radius_) && sameAs_extra(other);
    }

    void json(eckit::JSON& j) const override {
        j << type() << name();
        j << "kernel" << profile_.name();
        j << "kernel-radius" << radius_;
        json_extra(j);
    }

    void print(std::ostream& out) const override {
        out << "DifferentialKernel[name=" << name() << ",profile=";
        profile_.print(out);
        out << ",radius=" << radius_;
        print_extra(out);
        out << "]";
    }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }

    KernelProfile profile_;
    double radius_;
    int order_;
};


/// Directional derivative, from a linear fit (1, x, y)
class Derivative final : public DifferentialKernel<3> {
public:
    explicit Derivative(const param::MIRParametrisation& param) : DifferentialKernel(param, 1) {
        param.get("kernel-angle", angle_ = 0.);
    }

private:
    const std::string& name() const override {
        static const std::string NAME{"derivative"};
        return NAME;
    }

    vector_t basis(double x, double y) const override { return {1., x, y}; }

    vector_t coefficients() const override {
        const auto a = angle_ * M_PI / 180.;
        return {0., std::cos(a), std::sin(a)};
    }

    void json_extra(eckit::JSON& j) const override { j << "kernel-angle" << angle_; }
    void print_extra(std::ostream& out) const override { out << ",angle=" << angle_; }

    bool sameAs_extra(const DistanceWeighting& other) const override {
        const auto* o = dynamic_cast<const Derivative*>(&other);
        return (o != nullptr) && eckit::types::is_approximately_equal(angle_, o->angle_);
    }

    double angle_;
};


/// Laplacian, from a quadratic fit (1, x, y, x^2, xy, y^2): 2 (a_xx + a_yy)
class Laplacian final : public DifferentialKernel<6> {
public:
    explicit Laplacian(const param::MIRParametrisation& param) : DifferentialKernel(param, 2) {}

private:
    const std::string& name() const override {
        static const std::string NAME{"laplacian"};
        return NAME;
    }

    vector_t basis(double x, double y) const override { return {1., x, y, x * x, x * y, y * y}; }
    vector_t coefficients() const override { return {0., 0., 0., 2., 0., 2.}; }

    void json_extra(eckit::JSON&) const override {}
    void print_extra(std::ostream&) const override {}
    bool sameAs_extra(const DistanceWeighting&) const override { return true; }
};


/// Difference of (normalised) gaussians, inner minus outer
class DifferenceOfGaussians final : public DistanceWeighting {
public:
    explicit DifferenceOfGaussians(const param::MIRParametrisation& param) :
        inner_(kernel_radius(param)), outer_(1.6 * inner_) {
        param.get("kernel-radius-outer", outer_);
        if (!(outer_ > inner_)) {
            throw exception::UserError(
                "DifferenceOfGaussians: 'kernel-radius-outer' should be larger than 'kernel-radius'");
        }
    }

    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override {
        ASSERT(!neighbours.empty());

        std::vector<double> a;
        std::vector<double> b;
        a.reserve(neighbours.size());
        b.reserve(neighbours.size());

        util::CompensatedSum<double> sa;
        util::CompensatedSum<double> sb;
        for (const auto& n : neighbours) {
            const auto d2 = Point3::distance2(point, n.point());
            sa += a.emplace_back(std::exp(d2 * (-1. / (2. * inner_ * inner_))));
            sb += b.emplace_back(std::exp(d2 * (-1. / (2. * outer_ * outer_))));
        }
        const auto na = sa.value();
        const auto nb = sb.value();
        ASSERT(na > 0. && nb > 0.);

        triplets.clear();
        triplets.reserve(neighbours.size());
        for (size_t j = 0; j < neighbours.size(); ++j) {
            triplets.emplace_back(ip, neighbours[j].payload(), a[j] / na - b[j] / nb);
        }
    }

    WeightMatrix::Check validateMatrixWeights() const override { return {true, false, false}; }
    bool pruneMatrixWeights() const override { return false; }

private:
    bool sameAs(const DistanceWeighting& other) const override {
        using eckit::types::is_approximately_equal;
        const auto* o = dynamic_cast<const DifferenceOfGaussians*>(&other);
        return (o != nullptr) && is_approximately_equal(inner_, o->inner_) && is_approximately_equal(outer_, o->outer_);
    }

    void json(eckit::JSON& j) const override {
        j << type() << "difference-of-gaussians";
        j << "kernel-radius" << inner_;
        j << "kernel-radius-outer" << outer_;
    }

    void print(std::ostream& out) const override {
        out << "DifferenceOfGaussians[radius=" << inner_ << ",radiusOuter=" << outer_ << "]";
    }

    void hash(eckit::MD5& h) const override {
        std::ostringstream s;
        s << *this;
        h.add(s.str());
    }

    double inner_;
    double outer_;
};


const DistanceWeightingBuilder<Derivative> __distance1("derivative");
const DistanceWeightingBuilder<Laplacian> __distance2("laplacian");
const DistanceWeightingBuilder<DifferenceOfGaussians> __distance3("difference-of-gaussians");


}  // namespace


}  // namespace mir::method::knn::distance
