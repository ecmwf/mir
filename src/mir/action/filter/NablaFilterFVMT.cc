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


#include "mir/action/filter/NablaFilterFVMT.h"

#include <limits>
#include <memory>
#include <ostream>

#include "eckit/types/FloatCompare.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIREstimation.h"
#include "mir/caching/InMemoryMeshCache.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Atlas.h"
#include "mir/util/Earth.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Types.h"


namespace mir::action {


struct NablaOperation {
    NablaOperation(atlas::Mesh& mesh) :
        fvm_(mesh, atlas::util::Config("radius", util::Earth::radius())),
        nabla_(fvm_),
        nodes_(fvm_.node_columns().nodes()),
        nodeIsGhost_(nodes_) {}

    virtual ~NablaOperation() = default;

    NablaOperation(const NablaOperation&)            = delete;
    NablaOperation(NablaOperation&&)                 = delete;
    NablaOperation& operator=(const NablaOperation&) = delete;
    NablaOperation& operator=(NablaOperation&&)      = delete;

    virtual void operator()(data::MIRField&) const = 0;

protected:
    atlas::Field createField(size_t variables) const {
        ASSERT(variables > 0);
        return variables == 1 ? fvm_.node_columns().createField<double>()
                              : fvm_.node_columns().createField<double>(atlas::option::variables(variables));
    }

    atlas::Field readField(const data::MIRField& data, size_t variables) const {
        ASSERT(variables > 0);
        ASSERT(variables == data.dimensions());

        auto field = createField(variables);

        using atlas::array::Range;
        auto view = variables == 1 ? atlas::array::make_view<double, 1>(field).slice(Range::all(), Range::dummy())
                                   : atlas::array::make_view<double, 2>(field).slice(Range::all(), Range::all());

        // Copy input field (not great, but there you go)
        for (atlas::idx_t v = 0; v < atlas::idx_t(variables); ++v) {
            const auto& values = data.values(size_t(v));
            ASSERT(values.size() <= size_t(nodes_.size()));

            size_t m = 0;
            for (atlas::idx_t n = 0; n < nodes_.size(); ++n) {
                view(n, v) = nodeIsGhost_(n) ? 0. : values[m++];
            }
            ASSERT(values.size() == m);
        }

        field.set_dirty();
        field.haloExchange();
        return field;
    }

    void writeField(data::MIRField& data, const atlas::Field& field) const {
        ASSERT(data.dimensions() > 0);

        auto points    = data.values(0).size();
        auto variables = field.variables() > 0 ? field.variables() : 1;
        ASSERT(variables > 0);

        data.dimensions(size_t(variables));

        using atlas::array::Range;
        const auto view = variables == 1 ? atlas::array::make_view<double, 1>(field).slice(Range::all(), Range::dummy())
                                         : atlas::array::make_view<double, 2>(field).slice(Range::all(), Range::all());

        // Copy results (not great, but there you go)
        for (atlas::idx_t v = 0; v < variables; ++v) {
            MIRValuesVector values;
            values.reserve(points);

            for (atlas::idx_t n = 0; n < nodes_.size(); ++n) {
                if (!nodeIsGhost_(n)) {
                    values.push_back(view(n, v));
                }
            }
            ASSERT(values.size() == points);

            data.update(values, size_t(v));
        }
    }

    const atlas::numerics::Nabla& nabla() const { return nabla_; }

private:
    atlas::numerics::fvm::Method fvm_;
    atlas::numerics::Nabla nabla_;
    atlas::mesh::Nodes& nodes_;
    atlas::mesh::IsGhostNode nodeIsGhost_;
};


struct ScalarGradient final : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "ScalarGradient"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 1);
        auto b = createField(2);

        nabla().gradient(a, b);
        writeField(field, b);

        field.handle(0, 0);
        field.handle(1, 0);
    }
};


struct ScalarLaplacian final : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "ScalarLaplacian"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 1);
        auto b = createField(1);

        nabla().laplacian(a, b);
        writeField(field, b);
    }
};


struct UVGradient final : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "UVGradient"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 2);
        auto b = createField(4);

        nabla().gradient(a, b);
        writeField(field, b);

        field.handle(0, 0);
        field.handle(1, 0);
        field.handle(2, 1);
        field.handle(3, 1);
    }
};


struct UVDivergence final : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "UVDivergence"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 2);
        auto b = createField(1);

        nabla().divergence(a, b);
        writeField(field, b);
    }
};


struct UVVorticity final : NablaOperation {
    using NablaOperation::NablaOperation;
    static const char* name() { return "UVVorticity"; }
    void operator()(data::MIRField& field) const override {
        auto a = readField(field, 2);
        auto b = createField(1);

        nabla().curl(a, b);
        writeField(field, b);
    }
};


template <typename T>
bool NablaFilterFVMT<T>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const NablaFilterFVMT<T>*>(&other);
    return (o != nullptr) && meshGeneratorParams_.sameAs(o->meshGeneratorParams_);
}


template <typename T>
void NablaFilterFVMT<T>::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().nablaTimer());

    // Generate mesh (disabling incompatible features)
    auto& field = ctx.field();
    if (field.hasMissing()) {
        throw exception::UserError(std::string(name()) + ": missing values not supported");
    }

    auto params = meshGeneratorParams_;
    field.representation()->fillMeshGen(params);
    params.set("3d", false);
    params.set("force_include_north_pole", false);
    params.set("force_include_south_pole", false);

    auto grid = field.representation()->atlasGrid();
    auto mesh = caching::InMemoryMeshCache::atlasMesh(ctx.statistics(), grid, params);

    // Perform operation
    T operation(mesh);
    operation(field);

    if (polesMissingValues_) {
        auto missingValue = std::numeric_limits<double>::min();
        auto N            = field.representation()->numberOfPoints();

        for (size_t i = 0; i < field.dimensions(); ++i) {
            auto& values = field.direct(i);
            ASSERT(values.size() == N);

            for (const std::unique_ptr<repres::Iterator> it(field.representation()->iterator()); it->next();) {
                auto lat = it->pointUnrotated().lat().value();
                if (eckit::types::is_approximately_equal(lat, Latitude::NORTH_POLE.value()) ||
                    eckit::types::is_approximately_equal(lat, Latitude::SOUTH_POLE.value())) {
                    values.at(it->index()) = missingValue;
                }
            }
        }

        field.missingValue(missingValue);
        field.hasMissing(true);
    }
}


template <typename T>
const char* NablaFilterFVMT<T>::name() const {
    return T::name();
}


template <typename T>
void NablaFilterFVMT<T>::estimate(context::Context& /*unused*/, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


static const NablaFilterBuilder<NablaFilterFVMT<ScalarGradient>> __nabla1("scalar-gradient");
static const NablaFilterBuilder<NablaFilterFVMT<ScalarLaplacian>> __nabla2("scalar-laplacian");
static const NablaFilterBuilder<NablaFilterFVMT<UVGradient>> __nabla3("uv-gradient");
static const NablaFilterBuilder<NablaFilterFVMT<UVDivergence>> __nabla4("uv-divergence");
static const NablaFilterBuilder<NablaFilterFVMT<UVVorticity>> __nabla5("uv-vorticity");


}  // namespace mir::action
