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


#include "mir/netcdf/ReshapeVariableStep.h"

#include <ostream>

#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/MergePlan.h"
#include "mir/netcdf/Remapping.h"
#include "mir/netcdf/Reshape.h"
#include "mir/netcdf/Variable.h"
#include "mir/util/Log.h"


namespace mir {
namespace netcdf {


ReshapeVariableStep::ReshapeVariableStep(Variable& out, const Dimension& dimension, size_t growth) :
    out_(out), dimension_(dimension), growth_(growth), next_(nullptr) {}

ReshapeVariableStep::~ReshapeVariableStep() {
    delete next_;
}

int ReshapeVariableStep::rank() const {
    return 2;
}

void ReshapeVariableStep::print(std::ostream& out) const {
    out << "ReshapeVariableStep[" << out_ << ", dim=" << dimension_;

    if (next_ != nullptr) {
        out << ", next=" << *next_;
    }
    out << "]";
}

void ReshapeVariableStep::execute(MergePlan& /*plan*/) {
    Log::info() << "ReshapeVariableStep::execute(): " << out_ << std::endl;
#if 0
    const std::vector<Dimension *> &dims = out_.dimensions();

    HyperCube &cube = out_.cube();

    std::vector<Dimension *> dimensions; // Dimension to reshape, in reverse variable order
    std::vector<size_t> growth;
    std::vector<size_t> indexes;

    size_t idx = dims.size() - 1;
    for (auto j = dims.rbegin(); j != dims.rend(); ++j, --idx) {
        ReshapeVariableStep *v = this;
        while (v) {
            if (&v->dimension_ == *j) {
                dimensions.push_back(*j);
                growth.push_back(v->growth_);
                indexes.push_back(idx);
            }
            v = v->next_;
        }
    }

    const Variable &in = plan.link(out_);
    HyperCube incube = in.cube();

    size_t i = 0;

    ASSERT(cube.size() == incube.size());

    for (auto j = dimensions.begin(); j != dimensions.end(); ++j, ++i) {

        // Add at end
        size_t where = (*j)->count();
        size_t idx = indexes[i];

        const Remapping &dimremap = (*j)->remapping();
        Log::info() << "ReshapeVariableStep::execute() - dimremap " << (*j)->name() << ": " << dimremap << std::endl;

        size_t gap = 0;
        for (size_t k = 0; k < dimremap.size(); k++) {
            size_t g = dimremap[k] - k;
            if (g != gap) {
                Log::info() << "ReshapeVariableStep::execute() - where=" << k << " gap=" << g << std::endl;

                in.matrix()->reshape(new Reshape(incube, idx, k, g - gap, 'I'));

                incube.dimensions()[idx] += g - gap;
                gap += g;
            }

        }

        if (growth[i]) {
            out_.matrix()->reshape(new Reshape(cube, idx, where, growth[i], 'O'));
            cube.dimensions()[idx] += growth[i];
        }

        if (incube.dimensions(idx) < cube.dimensions(idx)) {
            in.matrix()->reshape(new Reshape(incube, idx,
                                             incube.dimensions(idx), cube.dimensions(idx) - incube.dimensions(idx), 'F'));
            incube.dimensions()[idx] += cube.dimensions(idx) - incube.dimensions(idx);
        }

    }


    Log::info() << "ReshapeVariableStep::execute() - Remapping out " << std::endl;
    const std::vector<Reshape *> a = out_.matrix()->reshape();
    for (auto j = a.begin(); j != a.end(); ++j) Log::info() << *(*j) << std::endl;

    Log::info() << "ReshapeVariableStep::execute() - Remapping in " << std::endl;
    const std::vector<Reshape *> b = in.matrix()->reshape();
    for (auto j = b.begin(); j != b.end(); ++j) Log::info() << *(*j) << std::endl;
#endif
}

bool ReshapeVariableStep::merge(Step* other) {
    const auto* o = dynamic_cast<ReshapeVariableStep*>(other);
    if (o != nullptr) {
        // Same variable
        if (&(o->out_) == &(out_)) {

            auto* next                = new ReshapeVariableStep(o->out_, o->dimension_, o->growth_);
            auto* self                = this;
            ReshapeVariableStep* prev = nullptr;

            while (self != nullptr) {
                prev = self;
                self = self->next_;
            }

            if (prev != nullptr) {
                prev->next_ = next;
            }
            else {
                next_ = next;
            }

            return true;
        }
    }
    return false;
}

}  // namespace netcdf
}  // namespace mir
