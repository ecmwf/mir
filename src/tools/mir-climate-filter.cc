/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <limits>
#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/SparseMatrix.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/method/WeightMatrix.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/search/Tree.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Domain.h"
#include "mir/util/MIRStatistics.h"


using namespace mir;
using namespace eckit::linalg;


class MIRClimateFilter : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);

    int numberOfPositionalArguments() const { return 2; }

    void usage(const std::string& tool) const {
        eckit::Log::info() << "\nUsage: " << tool
#if 0
                           << " --grid=WE/SN|--gridname=<namedgrid>|--griddef=<path> [--key1=value [--key2=value [...]]] input.grib output.grib"
                              "\nExamples: "
                              "\n  % "
                           << tool << " --grid=1/1 --area=90/-180/-90/179 in out"
#endif
                           << std::endl;
    }

public:
    MIRClimateFilter(int argc, char** argv) : tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<size_t>("multiply-on-nnz", "Number of non-zeros to trigger interpolation"));
        options_.push_back(new eckit::option::VectorOption<size_t>("k", "Range of neighbour points to weight (k, default [4, infty[)", 2));
        options_.push_back(new eckit::option::SimpleOption<double>("distance", "Radius [m] of neighbours to weight (k) (default 1.)"));
        options_.push_back(new eckit::option::SimpleOption<double>("delta", "Climate filter (topographic data smoothing operator) width of filter edge, must be greater than 'distance' (default 1000.)"));
        options_.push_back(new eckit::option::FactoryOption<LinearAlgebra>("backend", "Linear algebra backend (default '" + LinearAlgebra::backend().name() + "')"));
        options_.push_back(new eckit::option::FactoryOption<search::TreeFactory>("point-search-trees", "k-d tree control"));
    }
};


struct SimpleClimateFilter {
    SimpleClimateFilter(Point3 point, size_t kMin, size_t kMax, double distance, double delta,
                        const search::PointSearch& search) :
        point_(point),
        kMin_(kMin),
        kMax_(kMax),
        distance_(distance),
        delta_(delta),
        search_(search) {
        ASSERT(kMin_ <= kMax_);
    }

    using search_results_t = std::vector<search::PointSearch::PointValueType>;
    using weight_results_t = std::vector<double>;

    void closest(const Point3& p, search_results_t& search) const {
        search_.closestWithinRadius(p, distance_, search);
        if (search.size() < kMin_) {
            search_.closestNPoints(p, kMax_, search);
        }
    }

    void operator()(const search_results_t& search, weight_results_t& weights) const {
        ASSERT(kMin_ < search.size());
        weights.resize(0);

        double sum = 0.;
        for (size_t k = 0; k < kMax_; ++k) {
            auto h = weight(search[k].point());
            weights.emplace_back(h);
            sum += h;
        }

        if (sum > 0.) {
            double invSum = 1. / sum;
            std::for_each(weights.begin(), weights.end(), [=](double& w) { w *= invSum; });
        }
    }

    double weight(const Point3& p) const {
        auto h = 0.5 + 0.5 * std::cos(M_PI_2 * (Point3::distance(point_, p) - 0.5 * distance_ + delta_) / delta_);
        return std::max(0., std::min(0.99, h));
    }

private:
    const Point3 point_;
    const size_t kMin_;
    const size_t kMax_;
    const double distance_;
    const double delta_;
    const search::PointSearch& search_;
};


struct SimpleTimer {
    SimpleTimer(std::string name) : name_(std::move(name)), t_(0) {}
    friend std::ostream& operator<<(std::ostream& s, SimpleTimer& p) {
        s << " " << p.name_ << ": " << eckit::Seconds(p.t_);
        p.t_ = 0;
        return s;
    }
    double& operator+=(double more) { return (t_ += more); }
private:
    std::string name_;
    double t_;
};


