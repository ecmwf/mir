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


#include <fstream>
#include <ios>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"

#include "mir/caching/matrix/MatrixLoader.h"
#include "mir/caching/matrix/SharedMemoryLoader.h"
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRLoadMatrix : MIRTool {
    MIRLoadMatrix(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(
            new SimpleOption<bool>("load", "Load file into memory. If file is already loaded, does nothing."));
        options_.push_back(new SimpleOption<bool>(
            "unload",
            "Unload file from memory. If file is not loaded, or loader does not employ shmem, does nothing."));
        options_.push_back(new SimpleOption<bool>("wait", "After load/unload, wait for user input before exiting."));
        options_.push_back(new FactoryOption<caching::matrix::MatrixLoaderFactory>(
            "matrix-loader", "Select how to load matrices in memory"));

        options_.push_back(new SimpleOption<eckit::PathName>("dump", "Matrix dump (needs --load)"));
        options_.push_back(new SimpleOption<eckit::PathName>(
            "write-csr", "Write matrix as CSR (needs --load, writes nna, nnz, ia, ja, a in 0-based indexing)"));
        options_.push_back(new SimpleOption<eckit::PathName>(
            "write-mm", "Write matrix as MatrixMarket (needs --load, output in 1-based indexing)"));
        options_.push_back(new SimpleOption<eckit::PathName>("write-dense", "Write dense matrix (needs --load)"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Usage: " << tool << " [--load] [--unload] [--dump=path] <path>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
};


void display(Log::Channel& out, caching::matrix::MatrixLoader* loader, const std::string& path) {
    ASSERT(loader);

    // clang-format off
    out << "\n" "path:\t'" << path << "'"
        << "\n" "size:\t" << Log::Bytes(loader->size())
        << "\n" "address:\t" << std::hex << loader->address() << std::dec
        << "\n" "inSharedMemory:\t" << std::boolalpha << loader->inSharedMemory() << std::noboolalpha
        << std::endl;
    // clang-format on
}


void MIRLoadMatrix::execute(const eckit::option::CmdArgs& args) {
    using caching::matrix::SharedMemoryLoader;
    using method::WeightMatrix;

    Log::debug().setStream(std::cerr);
    Log::info().setStream(std::cerr);

    const param::ConfigurationWrapper param(args);

    bool load   = false;
    bool unload = false;
    bool wait   = false;
    param.get("load", load);
    param.get("unload", unload);
    param.get("wait", wait);

    std::string matrixLoader = LibMir::cacheLoader(LibMir::cache_loader::MATRIX);
    param.get("matrix-loader", matrixLoader);


    if (load || unload) {
        bool written = false;

        for (const std::string& path : args) {

            if (!load) {
                Log::info() << "---"
                               "\n"
                               "unloadSharedMemory"
                            << std::endl;
                SharedMemoryLoader::unloadSharedMemory(path);
                continue;
            }

            // matrix construction transfers ownership from loader
            Log::info() << "---"
                           "\n"
                           "load"
                        << std::endl;

            auto* loader = caching::matrix::MatrixLoaderFactory::build(matrixLoader, path);
            WeightMatrix W(loader);
            display(Log::info(), loader, path);

            if (!written) {
                std::string s;

                if (args.get("dump", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw exception::CantOpenFile(file);
                    }

                    W.dump(out);

                    out.close();
                    if (out.bad()) {
                        throw exception::WriteError(file);
                    }
                }

                if (args.get("write-csr", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw exception::CantOpenFile(file);
                    }

                    out.precision(std::numeric_limits<double>::digits10);
                    static const auto* nl    = "\n";
                    static const auto* space = " ";

                    const auto nna = W.rows();
                    const auto nnz = W.nonZeros();
                    out << nna << nl << nnz;

                    const auto* ia(W.outer());
                    const auto* sep = nl;
                    for (WeightMatrix::Size i = 0; i <= nna; ++i, sep = space) {
                        out << sep << *(ia++);
                    }

                    const auto* ja(W.inner());
                    sep = nl;
                    for (WeightMatrix::Size i = 0; i < nnz; ++i, sep = space) {
                        out << sep << *(ja++);
                    }

                    const auto* a(W.data());
                    sep = nl;
                    for (WeightMatrix::Size i = 0; i < nnz; ++i, sep = space) {
                        out << sep << *(a++);
                    }

                    out << nl;

                    out.close();
                    if (out.bad()) {
                        throw exception::WriteError(file);
                    }
                }

                if (args.get("write-mm", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw exception::CantOpenFile(file);
                    }

                    out.precision(std::numeric_limits<double>::digits10);
                    static const auto* nl = "\n";

                    out << "%%MatrixMarket matrix coordinate real general" << nl;
                    out << W.rows() << " " << W.cols() << " " << W.nonZeros() << nl;

                    for (auto i = W.begin(); i != W.end(); ++i) {
                        out << (i.row() + 1) << " " << (i.col() + 1) << " " << *i << nl;
                    }

                    out.close();
                    if (out.bad()) {
                        throw exception::WriteError(file);
                    }
                }

                if (args.get("write-dense", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw exception::CantOpenFile(file);
                    }

                    std::ostringstream str;
                    str.precision(std::numeric_limits<double>::digits10);

                    auto to_string = [&str](WeightMatrix::Scalar value) {
                        str.str("");
                        str << value;
                        return str.str();
                    };

                    static const auto* nl = "\n";
                    out << "# rows=" << W.rows() << nl << "# cols=" << W.cols() << nl << "# nonZeros=" << W.nonZeros()
                        << nl;

                    auto it = W.begin();
                    for (WeightMatrix::Size i = 0; i < W.rows(); ++i) {
                        std::vector<std::string> row(W.cols(), ".");
                        for (auto end = W.end(i); it != end; ++it) {
                            row[it.col()] = to_string(*it);
                        }
                        out << eckit::StringTools::join("\t", row) << nl;
                    }

                    out.close();
                    if (out.bad()) {
                        throw exception::WriteError(file);
                    }
                }
            }

            if (unload) {
                auto* shmLoader = dynamic_cast<SharedMemoryLoader*>(loader);
                if (shmLoader != nullptr) {
                    Log::info() << "---"
                                   "\n"
                                   "unload"
                                << std::endl;
                    SharedMemoryLoader::unloadSharedMemory(path);
                    display(Log::info(), loader, path);
                }
            }
        }
    }

    if (wait) {
        Log::info() << "Press enter/return to continue..." << std::endl;
        std::cin.get();
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRLoadMatrix tool(argc, argv);
    return tool.start();
}
