ecbuild_add_executable(
		TARGET      oultine
		SOURCES
Cropper.cc
Cropper.h
Interpolator.cc
Interpolator.h
MIRField.cc
MIRField.h
MIRInput.cc
MIRInput.h
MIRJob.cc
MIRJob.h
MIROuput.cc
MIROuput.h
MIRParam.h
Masker.cc
Masker.h
outline.cc
		INCLUDES    ${ECKIT_INCLUDE_DIRS}
		LIBS        ${ECKIT_LIBRARIES} )

