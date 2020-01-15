/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   Dec 2016


#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Bytes.h"
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


class MIRLoadMatrix : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const {
        eckit::Log::info() << "\n"
                           << "Usage: " << tool << " [--load] [--unload] [--dump=path] <path>" << std::endl;
    }

    int minimumPositionalArguments() const { return 1; }

public:
    // -- Constructors

    MIRLoadMatrix(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "load", "Load file into memory. If file is already loaded, does nothing."));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "unload",
            "Unload file from memory. If file is not loaded, or loader does not employ shmem, does nothing."));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "wait", "After load/unload, wait for user input before exiting."));
        options_.push_back(new eckit::option::FactoryOption<mir::caching::matrix::MatrixLoaderFactory>(
            "matrix-loader", "Select how to load matrices in memory"));

        options_.push_back(new eckit::option::SimpleOption<eckit::PathName>("dump", "Matrix dump (needs --load)"));
        options_.push_back(new eckit::option::SimpleOption<eckit::PathName>(
            "write-csr", "Write matrix as CSR (needs --load, writes nna, nnz, ia, ja, a in 0-based indexing)"));
        options_.push_back(new eckit::option::SimpleOption<eckit::PathName>(
            "write-mm", "Write matrix as MatrixMarket (needs --load, output in 1-based indexing)"));
        options_.push_back(
            new eckit::option::SimpleOption<eckit::PathName>("write-dense", "Write dense matrix (needs --load)"));
    }
};


void display(eckit::Channel& out, mir::caching::matrix::MatrixLoader* loader, std::string path) {
    ASSERT(loader);

    // clang-format off
    out << "\n" "path:\t'" << path << "'"
        << "\n" "size:\t" << eckit::Bytes(loader->size())
        << "\n" "address:\t" << std::hex << loader->address() << std::dec
        << "\n" "inSharedMemory:\t" << std::boolalpha << loader->inSharedMemory() << std::noboolalpha
        << std::endl;
    // clang-format on
}


void MIRLoadMatrix::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::caching::matrix;
    using mir::method::WeightMatrix;

    eckit::Log::debug<mir::LibMir>().setStream(std::cerr);
    eckit::Log::info().setStream(std::cerr);

    const mir::param::ConfigurationWrapper param(args);

    bool load   = false;
    bool unload = false;
    bool wait   = false;
    param.get("load", load);
    param.get("unload", unload);
    param.get("wait", wait);

    std::string matrixLoader = "file-io";
    param.get("matrix-loader", matrixLoader);


    if (load || unload) {
        bool written = false;

        for (std::string path : args) {

            if (!load) {
                eckit::Log::info() << "---" "\n" "unloadSharedMemory" << std::endl;
                SharedMemoryLoader::unloadSharedMemory(path);
                continue;
            }

            // matrix construction transfers ownership from loader
            eckit::Log::info() << "---" "\n" "load" << std::endl;

            auto loader = MatrixLoaderFactory::build(matrixLoader, path);
            WeightMatrix W(loader);
            display(eckit::Log::info(), loader, path);

            if (!written) {
                std::string s;

                if (args.get("dump", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw eckit::CantOpenFile(file);
                    }

                    W.dump(out);

                    out.close();
                    if (out.bad()) {
                        throw eckit::WriteError(file);
                    }
                }

                if (args.get("write-csr", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw eckit::CantOpenFile(file);
                    }

                    out.precision(std::numeric_limits<double>::digits10);
                    static auto nl    = "\n";
                    static auto space = " ";

                    const auto nna = W.rows();
                    const auto nnz = W.nonZeros();
                    out << nna << nl << nnz;

                    auto ia(W.outer());
                    auto sep = nl;
                    for (WeightMatrix::Size i = 0; i <= nna; ++i, sep = space) {
                        out << sep << *(ia++);
                    }

                    auto ja(W.inner());
                    sep = nl;
                    for (WeightMatrix::Size i = 0; i < nnz; ++i, sep = space) {
                        out << sep << *(ja++);
                    }

                    auto a(W.data());
                    sep = nl;
                    for (WeightMatrix::Size i = 0; i < nnz; ++i, sep = space) {
                        out << sep << *(a++);
                    }

                    out << nl;

                    out.close();
                    if (out.bad()) {
                        throw eckit::WriteError(file);
                    }
                }

                if (args.get("write-mm", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw eckit::CantOpenFile(file);
                    }

                    out.precision(std::numeric_limits<double>::digits10);
                    static auto nl = "\n";

                    out << "%%MatrixMarket matrix coordinate real general" << nl;
                    out << W.rows() << " " << W.cols() << " " << W.nonZeros() << nl;

                    for (auto i = W.begin(); i != W.end(); ++i) {
                        out << (i.row() + 1) << " " << (i.col() + 1) << " " << *i << nl;
                    }

                    out.close();
                    if (out.bad()) {
                        throw eckit::WriteError(file);
                    }
                }

                if (args.get("write-dense", s)) {
                    written = true;

                    eckit::PathName file(s);
                    std::ofstream out(file.asString().c_str());
                    if (!out) {
                        throw eckit::CantOpenFile(file);
                    }

                    std::ostringstream str;
                    str.precision(std::numeric_limits<double>::digits10);

                    auto to_string = [&str](WeightMatrix::Scalar value) {
                        str.str("");
                        str << value;
                        return str.str();
                    };

                    static auto nl = "\n";
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
                        throw eckit::WriteError(file);
                    }
                }
            }

            if (unload) {
                auto shmLoader = dynamic_cast<SharedMemoryLoader*>(loader);
                if (shmLoader) {
                    eckit::Log::info() << "---" "\n" "unload" << std::endl;
                    shmLoader->unloadSharedMemory(path);
                    display(eckit::Log::info(), loader, path);
                }
            }
        }
    }

    if (wait) {
        eckit::Log::info() << "Press enter/return to continue..." << std::endl;
        std::cin.get();
    }
}


int main(int argc, char** argv) {
    MIRLoadMatrix tool(argc, argv);
    return tool.start();
}
