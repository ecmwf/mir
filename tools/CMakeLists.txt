ecbuild_add_executable( TARGET  eigen_mmap_dense
						SOURCES eigen_mmap_dense.cpp utils.h
						CONDITION EIGEN3_FOUND )

ecbuild_add_executable( TARGET  eigen_sparse
						SOURCES eigen_sparse.cpp utils.h
						CONDITION EIGEN3_FOUND )

ecbuild_add_executable( TARGET  trilinos_sparse
						SOURCES trilinos_sparse.cpp utils.h
						CONDITION TRILINOS_FOUND )
