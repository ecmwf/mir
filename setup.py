import os
from typing import Iterable, Tuple, List
from itertools import groupby

from distutils.core import setup
from distutils.extension import Extension

from Cython.Build import cythonize
from Cython.Distutils import build_ext


if (source_lib_root := os.getenv("SOURCE_LIB_ROOT", "")):
	print(f"assuming standalone wheel building, with {source_lib_root=}")

	include_dirs = [f"{source_lib_root}/include"]
	library_dirs = [f"{source_lib_root}/lib64"]

	def extract(prefix: str) -> Iterable[Tuple[str, List[str]]]:
		walk = os.walk(f"{source_lib_root}{prefix}")
		# need: (relative prefix, list of full paths
		mapped = ((e[0][len(source_lib_root):], f"{e[0]}/{f}") for e in walk for f in e[2])
		for k, g in groupby(mapped, lambda e: e[0]):
			yield (k, list(e[1] for e in g))

	# NOTE the `-rpath` to linker must hit the `lib` which the data_files install to
	extra_link_args = ["-std=c++17", "-Wl,-rpath,$ORIGIN/../.."]
	data_files = [
		("lib", [f"{source_lib_root}lib64/{e}" for e in os.listdir(f"{source_lib_root}lib64") if e.endswith("so")]),
	]
	# etc & share: configs of eckit, mir, eccodes
	# sadly it seems one cant list+generator in python, so we have to extend
	data_files.extend(extract("etc/eccodes"))
	data_files.extend(extract("etc/eckit"))
	data_files.extend(extract("etc/mir"))
	data_files.extend(extract("share/eccodes")) # possibly redundant since we compile eccodes with memfs
	data_files.extend(extract("share/eckit"))
	data_files.extend(extract("share/mir"))
	kwargs_set = {
		"data_files": data_files,
	}
	kwargs_ext = {
		"extra_link_args": extra_link_args,
	}
else:
	home = os.environ.get("HOME")
	source = os.environ.get("MIR_BUNDLE_SOURCE_DIR", os.path.join(home, "git", "mir-bundle"))
	build = os.environ.get("MIR_BUNDLE_BUILD_DIR", os.path.join(home, "build", "mir-bundle"))
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
	extra_link_args = []
	kwargs_set = {}
	kwargs_ext = {
		"runtime_library_dirs": library_dirs,
		"extra_link_args": ["-std=c++17"]
	}
	print(f"assuming .so module building, with library_dirs: {' '.join(library_dirs)} and include_dirs: {' '.join(include_dirs)}")

setup(
	name="mir-python",
	version="0.2.0",
	ext_modules=cythonize(
		Extension(
			"mir",
			["src/mir/mir.pyx", "src/mir/pyio.cc"],
			language="c++",
			libraries=["mir"],
			library_dirs=library_dirs,
			include_dirs=include_dirs + ['src/mir'],
			extra_compile_args=["-std=c++17"],
			**kwargs_ext,
		),
		compiler_directives={"language_level": 3, "c_string_encoding": "default"},
	),
	**kwargs_set,
)
