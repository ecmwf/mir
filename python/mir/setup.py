import os
import sys
import warnings
from itertools import groupby
from pathlib import Path
from typing import Iterable
from typing import List
from typing import Tuple

from Cython.Build import cythonize
from setuptools import Extension
from setuptools import setup

define_macros = []
extra_include_dirs = []
if "--without-numpy" not in sys.argv:
    from numpy import get_include

    define_macros.append(("MIR_PYTHON_HAVE_NUMPY", 1))
    define_macros.append(("NPY_NO_DEPRECATED_API", "NPY_1_7_API_VERSION"))
    extra_include_dirs = [get_include()]
else:
    define_macros.append(("MIR_PYTHON_HAVE_NUMPY", 0))

if source_lib_root := os.getenv("SOURCE_LIB_ROOT", ""):
    # NOTE this whole branch is probably obsolete -- we dont want to build such wheels anymore
    print(f"assuming standalone wheel building, with {source_lib_root=}")

    include_dirs = [f"{source_lib_root}/include"]
    library_dirs = [f"{source_lib_root}/lib64"]

    def extract(prefix: str) -> Iterable[Tuple[str, List[str]]]:
        walk = os.walk(f"{source_lib_root}{prefix}")
        # need: (relative prefix, list of full paths
        mapped = (
            (e[0][len(source_lib_root) :], f"{e[0]}/{f}") for e in walk for f in e[2]
        )
        for k, g in groupby(mapped, lambda e: e[0]):
            yield (k, list(e[1] for e in g))

    # NOTE the `-rpath` to linker must hit the `lib` which the data_files install to
    extra_link_args = ["-std=c++17", "-Wl,-rpath,$ORIGIN/../.."]
    data_files = [
        (
            "lib",
            [
                f"{source_lib_root}lib64/{e}"
                for e in os.listdir(f"{source_lib_root}lib64")
                if ".so" in e
            ],
        ),
    ]
    # etc & share: configs of eckit, mir, eccodes
    # sadly it seems one cant list+generator in python, so we have to extend
    data_files.extend(extract("etc/eckit"))
    data_files.extend(extract("etc/eccodes"))
    data_files.extend(extract("etc/atlas"))
    data_files.extend(extract("etc/mir"))
    data_files.extend(
        extract("share/eccodes")
    )  # possibly redundant since we compile eccodes with memfs
    data_files.extend(extract("share/eckit"))
    data_files.extend(extract("etc/atlas"))
    data_files.extend(extract("share/mir"))
    kwargs_set = {
        "data_files": data_files,
    }
    kwargs_ext = {
        "extra_link_args": extra_link_args,
    }
else:
    home = os.getenv("HOME")
    source = os.getenv("MIR_BUNDLE_SOURCE_DIR", str(Path(home, "git", "mir-bundle")))
    build = os.getenv("MIR_BUNDLE_BUILD_DIR", str(Path(home, "build", "mir-bundle")))
    library_dirs = os.getenv("MIR_LIB_DIR", str(Path(build, "lib"))).split(":")

    include_dirs_default = ":".join(
        str(Path(base, *path))
        for base in (source, build)
        for path in (
            ("mir", "src"),
            ("eccodes", "src"),
            ("eccodes", "src", "eccodes"),
            ("eckit", "src"),
        )
    )
    include_dirs = os.getenv("MIR_INCLUDE_DIRS", include_dirs_default).split(":")

    extra_link_args = []
    kwargs_set = {}
    kwargs_ext = {
        "runtime_library_dirs": library_dirs,
        "extra_link_args": ["-std=c++17"],
    }
    print(
        f"assuming .so module building, with library_dirs: {' '.join(library_dirs)} and include_dirs: {' '.join(include_dirs)}"
    )

try:
    from setup_utils import ext_kwargs as wheel_ext_kwargs

    kwargs_set.update(wheel_ext_kwargs[sys.platform])
except ImportError:
    warnings.warn("failed to import setup_utils, won't mark the wheel as manylinux")

version: str
try:
    with open("../../VERSION", "r") as f:
        version = f.readlines()[0].strip()
except Exception:
    warnings.warn("failed to read VERSION, falling back to 0.0.0")
    version = "0.0.0"

install_requires = ["findlibs", "numpy", "pyyaml"]
try:
    import mirlib

    install_requires.append(f"mirlib=={mirlib.__version__}")
except ImportError:
    warnings.warn("failed to import prereq libs, not listing as a dependency")

setup(
    name="mir-python",
    version=version,
    install_requires=install_requires,
    ext_modules=cythonize(
        Extension(
            "_mir",
            [
                "src/_mir/_mir.pyx",
                "src/_mir/mir/input/ArrayInput.cc",
                "src/_mir/mir/input/PyGribInput.cc",
                "src/_mir/mir/output/ArrayOutput.cc",
                "src/_mir/mir/output/PyGribOutput.cc",
            ],
            language="c++",
            libraries=["mir"],
            library_dirs=library_dirs,
            include_dirs=include_dirs + extra_include_dirs + ["src/_mir"],
            extra_compile_args=["-std=c++17"],
            define_macros=define_macros,
            **kwargs_ext,
        ),
        compiler_directives={"language_level": 3, "c_string_encoding": "default"},
    ),
    **kwargs_set,
)
