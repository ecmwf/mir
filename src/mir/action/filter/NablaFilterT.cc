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


#include "mir/action/filter/NablaFilterT.h"

#include <ostream>

#include "eckit/exception/Exceptions.h"

#include "mir/action/context/Context.h"
#include "mir/api/Atlas.h"
#include "mir/api/MIREstimation.h"
#include "mir/caching/InMemoryMeshCache.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {


struct NablaOperation {
    NablaOperation(atlas::Mesh& mesh) :
        fvm_(mesh, atlas::util::Config("radius", atlas::util::Earth::radius())),
        nabla_(fvm_),
        nodes_(fvm_.node_columns().nodes()),
        nodeIsGhost_(nodes_) {}

    virtual void operator()(data::MIRField&) const = 0;

protected:
    atlas::Field createField(size_t variables) const {
        ASSERT(variables >= 1);
        return fvm_.node_columns().createField<double>(atlas::option::variables(variables));
    }

    atlas::Field readField(const data::MIRField& data, size_t variables) const {
        ASSERT(data.dimensions() == variables);

        auto field = createField(variables);
        auto view  = atlas::array::make_view<double, 2>(field);

        // Set input field (copying is not great, but there you go)
        size_t d = 0;
        for (atlas::idx_t v = 0; v < field.variables(); ++v) {
            auto& values = data.values(d++);
            ASSERT(values.size() <= size_t(nodes_.size()));

            size_t m = 0;
            for (atlas::idx_t n = 0; n < nodes_.size(); ++n) {
                view(n, v) = nodeIsGhost_(n) ? 0. : values[m++];
            }
            ASSERT(values.size() == m);
        }

        field.haloExchange();
        return field;
    }

    void writeField(data::MIRField& data, const atlas::Field& field) const {
        ASSERT(field.variables() >= 1);
        data.dimensions(field.variables());

        auto points = data.values(0).size();
        auto view   = atlas::array::make_view<double, 2>(field);

        // Set results (copying is not great, but there you go)
        size_t d = 0;
        for (atlas::idx_t v = 0; v < field.variables(); ++v) {
            data::MIRValuesVector values;
            values.reserve(points);

            for (atlas::idx_t n = 0; n < nodes_.size(); ++n) {
                if (!nodeIsGhost_(n)) {
                    values.push_back(view(n, v));
                }
            }
            ASSERT(values.size() == points);

            data.update(values, d++);
        }
    }

    const atlas::numerics::Nabla& nabla() const { return nabla_; }

private:
    atlas::numerics::fvm::Method fvm_;
    atlas::numerics::Nabla nabla_;
    atlas::mesh::Nodes& nodes_;
    atlas::mesh::IsGhostNode nodeIsGhost_;
};


struct ScalarGradient : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "ScalarGradient"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 1);
        auto b = createField(2);

        nabla().gradient(a, b);
        writeField(field, b);
    }
};


struct ScalarLaplacian : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "ScalarLaplacian"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 1);
        auto b = createField(1);

        nabla().laplacian(a, b);
        writeField(field, b);
    }
};


struct UVGradient : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "UVGradient"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 2);
        auto b = createField(4);

        nabla().gradient(a, b);
        writeField(field, b);
    }
};


struct UVDivergence : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "UVDivergence"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 2);
        auto b = createField(1);

        nabla().divergence(a, b);
        writeField(field, b);
    }
};


struct UVCurl : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "UVCurl"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 2);
        auto b = createField(2);

        nabla().curl(a, b);
        writeField(field, b);
    }
};


template <typename T>
bool NablaFilterT<T>::sameAs(const Action& other) const {
    auto o = dynamic_cast<const NablaFilterT<T>*>(&other);
    return (o != nullptr) && meshGeneratorParams_.sameAs(o->meshGeneratorParams_);
}


template <typename T>
void NablaFilterT<T>::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().nablaTimer());

    // Generate mesh (disabling incompatible features)
    auto& field = ctx.field();
    if (field.hasMissing()) {
        throw eckit::UserError(std::string(name()) + ": missing values not supported");
    }

    auto params = meshGeneratorParams_;
    field.representation()->fill(params);
    params.set("3d", false).set("force_include_north_pole", false).set("force_include_south_pole", false);

    auto grid = field.representation()->atlasGrid();
    auto mesh = caching::InMemoryMeshCache::atlasMesh(ctx.statistics(), grid, params);

    // Perform operation
    T operation(mesh);
    operation(field);
}


template <typename T>
const char* NablaFilterT<T>::name() const {
    return T::name();
}


template <typename T>
void NablaFilterT<T>::estimate(context::Context&, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


static NablaFilterBuilder<NablaFilterT<ScalarGradient>> __nabla1("scalar-gradient");
static NablaFilterBuilder<NablaFilterT<ScalarLaplacian>> __nabla2("scalar-laplacian");
static NablaFilterBuilder<NablaFilterT<UVGradient>> __nabla3("uv-gradient");
static NablaFilterBuilder<NablaFilterT<UVDivergence>> __nabla4("uv-divergence");
static NablaFilterBuilder<NablaFilterT<UVCurl>> __nabla5("uv-curl");


}  // namespace action
}  // namespace mir
