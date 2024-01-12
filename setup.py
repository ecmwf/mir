import os
from distutils.core import setup
from distutils.extension import Extension

from Cython.Build import cythonize
from Cython.Distutils import build_ext

home = os.environ.get("HOME")
source = os.environ.get(
    "MIR_BUNDLE_SOURCE_DIR", os.path.join(home, "git", "mir-bundle")
)
build = os.environ.get(
    "MIR_BUNDLE_BUILD_DIR", os.path.join(home, "build", "mir-bundle")
)

library_dirs = (
    os.environ.get("MIR_LIB_DIR").split(":")
    if "MIR_LIB_DIR" in os.environ
    else [os.path.join(build, "lib")]
)

include_dirs = (
    os.environ.get("MIR_INCLUDE_DIRS").split(":")
    if "MIR_INCLUDE_DIRS" in os.environ
    else [
        os.path.join(base, pkg, "src")
        for base in [source, build]
        for pkg in ["mir", "eckit", "eccodes"]
    ]
)

print("library_dirs:", " ".join(library_dirs))
print("include_dirs:", " ".join(include_dirs))

# FIXME: sort out how to discover MIR libraries
setup(
    name="mir-python",
    version="0.2.0",
    ext_modules=cythonize(
        Extension(
            "mir",
            ["mir.pyx", "pyio.cc"],
            language="c++",
            libraries=["mir"],
            library_dirs=library_dirs,
            runtime_library_dirs=library_dirs,
            include_dirs=include_dirs,
            extra_compile_args=["-std=c++17"],
            extra_link_args=["-std=c++17"],
        ),
        compiler_directives={"language_level": 3, "c_string_encoding": "default"},
    ),
)
