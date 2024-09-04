import os
from typing import Iterable, Tuple, List
from itertools import groupby

from distutils.core import setup
from distutils.extension import Extension

from Cython.Build import cythonize
from Cython.Distutils import build_ext

target_root = os.getenv("BUILD_ROOT", "/target/")
include_dirs = [f"/{target_root}/include"]
library_dirs = [f"/{target_root}/lib64"]
libraries = ["mir"]


def extract(prefix: str) -> Iterable[Tuple[str, List[str]]]:
    walk = os.walk(f"{target_root}{prefix}")
    # need: (relative prefix, list of full paths
    mapped = ((e[0][len(target_root):], f"{e[0]}/{f}") for e in walk for f in e[2])
    for k, g in groupby(mapped, lambda e: e[0]):
       	yield (k, list(e[1] for e in g))

# note this is tied to 'rpath' of linker invocation in the Extension
data_files = [ 
     ("lib", [f"/{target_root}lib64/{e}" for e in os.listdir(f"/{target_root}lib64") if e.endswith("so")]),
]
# configs of eckit, mir, eccodes
data_files.extend(extract("etc"))
# expectedly redundant since we compile eccodes with memfs instead of definitions/samples
data_files.extend(extract("share"))
# sadly it seems one cant list+generator in python, so we have to extend

setup(
    name="mir",
    version="0.2.0",
    setup_requires=['wheel'],
    ext_modules=cythonize(
        Extension(
            "mir",
            ["src/mir/mir.pyx", "src/mir/pyio.cc"],
            language="c++",
            libraries=libraries,
            library_dirs=library_dirs,
            include_dirs=include_dirs + ['src/mir'],
            extra_compile_args=["-std=c++17"],
            extra_link_args=["-std=c++17", "-Wl,-rpath,$ORIGIN/../.."],
        ),
        compiler_directives={"language_level": 3, "c_string_encoding": "default"},
    ),
    data_files=data_files,
    include_package_data=True,
)