void MIRClimateFilter::execute(const eckit::option::CmdArgs& args) {
    eckit::ResourceUsage usage("MIRClimateFilter");
    eckit::TraceTimer<LibMir> timer("MIRClimateFilter");


    // linear algebra backend, statistics, input and output
    auto& la(LinearAlgebra::hasBackend("mkl") ? LinearAlgebra::getBackend("mkl") : LinearAlgebra::backend());

    util::MIRStatistics statistics;
    std::unique_ptr<input::MIRInput> in(new input::GribFileInput(args(0)));
    std::unique_ptr<output::MIROutput> out(new output::GribFileOutput(args(1)));
    ASSERT(in);
    ASSERT(out);

    auto& log = eckit::Log::info();
    static const param::DefaultParametrisation defaults;
    static const param::ConfigurationWrapper commandLine(args);


    int field = 0;
    while (in->next()) {
        log << "============> field: " << ++field << std::endl;

        // setup parametrisation
        std::unique_ptr<param::MIRParametrisation> param(
            new param::CombinedParametrisation(commandLine, in->parametrisation(), defaults));
        ASSERT(param);


        size_t nnz = 1000;
        param->get("multiply-on-nnz", nnz);
        ASSERT(nnz >= 2);

        std::vector<size_t> k{4, std::numeric_limits<size_t>::max()};
        param->get("k", k);
        ASSERT(k.size() == 2 && k[0] < k[1]);

        double distance = 1.;
        param->get("distance", distance = 1.);
        ASSERT(distance > 0.);

        double delta = 1000.;
        param->get("delta", delta);
        ASSERT(delta > 0.);


        // set field (input values, copied to output)
        double t = timer.elapsed();
        context::Context ctx(*in, statistics);
        ASSERT(ctx.field().dimensions() == 1);


        auto& input = ctx.field().values(0);
        mir::MIRValuesVector output(input);

        repres::RepresentationHandle rep(ctx.field().representation());
        if (!rep->domain().isGlobal()) {
            throw eckit::UserError("MIRClimateFilter: input field should be global");
        }
        log << "field: " << eckit::Seconds(timer.elapsed() - t) << std::endl;


        // build/get k-d tree
        t = timer.elapsed();
        search::PointSearch tree(*param, *rep);
        log << "k-d tree: " << eckit::Seconds(timer.elapsed() - t) << std::endl;


        // allocating temporary memory
        SimpleClimateFilter::search_results_t closest;
        SimpleClimateFilter::weight_results_t weights;

        std::vector<method::WeightMatrix::Triplet> triplets;
        triplets.reserve(size_t(nnz * 1.5));

        auto n         = rep->numberOfPoints();
        size_t i       = 0;
        Size rowOffset = 0;
        Vector x(const_cast<double*>(input.data()), n);

        {
            eckit::ProgressTimer progress("Locating", n, "point", double(5), log);

            SimpleTimer tClosest("closest");
            SimpleTimer tWeights("weights");
            SimpleTimer tMultipl("multiply");

            const std::unique_ptr<repres::Iterator> it(rep->iterator());
            while (it->next()) {
                if (++progress) {
                    log << PointLatLon(it->pointUnrotated()) << tClosest << tWeights << tMultipl << std::endl;
                    tree.statsPrint(log, false);
                    tree.statsReset();
                }

                ASSERT(i < n);
                const Point3 p(it->point3D());
                const SimpleClimateFilter filter(p, k[0], k[1], distance, delta, tree);

                t = timer.elapsed();
                filter.closest(p, closest);
                tClosest += timer.elapsed() - t;


                t = timer.elapsed();
                {
                    // calculate weights (possibly not all neighbour points are used), set weight triplets
                    filter(closest, weights);
                    ASSERT(weights.size() <= closest.size());

                    for (size_t j = 0; j < weights.size(); ++j) {
                        triplets.emplace_back(method::WeightMatrix::Triplet{i - rowOffset, closest[j].payload(), weights[j]});
                    }
                }
                tWeights += timer.elapsed() - t;


                ++i;
                if (triplets.size() > nnz) {
                    t = timer.elapsed();

                    Vector y(output.data() + rowOffset, n - rowOffset);
                    SparseMatrix M(n - rowOffset, n, triplets);
                    triplets.clear();
                    rowOffset = i;

                    la.spmv(M, x, y);
                    tMultipl += timer.elapsed() - t;
                }

            }
        }


        if (!triplets.empty()) {
            Vector y(output.data() + rowOffset, n - rowOffset);
            SparseMatrix M(n - rowOffset, n, triplets);
            triplets.clear();

            la.spmv(M, x, y);
        }


        log << eckit::Plural(field, "field") << " in " << eckit::Seconds(timer.elapsed())
            << ", rate: " << double(field) / double(timer.elapsed()) << " "
            << "field/s" << std::endl;
    }

    statistics.report(log);
}

int main(int argc, char** argv) {
    MIRClimateFilter tool(argc, argv);
    return tool.start();
}
