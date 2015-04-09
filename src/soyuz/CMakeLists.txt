list( APPEND mir_srcs
Action.cc
Action.h
AreaCropper.cc
Gridded.cc
Gridded.h
AreaCropper.h
Bitmap.cc
Bitmap.h
BitmapFilter.cc
BitmapFilter.h
DummyInput.cc
DummyInput.h
DummyOutput.cc
DummyOutput.h
ECMWFLogic.cc
ECMWFLogic.h
FrameFilter.cc
FrameFilter.h
Grib.h
GribFileInput.cc
GribFileInput.h
GribFileOutput.cc
GribFileOutput.h
GribInput.cc
GribInput.h
GribOutput.cc
GribOutput.h
GribStreamInput.cc
GribStreamInput.h
MARSLogic.cc
MARSLogic.h
MIRCombinedParametrisation.cc
MIRCombinedParametrisation.h
MIRConfiguration.cc
MIRConfiguration.h
MIRDefaults.cc
MIRDefaults.h
MIRField.cc
MIRField.h
MIRInput.cc
MIRInput.h
MIRJob.cc
MIRJob.h
MIRLogic.cc
MIRLogic.h
MIROutput.cc
MIROutput.h
MIRParametrisation.cc
MIRParametrisation.h
NetcdfFileInput.cc
NetcdfFileInput.h
NullCropper.cc
NullCropper.h
NullFilter.cc
NullFilter.h
NullInterpolator.cc
NullInterpolator.h
NullTransformer.cc
NullTransformer.h
RegularLatLon.cc
RegularLatLon.h
Representation.cc
Representation.h
Sh2ShTransform.cc
Sh2ShTransform.h
SphericalHarmonics.cc
SphericalHarmonics.h
emoslib.cc
)


ecbuild_add_library(TARGET		mir
					SOURCES		${mir_srcs}
					INCLUDES  ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
					LIBS	  ${ATLAS_LIBRARIES} ${ECKIT_LIBRARIES} ${NETCDF_LIBRARIES})


ecbuild_add_test(
		TARGET    soyuz
		SOURCES   mir.cc
		RESOURCES test.bitmap test.grib 10.grib test.nc
		INCLUDES  ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
		LIBS      mir ${ECKIT_LIBRARIES}    ${NETCDF_LIBRARIES}
)

