### config headers

ecbuild_generate_config_headers( DESTINATION $ {INSTALL_INCLUDE_DIR}/mir )

configure_file( api/mir_config.h.in   api/mir_config.h  )
configure_file( api/mir_version.h.in  api/mir_version.h )

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/api/mir_config.h
        ${CMAKE_CURRENT_BINARY_DIR}/api/mir_version.h
        DESTINATION
        ${INSTALL_INCLUDE_DIR}/mir )

configure_file( api/mir_version.cc.in  api/mir_version.cc )

### mir sources

# list( APPEND mir_srcs api/mir_version.cc )

list( APPEND mir_srcs
      action/Action.cc
      action/Action.h
      action/ActionPlan.cc
      action/ActionPlan.h
      action/AreaCropper.cc
      action/AreaCropper.h
      action/BitmapFilter.cc
      action/BitmapFilter.h
      action/FrameFilter.cc
      action/FrameFilter.h
      action/Gridded2GriddedInterpolation.cc
      action/Gridded2GriddedInterpolation.h
      action/Gridded2ReducedGG.cc
      action/Gridded2ReducedGG.h
      action/Gridded2ReducedLL.cc
      action/Gridded2ReducedLL.h
      action/Gridded2RegularGG.cc
      action/Gridded2RegularGG.h
      action/Gridded2RegularLL.cc
      action/Gridded2RegularLL.h
      action/NullAction.cc
      action/NullAction.h
      action/Sh2GriddedTransform.cc
      action/Sh2GriddedTransform.h
      action/Sh2ReducedGG.cc
      action/Sh2ReducedGG.h
      action/Sh2RegularGG.cc
      action/Sh2RegularGG.h
      action/Sh2RegularLL.cc
      action/Sh2RegularLL.h
      action/Sh2ShTransform.cc
      action/Sh2ShTransform.h
      action/VOD2UVTransform.cc
      action/VOD2UVTransform.h
      api/MIRJob.cc
      api/MIRJob.h
      api/emoslib.cc
      data/MIRField.cc
      data/MIRField.h
      input/DummyInput.cc
      input/DummyInput.h
      input/GribFileInput.cc
      input/GribFileInput.h
      input/GribInput.cc
      input/GribInput.h
      input/GribMemoryInput.cc
      input/GribMemoryInput.h
      input/GribStreamInput.cc
      input/GribStreamInput.h
      input/MIRInput.cc
      input/MIRInput.h
      input/NetcdfFileInput.cc
      input/NetcdfFileInput.h
      logic/AutoReduced.cc
      logic/AutoReduced.h
      logic/AutoResol.cc
      logic/AutoResol.h
      logic/MARSLogic.cc
      logic/MARSLogic.h
      logic/MIRLogic.cc
      logic/MIRLogic.h
      logic/ToolLogic.cc
      logic/ToolLogic.h
      method/Bilinear.cc
      method/Bilinear.h
      method/FiniteElement.cc
      method/FiniteElement.h
      method/KNearest.cc
      method/KNearest.h
      method/Method.cc
      method/Method.h
      method/MethodWeighted.cc
      method/MethodWeighted.h
      method/PseudoLaplace.cc
      method/PseudoLaplace.h
      method/WeightCache.cc
      method/WeightCache.h
      output/DummyOutput.cc
      output/DummyOutput.h
      output/GribFileOutput.cc
      output/GribFileOutput.h
      output/GribMemoryOutput.cc
      output/GribMemoryOutput.h
      output/GribOutput.cc
      output/GribOutput.h
      output/GribStreamOutput.cc
      output/GribStreamOutput.h
      output/MIROutput.cc
      output/MIROutput.h
      param/DelayedParametrisation.cc
      param/DelayedParametrisation.h
      param/FieldParametrisation.cc
      param/FieldParametrisation.h
      param/MIRCombinedParametrisation.cc
      param/MIRCombinedParametrisation.h
      param/MIRConfiguration.cc
      param/MIRConfiguration.h
      param/MIRDefaults.cc
      param/MIRDefaults.h
      param/MIRParametrisation.cc
      param/MIRParametrisation.h
      param/RuntimeParametrisation.cc
      param/RuntimeParametrisation.h
      param/SimpleParametrisation.cc
      param/SimpleParametrisation.h
      repres/Albers.cc
      repres/Albers.h
      repres/AzimuthRange.cc
      repres/AzimuthRange.h
      repres/EquatorialAzimuthalEquidistant.cc
      repres/EquatorialAzimuthalEquidistant.h
      repres/Gridded.cc
      repres/Gridded.h
      repres/IrregularLatlon.cc
      repres/IrregularLatlon.h
      repres/Lambert.cc
      repres/Lambert.h
      repres/LambertAzimuthalEqualArea.cc
      repres/LambertAzimuthalEqualArea.h
      repres/Mercator.cc
      repres/Mercator.h
      repres/PolarStereographic.cc
      repres/PolarStereographic.h
      repres/ReducedGG.cc
      repres/ReducedGG.h
      repres/ReducedLL.cc
      repres/ReducedLL.h
      repres/RegularGG.cc
      repres/RegularGG.h
      repres/RegularLL.cc
      repres/RegularLL.h
      repres/Representation.cc
      repres/Representation.h
      repres/RotatedGG.cc
      repres/RotatedGG.h
      repres/RotatedLL.cc
      repres/RotatedLL.h
      repres/RotatedSH.cc
      repres/RotatedSH.h
      repres/SpaceView.cc
      repres/SpaceView.h
      repres/SphericalHarmonics.cc
      repres/SphericalHarmonics.h
      repres/StretchedGG.cc
      repres/StretchedGG.h
      repres/StretchedLL.cc
      repres/StretchedLL.h
      repres/StretchedRotatedGG.cc
      repres/StretchedRotatedGG.h
      repres/StretchedRotatedLL.cc
      repres/StretchedRotatedLL.h
      repres/StretchedRotatedSH.cc
      repres/StretchedRotatedSH.h
      repres/StretchedSH.cc
      repres/StretchedSH.h
      repres/TransverseMercator.cc
      repres/TransverseMercator.h
      repres/TriangularGrid.cc
      repres/TriangularGrid.h
      repres/UnstructuredGrid.cc
      repres/UnstructuredGrid.h
      util/Arguments.cc
      util/Arguments.h
      util/Bitmap.cc
      util/Bitmap.h
      util/BoundingBox.cc
      util/BoundingBox.h
      util/Grib.h
      util/Increments.cc
      util/Increments.h
      util/PointSearch.cc
      util/PointSearch.h
    )


ecbuild_add_library(TARGET             mir
                    INSTALL_HEADERS    LISTED
                    HEADER_DESTINATION ${INSTALL_INCLUDE_DIR}/mir
                    SOURCES            ${mir_srcs}
                    GENERATED          api/mir_version.cc
                    INCLUDES           ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
                    LIBS               ${ATLAS_LIBRARIES} ${ECKIT_LIBRARIES} ${NETCDF_LIBRARIES})


