### system headers

include(CheckCSourceCompiles)

check_c_source_compiles("
#include <sys/stat.h>
int main() {
    struct stat s;
    s.st_ctimespec;
    return 0;
}"
HAS_SYS_STAT_ST_CTIMESPEC)

### config headers

ecbuild_generate_config_headers(DESTINATION ${INSTALL_INCLUDE_DIR}/mir)

configure_file(api/mir_config.h.in api/mir_config.h @ONLY)
configure_file(api/mir_version.h.in api/mir_version.h @ONLY)
configure_file(api/mir_version.cc.in api/mir_version.cc @ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/api/mir_config.h ${CMAKE_CURRENT_BINARY_DIR}/api/mir_version.h
    DESTINATION ${INSTALL_INCLUDE_DIR}/mir/api)


unset(mir_srcs)
list(APPEND mir_srcs

    ${CMAKE_CURRENT_BINARY_DIR}/api/mir_version.cc

    action/area/Area.cc
    action/area/Area.h
    action/area/AreaCropper.cc
    action/area/AreaCropper.h
    action/area/AreaMasker.cc
    action/area/AreaMasker.h
    action/calc/FormulaAction.cc
    action/calc/FormulaAction.h
    action/context/Context.cc
    action/context/Context.h
    action/filter/AddRandomFilter.cc
    action/filter/AddRandomFilter.h
    action/filter/AdjustWindsAtPoles.cc
    action/filter/AdjustWindsAtPoles.h
    action/filter/AdjustWindsDirections.cc
    action/filter/AdjustWindsDirections.h
    action/filter/AdjustWindsScaleCosLatitude.cc
    action/filter/AdjustWindsScaleCosLatitude.h
    action/filter/Bandpass.cc
    action/filter/Bandpass.h
    action/filter/BitmapFilter.cc
    action/filter/BitmapFilter.h
    action/filter/CesaroSummationFilter.cc
    action/filter/CesaroSummationFilter.h
    action/filter/FrameFilter.cc
    action/filter/FrameFilter.h
    action/filter/GlobaliseFilter.cc
    action/filter/GlobaliseFilter.h
    action/filter/NablaFilter.cc
    action/filter/NablaFilter.h
    action/filter/ShTruncate.cc
    action/filter/ShTruncate.h
    action/filter/StatisticsFilter.cc
    action/filter/StatisticsFilter.h
    action/filter/UnstructuredFilter.cc
    action/filter/UnstructuredFilter.h
    action/interpolate/Gridded2GridDef.cc
    action/interpolate/Gridded2GridDef.h
    action/interpolate/Gridded2GridSpec.cc
    action/interpolate/Gridded2GridSpec.h
    action/interpolate/Gridded2GriddedInterpolation.cc
    action/interpolate/Gridded2GriddedInterpolation.h
    action/interpolate/Gridded2NamedGrid.cc
    action/interpolate/Gridded2NamedGrid.h
    action/interpolate/Gridded2OctahedralGG.cc
    action/interpolate/Gridded2OctahedralGG.h
    action/interpolate/Gridded2Points.cc
    action/interpolate/Gridded2Points.h
    action/interpolate/Gridded2ReducedGG.cc
    action/interpolate/Gridded2ReducedGG.h
    action/interpolate/Gridded2ReducedGGPLGiven.cc
    action/interpolate/Gridded2ReducedGGPLGiven.h
    action/interpolate/Gridded2ReducedLL.cc
    action/interpolate/Gridded2ReducedLL.h
    action/interpolate/Gridded2ReducedLLPLGiven.cc
    action/interpolate/Gridded2ReducedLLPLGiven.h
    action/interpolate/Gridded2RegularGG.cc
    action/interpolate/Gridded2RegularGG.h
    action/interpolate/Gridded2RegularLL.cc
    action/interpolate/Gridded2RegularLL.h
    action/interpolate/Gridded2RotatedGrid.cc
    action/interpolate/Gridded2RotatedGrid.h
    action/interpolate/Gridded2RotatedLL.cc
    action/interpolate/Gridded2RotatedLL.h
    action/interpolate/Gridded2RotatedNamedGrid.cc
    action/interpolate/Gridded2RotatedNamedGrid.h
    action/interpolate/Gridded2RotatedOctahedralGG.cc
    action/interpolate/Gridded2RotatedOctahedralGG.h
    action/interpolate/Gridded2RotatedReducedGG.cc
    action/interpolate/Gridded2RotatedReducedGG.h
    action/interpolate/Gridded2RotatedReducedGGPLGiven.cc
    action/interpolate/Gridded2RotatedReducedGGPLGiven.h
    action/interpolate/Gridded2RotatedRegularGG.cc
    action/interpolate/Gridded2RotatedRegularGG.h
    action/interpolate/Gridded2TypedGrid.cc
    action/interpolate/Gridded2TypedGrid.h
    action/interpolate/Gridded2UnrotatedGrid.cc
    action/interpolate/Gridded2UnrotatedGrid.h
    action/io/Copy.cc
    action/io/Copy.h
    action/io/EndAction.cc
    action/io/EndAction.h
    action/io/Save.cc
    action/io/Save.h
    action/io/Set.cc
    action/io/Set.h
    action/misc/CheckerBoard.cc
    action/misc/CheckerBoard.h
    action/misc/Noop.cc
    action/misc/Noop.h
    action/misc/ReferencePattern.cc
    action/misc/ReferencePattern.h
    action/misc/ResetMissingValues.cc
    action/misc/ResetMissingValues.h
    action/misc/SelectField.cc
    action/misc/SelectField.h
    action/misc/SetMetadata.cc
    action/misc/SetMetadata.h
    action/plan/Action.cc
    action/plan/Action.h
    action/plan/ActionGraph.cc
    action/plan/ActionGraph.h
    action/plan/ActionNode.cc
    action/plan/ActionNode.h
    action/plan/ActionPlan.cc
    action/plan/ActionPlan.h
    action/plan/Executor.cc
    action/plan/Executor.h
    action/plan/Job.cc
    action/plan/Job.h
    action/plan/SimpleExecutor.cc
    action/plan/SimpleExecutor.h
    action/plan/ThreadExecutor.cc
    action/plan/ThreadExecutor.h
    api/MIRComplexJob.cc
    api/MIRComplexJob.h
    api/MIRJob.cc
    api/MIRJob.h
    api/MIRWatcher.cc
    api/MIRWatcher.h
    caching/AreaCacheEntry.cc
    caching/AreaCacheEntry.h
    caching/AreaCropperCache.cc
    caching/AreaCropperCache.h
    caching/AreaMaskerCache.cc
    caching/AreaMaskerCache.h
    caching/InMemoryCache.cc
    caching/InMemoryCache.h
    caching/InMemoryCacheBase.cc
    caching/InMemoryCacheBase.h
    caching/InMemoryCacheStatistics.cc
    caching/InMemoryCacheStatistics.h
    caching/InMemoryCacheUsage.cc
    caching/InMemoryCacheUsage.h
    caching/SharedMemoryKey.cc
    caching/SharedMemoryKey.h
    caching/WeightCache.cc
    caching/WeightCache.h
    caching/matrix/FileLoader.cc
    caching/matrix/FileLoader.h
    caching/matrix/MatrixLoader.cc
    caching/matrix/MatrixLoader.h
    caching/matrix/SharedMemoryLoader.cc
    caching/matrix/SharedMemoryLoader.h
    compare/BufrField.cc
    compare/BufrField.h
    compare/Field.cc
    compare/Field.h
    compare/FieldComparator.cc
    compare/FieldComparator.h
    compare/FieldInfo.cc
    compare/FieldInfo.h
    compare/FieldSet.cc
    compare/FieldSet.h
    compare/GribField.cc
    compare/GribField.h
    compare/MultiFile.cc
    compare/MultiFile.h
    compat/GribCompatibility.cc
    compat/GribCompatibility.h
    config/LibMir.cc
    config/LibMir.h
    data/CartesianVector2DField.cc
    data/CartesianVector2DField.h
    data/Field.cc
    data/Field.h
    data/MIRField.cc
    data/MIRField.h
    data/MIRFieldStats.cc
    data/MIRFieldStats.h
    data/Space.cc
    data/Space.h
    data/space/Space1DAngleT.cc
    data/space/Space1DAngleT.h
    data/space/Space1DLinear.cc
    data/space/Space1DLinear.h
    data/space/Space2DVectorZonalMeridional.cc
    data/space/Space2DVectorZonalMeridional.h
    data/space/Space3DCartesian.cc
    data/space/Space3DCartesian.h
    data/space/Space3DVectorZonalMeridionalVertical.cc
    data/space/Space3DVectorZonalMeridionalVertical.h
    data/space/SpaceLinear.cc
    data/space/SpaceLinear.h
    data/space/SpaceLogarithmic.cc
    data/space/SpaceLogarithmic.h
    grib/BasicAngle.cc
    grib/BasicAngle.h
    grib/Config.cc
    grib/Config.h
    grib/Packing.cc
    grib/Packing.h
    input/ArtificialInput.cc
    input/ArtificialInput.h
    input/ConstantInput.cc
    input/ConstantInput.h
    input/DistributionInput.cc
    input/DistributionInput.h
    input/DummyInput.cc
    input/DummyInput.h
    input/EmptyInput.cc
    input/EmptyInput.h
    input/GeoPointsFileInput.cc
    input/GeoPointsFileInput.h
    input/GribAllFileInput.cc
    input/GribAllFileInput.h
    input/GribDataHandleInput.cc
    input/GribDataHandleInput.h
    input/GribFileInput.cc
    input/GribFileInput.h
    input/GribInput.cc
    input/GribInput.h
    input/GribMemoryInput.cc
    input/GribMemoryInput.h
    input/GribStreamInput.cc
    input/GribStreamInput.h
    input/GridSpecInput.cc
    input/GridSpecInput.h
    input/GriddefInput.cc
    input/GriddefInput.h
    input/MIRInput.cc
    input/MIRInput.h
    input/MultiDimensionalGribFileInput.cc
    input/MultiDimensionalGribFileInput.h
    input/MultiDimensionalInput.cc
    input/MultiDimensionalInput.h
    input/MultiStreamInput.cc
    input/MultiStreamInput.h
    input/RawInput.cc
    input/RawInput.h
    input/VectorInput.cc
    input/VectorInput.h
    iterator/UnstructuredIterator.cc
    iterator/UnstructuredIterator.h
    iterator/detail/RegularIterator.cc
    iterator/detail/RegularIterator.h
    key/Area.cc
    key/Area.h
    key/Key.cc
    key/Key.h
    key/grid/ClassicPattern.cc
    key/grid/ClassicPattern.h
    key/grid/Grid.cc
    key/grid/Grid.h
    key/grid/GridPattern.cc
    key/grid/GridPattern.h
    key/grid/GridSpec.cc
    key/grid/GridSpec.h
    key/grid/GridSpecPattern.cc
    key/grid/GridSpecPattern.h
    key/grid/NamedClassic.cc
    key/grid/NamedClassic.h
    key/grid/NamedFromFile.cc
    key/grid/NamedFromFile.h
    key/grid/NamedGrid.h
    key/grid/NamedOctahedral.cc
    key/grid/NamedOctahedral.h
    key/grid/NamedRegular.cc
    key/grid/NamedRegular.h
    key/grid/OctahedralPattern.cc
    key/grid/OctahedralPattern.h
    key/grid/RegularLL.cc
    key/grid/RegularLL.h
    key/grid/RegularLLPattern.cc
    key/grid/RegularLLPattern.h
    key/grid/RegularPattern.cc
    key/grid/RegularPattern.h
    key/grid/TypedGrid.cc
    key/grid/TypedGrid.h
    key/intgrid/Automatic.cc
    key/intgrid/Automatic.h
    key/intgrid/Intgrid.cc
    key/intgrid/Intgrid.h
    key/intgrid/NamedGrid.cc
    key/intgrid/NamedGrid.h
    key/intgrid/None.cc
    key/intgrid/None.h
    key/intgrid/Source.cc
    key/intgrid/Source.h
    key/resol/Resol.cc
    key/resol/Resol.h
    key/style/CustomParametrisation.cc
    key/style/CustomParametrisation.h
    key/style/CustomStyle.cc
    key/style/CustomStyle.h
    key/style/ECMWFStyle.cc
    key/style/ECMWFStyle.h
    key/style/MIRStyle.cc
    key/style/MIRStyle.h
    key/truncation/Automatic.cc
    key/truncation/Automatic.h
    key/truncation/None.cc
    key/truncation/None.h
    key/truncation/Ordinal.cc
    key/truncation/Ordinal.h
    key/truncation/Truncation.cc
    key/truncation/Truncation.h
    lsm/FileLSM.cc
    lsm/FileLSM.h
    lsm/GribFileMask.cc
    lsm/GribFileMask.h
    lsm/GribFileMaskFromMIR.cc
    lsm/GribFileMaskFromMIR.h
    lsm/GribFileMaskFromUser.cc
    lsm/GribFileMaskFromUser.h
    lsm/LSMSelection.cc
    lsm/LSMSelection.h
    lsm/LandSeaMasks.cc
    lsm/LandSeaMasks.h
    lsm/MappedMask.cc
    lsm/MappedMask.h
    lsm/Mask.cc
    lsm/Mask.h
    lsm/NamedLSM.cc
    lsm/NamedLSM.h
    lsm/NoMask.cc
    lsm/NoMask.h
    lsm/NoneLSM.cc
    lsm/NoneLSM.h
    lsm/TenMinutesMask.cc
    lsm/TenMinutesMask.h
    method/Cropping.cc
    method/Cropping.h
    method/FailMethod.cc
    method/FailMethod.h
    method/Matrix.cc
    method/Matrix.h
    method/MatrixCacheCreator.cc
    method/MatrixCacheCreator.h
    method/Method.cc
    method/Method.h
    method/MethodWeighted.cc
    method/MethodWeighted.h
    method/WeightMatrix.cc
    method/WeightMatrix.h
    method/gridbox/GridBoxAverage.cc
    method/gridbox/GridBoxAverage.h
    method/gridbox/GridBoxMethod.cc
    method/gridbox/GridBoxMethod.h
    method/gridbox/GridBoxStatistics.cc
    method/gridbox/GridBoxStatistics.h
    method/knn/KNearest.cc
    method/knn/KNearest.h
    method/knn/KNearestNeighbours.cc
    method/knn/KNearestNeighbours.h
    method/knn/KNearestStatistics.cc
    method/knn/KNearestStatistics.h
    method/knn/NearestLSM.cc
    method/knn/NearestLSM.h
    method/knn/NearestNeighbour.cc
    method/knn/NearestNeighbour.h
    method/knn/distance/ClimateFilter.cc
    method/knn/distance/ClimateFilter.h
    method/knn/distance/Cressman.cc
    method/knn/distance/Cressman.h
    method/knn/distance/DistanceWeighting.cc
    method/knn/distance/DistanceWeighting.h
    method/knn/distance/DistanceWeightingWithLSM.cc
    method/knn/distance/DistanceWeightingWithLSM.h
    method/knn/distance/GaussianDistanceWeighting.cc
    method/knn/distance/GaussianDistanceWeighting.h
    method/knn/distance/InverseDistanceWeighting.cc
    method/knn/distance/InverseDistanceWeighting.h
    method/knn/distance/InverseDistanceWeightingSquared.cc
    method/knn/distance/InverseDistanceWeightingSquared.h
    method/knn/distance/NearestLSM.cc
    method/knn/distance/NearestLSM.h
    method/knn/distance/NearestLSMWithLowestIndex.cc
    method/knn/distance/NearestLSMWithLowestIndex.h
    method/knn/distance/NearestNeighbour.cc
    method/knn/distance/NearestNeighbour.h
    method/knn/distance/NoDistanceWeighting.cc
    method/knn/distance/NoDistanceWeighting.h
    method/knn/distance/PseudoLaplace.cc
    method/knn/distance/PseudoLaplace.h
    method/knn/pick/Distance.cc
    method/knn/pick/Distance.h
    method/knn/pick/DistanceAndNClosest.cc
    method/knn/pick/DistanceAndNClosest.h
    method/knn/pick/DistanceOrNClosest.cc
    method/knn/pick/DistanceOrNClosest.h
    method/knn/pick/LongestElementDiagonalAndNClosest.cc
    method/knn/pick/LongestElementDiagonalAndNClosest.h
    method/knn/pick/NClosest.cc
    method/knn/pick/NClosest.h
    method/knn/pick/NClosestOrNearest.cc
    method/knn/pick/NClosestOrNearest.h
    method/knn/pick/NearestNeighbourWithLowestIndex.cc
    method/knn/pick/NearestNeighbourWithLowestIndex.h
    method/knn/pick/Pick.cc
    method/knn/pick/Pick.h
    method/knn/pick/Sample.cc
    method/knn/pick/Sample.h
    method/knn/pick/SortedSample.cc
    method/knn/pick/SortedSample.h
    method/nonlinear/Heaviest.cc
    method/nonlinear/Heaviest.h
    method/nonlinear/MissingIfAllMissing.cc
    method/nonlinear/MissingIfAllMissing.h
    method/nonlinear/MissingIfAnyMissing.cc
    method/nonlinear/MissingIfAnyMissing.h
    method/nonlinear/MissingIfHeaviestMissing.cc
    method/nonlinear/MissingIfHeaviestMissing.h
    method/nonlinear/NoNonLinear.cc
    method/nonlinear/NoNonLinear.h
    method/nonlinear/NonLinear.cc
    method/nonlinear/NonLinear.h
    method/nonlinear/SimulatedMissingValue.cc
    method/nonlinear/SimulatedMissingValue.h
    method/solver/Multiply.cc
    method/solver/Multiply.h
    method/solver/Solver.h
    method/solver/Statistics.cc
    method/solver/Statistics.h
    method/voronoi/VoronoiMethod.cc
    method/voronoi/VoronoiMethod.h
    method/voronoi/VoronoiStatistics.cc
    method/voronoi/VoronoiStatistics.h
    output/EmptyOutput.cc
    output/EmptyOutput.h
    output/GeoPointsFileOutput.cc
    output/GeoPointsFileOutput.h
    output/GeoPointsFileOutputXYV.cc
    output/GeoPointsFileOutputXYV.h
    output/GeoPointsFileOutputXYVector.cc
    output/GeoPointsFileOutputXYVector.h
    output/GeoPointsOutput.cc
    output/GeoPointsOutput.h
    output/GribFileOutput.cc
    output/GribFileOutput.h
    output/GribMemoryOutput.cc
    output/GribMemoryOutput.h
    output/GribOutput.cc
    output/GribOutput.h
    output/GribStreamOutput.cc
    output/GribStreamOutput.h
    output/GriddefOutput.cc
    output/GriddefOutput.h
    output/MIROutput.cc
    output/MIROutput.h
    output/MultiDimensionalOutput.cc
    output/MultiDimensionalOutput.h
    output/RawOutput.cc
    output/RawOutput.h
    output/ResizableOutput.cc
    output/ResizableOutput.h
    output/ValuesOutput.cc
    output/ValuesOutput.h
    output/VectorOutput.cc
    output/VectorOutput.h
    param/CachedParametrisation.cc
    param/CachedParametrisation.h
    param/CombinedParametrisation.cc
    param/CombinedParametrisation.h
    param/ConfigurationWrapper.cc
    param/ConfigurationWrapper.h
    param/DefaultParametrisation.cc
    param/DefaultParametrisation.h
    param/FieldParametrisation.cc
    param/FieldParametrisation.h
    param/GridSpecParametrisation.cc
    param/GridSpecParametrisation.h
    param/MIRParametrisation.cc
    param/MIRParametrisation.h
    param/Rules.cc
    param/Rules.h
    param/RuntimeParametrisation.cc
    param/RuntimeParametrisation.h
    param/SameParametrisation.cc
    param/SameParametrisation.h
    param/SimpleParametrisation.cc
    param/SimpleParametrisation.h
    repres/Gridded.cc
    repres/Gridded.h
    repres/HEALPix.cc
    repres/HEALPix.h
    repres/Iterator.cc
    repres/Iterator.h
    repres/Representation.cc
    repres/Representation.h
    repres/gauss/Gaussian.cc
    repres/gauss/Gaussian.h
    repres/gauss/GaussianIterator.cc
    repres/gauss/GaussianIterator.h
    repres/gauss/reduced/Classic.cc
    repres/gauss/reduced/Classic.h
    repres/gauss/reduced/FromPL.cc
    repres/gauss/reduced/FromPL.h
    repres/gauss/reduced/Octahedral.cc
    repres/gauss/reduced/Octahedral.h
    repres/gauss/reduced/Reduced.cc
    repres/gauss/reduced/Reduced.h
    repres/gauss/reduced/ReducedClassic.cc
    repres/gauss/reduced/ReducedClassic.h
    repres/gauss/reduced/ReducedFromPL.cc
    repres/gauss/reduced/ReducedFromPL.h
    repres/gauss/reduced/ReducedOctahedral.cc
    repres/gauss/reduced/ReducedOctahedral.h
    repres/gauss/reduced/RotatedClassic.cc
    repres/gauss/reduced/RotatedClassic.h
    repres/gauss/reduced/RotatedFromPL.cc
    repres/gauss/reduced/RotatedFromPL.h
    repres/gauss/reduced/RotatedOctahedral.cc
    repres/gauss/reduced/RotatedOctahedral.h
    repres/gauss/regular/Regular.cc
    repres/gauss/regular/Regular.h
    repres/gauss/regular/RegularGG.cc
    repres/gauss/regular/RegularGG.h
    repres/gauss/regular/RotatedGG.cc
    repres/gauss/regular/RotatedGG.h
    repres/latlon/LatLon.cc
    repres/latlon/LatLon.h
    repres/latlon/ReducedLL.cc
    repres/latlon/ReducedLL.h
    repres/latlon/RegularLL.cc
    repres/latlon/RegularLL.h
    repres/latlon/RotatedLL.cc
    repres/latlon/RotatedLL.h
    repres/other/None.cc
    repres/other/None.h
    repres/other/UnstructuredGrid.cc
    repres/other/UnstructuredGrid.h
    repres/regular/Lambert.cc
    repres/regular/Lambert.h
    repres/regular/LambertAzimuthalEqualArea.cc
    repres/regular/LambertAzimuthalEqualArea.h
    repres/regular/Mercator.cc
    repres/regular/Mercator.h
    repres/regular/PolarStereographic.cc
    repres/regular/PolarStereographic.h
    repres/regular/RegularGrid.cc
    repres/regular/RegularGrid.h
    repres/regular/SpaceView.cc
    repres/regular/SpaceView.h
    repres/sh/SphericalHarmonics.cc
    repres/sh/SphericalHarmonics.h
    repres/unsupported/Albers.cc
    repres/unsupported/Albers.h
    repres/unsupported/AzimuthRange.cc
    repres/unsupported/AzimuthRange.h
    repres/unsupported/EquatorialAzimuthalEquidistant.cc
    repres/unsupported/EquatorialAzimuthalEquidistant.h
    repres/unsupported/IrregularLatlon.cc
    repres/unsupported/IrregularLatlon.h
    repres/unsupported/RotatedSH.cc
    repres/unsupported/RotatedSH.h
    repres/unsupported/StretchedGG.cc
    repres/unsupported/StretchedGG.h
    repres/unsupported/StretchedLL.cc
    repres/unsupported/StretchedLL.h
    repres/unsupported/StretchedRotatedGG.cc
    repres/unsupported/StretchedRotatedGG.h
    repres/unsupported/StretchedRotatedLL.cc
    repres/unsupported/StretchedRotatedLL.h
    repres/unsupported/StretchedRotatedSH.cc
    repres/unsupported/StretchedRotatedSH.h
    repres/unsupported/StretchedSH.cc
    repres/unsupported/StretchedSH.h
    repres/unsupported/TransverseMercator.cc
    repres/unsupported/TransverseMercator.h
    search/PointSearch.cc
    search/PointSearch.h
    search/Tree.cc
    search/Tree.h
    search/tree/TreeMapped.cc
    search/tree/TreeMapped.h
    search/tree/TreeMappedAnonymousMemory.cc
    search/tree/TreeMappedAnonymousMemory.h
    search/tree/TreeMappedFile.cc
    search/tree/TreeMappedFile.h
    search/tree/TreeMemory.cc
    search/tree/TreeMemory.h
    stats/Comparator.cc
    stats/Comparator.h
    stats/Distribution.cc
    stats/Distribution.h
    stats/Field.cc
    stats/Field.h
    stats/Method.cc
    stats/Method.h
    stats/Statistics.cc
    stats/Statistics.h
    stats/comparator/ComparatorT.cc
    stats/comparator/ComparatorT.h
    stats/comparator/MissingValues.cc
    stats/comparator/MissingValues.h
    stats/comparator/Spectral.cc
    stats/comparator/Spectral.h
    stats/detail/AngleT.cc
    stats/detail/AngleT.h
    stats/detail/CentralMomentsT.cc
    stats/detail/CentralMomentsT.h
    stats/detail/Counter.cc
    stats/detail/Counter.h
    stats/detail/CounterBinary.cc
    stats/detail/CounterBinary.h
    stats/detail/ModeT.cc
    stats/detail/ModeT.h
    stats/detail/PNormsT.h
    stats/detail/ScalarT.h
    stats/distribution/DistributionT.cc
    stats/distribution/DistributionT.h
    stats/field/CentralMomentStats.cc
    stats/field/CentralMomentStats.h
    stats/field/CounterStats.cc
    stats/field/CounterStats.h
    stats/field/ModeStats.cc
    stats/field/ModeStats.h
    stats/method/MethodT.cc
    stats/method/MethodT.h
    stats/statistics/GridBoxIntegral.cc
    stats/statistics/GridBoxIntegral.h
    stats/statistics/SimplePackingEntropy.cc
    stats/statistics/SimplePackingEntropy.h
    stats/statistics/Spectral.cc
    stats/statistics/Spectral.h
    stats/statistics/StatisticsT.cc
    stats/statistics/StatisticsT.h
    tools/Count.cc
    tools/Count.h
    tools/MIRTool.cc
    tools/MIRTool.h
    util/Angles.h
    util/Atlas.h
    util/Bitmap.cc
    util/Bitmap.h
    util/BoundingBox.cc
    util/BoundingBox.h
    util/CheckDuplicatePoints.cc
    util/CheckDuplicatePoints.h
    util/DeprecatedFunctionality.cc
    util/DeprecatedFunctionality.h
    util/Domain.cc
    util/Domain.h
    util/Error.cc
    util/Error.h
    util/Exceptions.h
    util/Formula.h
    util/FormulaBinop.cc
    util/FormulaBinop.h
    util/FormulaFunction.cc
    util/FormulaFunction.h
    util/FormulaIdent.cc
    util/FormulaIdent.h
    util/FormulaNumber.cc
    util/FormulaNumber.h
    util/FormulaParser.cc
    util/FormulaParser.h
    util/FormulaString.cc
    util/FormulaString.h
    util/Function.cc
    util/Function.h
    util/GlobaliseUnstructured.cc
    util/GlobaliseUnstructured.h
    util/Grib.cc
    util/Grib.h
    util/GridBox.cc
    util/GridBox.h
    util/Increments.cc
    util/Increments.h
    util/IndexMapping.h
    util/Latitude.cc
    util/Latitude.h
    util/LatitudeIncrement.h
    util/Log.cc
    util/Log.h
    util/LongitudeDouble.cc
    util/LongitudeDouble.h
    util/LongitudeFraction.cc
    util/LongitudeFraction.h
    util/LongitudeIncrement.h
    util/MIRStatistics.cc
    util/MIRStatistics.h
    util/MeshGeneratorParameters.cc
    util/MeshGeneratorParameters.h
    util/Mutex.h
    util/PlanParser.cc
    util/PlanParser.h
    util/Point2ToPoint3.cc
    util/Point2ToPoint3.h
    util/PointLatLonT.h
    util/Regex.cc
    util/Regex.h
    util/Reorder.cc
    util/Reorder.h
    util/Rotation.cc
    util/Rotation.h
    util/Shape.cc
    util/Shape.h
    util/SpectralOrder.cc
    util/SpectralOrder.h
    util/SpectralOrderT.cc
    util/SpectralOrderT.h
    util/Trace.cc
    util/Trace.h
    util/Translator.h
    util/Types.h
    util/ValueMap.cc
    util/ValueMap.h
    util/Wind.cc
    util/Wind.h)

if(mir_HAVE_GRID_FESOM)
    list(APPEND mir_srcs repres/FESOM.cc repres/FESOM.h)
endif()

if(mir_HAVE_GRID_ICON)
    list(APPEND mir_srcs repres/ICON.cc repres/ICON.h)
endif()

if(mir_HAVE_GRID_ORCA)
    list(APPEND mir_srcs repres/ORCA.cc repres/ORCA.h)
endif()

if(mir_HAVE_ATLAS)
    list(APPEND mir_srcs
        action/filter/NablaFilterFVMT.cc
        action/filter/NablaFilterFVMT.h
        action/transform/InvtransScalar.cc
        action/transform/InvtransScalar.h
        action/transform/InvtransVodTouv.cc
        action/transform/InvtransVodTouv.h
        action/transform/ShToGridDef.cc
        action/transform/ShToGridDef.h
        action/transform/ShToGridSpec.cc
        action/transform/ShToGridSpec.h
        action/transform/ShToGridded.cc
        action/transform/ShToGridded.h
        action/transform/ShToNamedGrid.cc
        action/transform/ShToNamedGrid.h
        action/transform/ShToOctahedralGG.cc
        action/transform/ShToOctahedralGG.h
        action/transform/ShToPoints.cc
        action/transform/ShToPoints.h
        action/transform/ShToReducedGG.cc
        action/transform/ShToReducedGG.h
        action/transform/ShToReducedGGPLGiven.cc
        action/transform/ShToReducedGGPLGiven.h
        action/transform/ShToRegularGG.cc
        action/transform/ShToRegularGG.h
        action/transform/ShToRegularLL.cc
        action/transform/ShToRegularLL.h
        action/transform/ShToRotatedNamedGrid.cc
        action/transform/ShToRotatedNamedGrid.h
        action/transform/ShToRotatedOctahedralGG.cc
        action/transform/ShToRotatedOctahedralGG.h
        action/transform/ShToRotatedReducedGG.cc
        action/transform/ShToRotatedReducedGG.h
        action/transform/ShToRotatedReducedGGPLGiven.cc
        action/transform/ShToRotatedReducedGGPLGiven.h
        action/transform/ShToRotatedRegularGG.cc
        action/transform/ShToRotatedRegularGG.h
        action/transform/ShToRotatedRegularLL.cc
        action/transform/ShToRotatedRegularLL.h
        action/transform/ShToTypedGrid.cc
        action/transform/ShToTypedGrid.h
        action/transform/ShVodToUV.cc
        action/transform/ShVodToUV.h
        action/transform/TransCache.cc
        action/transform/TransCache.h
        caching/InMemoryMeshCache.cc
        caching/InMemoryMeshCache.h
        caching/LegendreCache.cc
        caching/LegendreCache.h
        caching/legendre/FileLoader.cc
        caching/legendre/FileLoader.h
        caching/legendre/LegendreLoader.cc
        caching/legendre/LegendreLoader.h
        caching/legendre/MappedMemoryLoader.cc
        caching/legendre/MappedMemoryLoader.h
        caching/legendre/NoLoader.cc
        caching/legendre/NoLoader.h
        caching/legendre/SharedMemoryLoader.cc
        caching/legendre/SharedMemoryLoader.h
        key/grid/SourcePattern.cc
        key/grid/SourcePattern.h
        method/ProxyMethod.cc
        method/ProxyMethod.h
        method/fe/CalculateCellLongestDiagonal.cc
        method/fe/CalculateCellLongestDiagonal.h
        method/fe/FEBilinear.cc
        method/fe/FEBilinear.h
        method/fe/FELinear.cc
        method/fe/FELinear.h
        method/fe/FiniteElement.cc
        method/fe/FiniteElement.h
        output/GmshOutput.cc
        output/GmshOutput.h
        stats/statistics/Integral.cc
        stats/statistics/Integral.h)
    if(mir_HAVE_PNG)
        list(APPEND mir_srcs output/PNGOutput.h output/PNGOutput.cc)
    endif()
else()
    list(APPEND mir_srcs util/Atlas.cc)
endif()


if(mir_HAVE_NETCDF)
    list(APPEND mir_srcs
        input/NetcdfFileInput.cc
        input/NetcdfFileInput.h
        netcdf/Attribute.cc
        netcdf/Attribute.h
        netcdf/CellMethodInputVariable.cc
        netcdf/CellMethodInputVariable.h
        netcdf/CellMethodOutputVariable.cc
        netcdf/CellMethodOutputVariable.h
        netcdf/Codec.cc
        netcdf/Codec.h
        netcdf/CoordinateInputVariable.cc
        netcdf/CoordinateInputVariable.h
        netcdf/CoordinateOutputVariable.cc
        netcdf/CoordinateOutputVariable.h
        netcdf/DataInputVariable.cc
        netcdf/DataInputVariable.h
        netcdf/DataOutputVariable.cc
        netcdf/DataOutputVariable.h
        netcdf/Dimension.cc
        netcdf/Dimension.h
        netcdf/DummyInputVariable.cc
        netcdf/DummyInputVariable.h
        netcdf/DummyMatrix.cc
        netcdf/DummyMatrix.h
        netcdf/DummyVariable.h
        netcdf/Endowed.cc
        netcdf/Endowed.h
        netcdf/Exceptions.cc
        netcdf/Exceptions.h
        netcdf/Dataset.cc
        netcdf/Dataset.h
        netcdf/GregorianCalendar.cc
        netcdf/GregorianCalendar.h
        netcdf/JulianCalendar.cc
        netcdf/JulianCalendar.h
        netcdf/AllLeapCalendar.cc
        netcdf/AllLeapCalendar.h
        netcdf/NoLeapCalendar.cc
        netcdf/NoLeapCalendar.h
        netcdf/NoneCalendar.cc
        netcdf/NoneCalendar.h
        netcdf/ProlepticCalendar.cc
        netcdf/ProlepticCalendar.h
        netcdf/ThreeSixtyCalendar.cc
        netcdf/ThreeSixtyCalendar.h
        netcdf/Calendar.cc
        netcdf/Calendar.h
        netcdf/PackingCodec.cc
        netcdf/PackingCodec.h
        netcdf/HyperCube.cc
        netcdf/HyperCube.h
        netcdf/InputAttribute.cc
        netcdf/InputAttribute.h
        netcdf/InputDimension.cc
        netcdf/InputDimension.h
        netcdf/InputDataset.cc
        netcdf/InputDataset.h
        netcdf/InputMatrix.cc
        netcdf/InputMatrix.h
        netcdf/InputVariable.cc
        netcdf/InputVariable.h
        netcdf/Mapper.h
        netcdf/Matrix.cc
        netcdf/Matrix.h
        netcdf/MergeCoordinateMatrix.cc
        netcdf/MergeCoordinateMatrix.h
        netcdf/MergeCoordinateStep.cc
        netcdf/MergeCoordinateStep.h
        netcdf/MergeDataMatrix.cc
        netcdf/MergeDataMatrix.h
        netcdf/MergeDataStep.cc
        netcdf/MergeDataStep.h
        netcdf/MergePlan.cc
        netcdf/MergePlan.h
        netcdf/NCFile.cc
        netcdf/NCFile.h
        netcdf/NCFileCache.cc
        netcdf/NCFileCache.h
        netcdf/OutputAttribute.cc
        netcdf/OutputAttribute.h
        netcdf/OutputDimension.cc
        netcdf/OutputDimension.h
        netcdf/OutputDataset.cc
        netcdf/OutputDataset.h
        netcdf/OutputVariable.cc
        netcdf/OutputVariable.h
        netcdf/Remapping.cc
        netcdf/Remapping.h
        netcdf/Reshape.cc
        netcdf/Reshape.h
        netcdf/GridSpec.h
        netcdf/GridSpec.cc
        netcdf/RegularLL.h
        netcdf/RegularLL.cc
        netcdf/Rectilinear.h
        netcdf/Rectilinear.cc
        netcdf/Curvilinear.h
        netcdf/Curvilinear.cc
        netcdf/Field.h
        netcdf/Field.cc
        netcdf/ReshapeVariableStep.cc
        netcdf/ReshapeVariableStep.h
        netcdf/ScalarCoordinateInputVariable.cc
        netcdf/ScalarCoordinateInputVariable.h
        netcdf/ScalarCoordinateOutputVariable.cc
        netcdf/ScalarCoordinateOutputVariable.h
        netcdf/SimpleInputVariable.cc
        netcdf/SimpleInputVariable.h
        netcdf/SimpleOutputVariable.cc
        netcdf/SimpleOutputVariable.h
        netcdf/Step.cc
        netcdf/Step.h
        netcdf/Type.cc
        netcdf/Type.h
        netcdf/Curvilinear.cc
        netcdf/Curvilinear.h
        netcdf/UnstructuredGrid.cc
        netcdf/UnstructuredGrid.h
        netcdf/UpdateCoordinateStep.cc
        netcdf/UpdateCoordinateStep.h
        netcdf/Value.cc
        netcdf/Value.h
        netcdf/ValueT.cc
        netcdf/ValueT.h
        netcdf/Variable.cc
        netcdf/Variable.h
        netcdf/VirtualInputDimension.cc
        netcdf/VirtualInputDimension.h
        netcdf/VirtualOutputDimension.cc
        netcdf/VirtualOutputDimension.h)
endif()

ecbuild_add_library(
    TARGET             mir
    INSTALL_HEADERS    LISTED
    HEADER_DESTINATION ${INSTALL_INCLUDE_DIR}/mir
    TEMPLATES          caching/InMemoryCache.cc
    PUBLIC_LIBS        eckit_linalg eckit_geometry eckit_option eckit eckit_geo eccodes
    PUBLIC_INCLUDES    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src> $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    SOURCES            ${mir_srcs})

if(mir_HAVE_ATLAS)
    target_link_libraries(mir PUBLIC atlas)
endif()

if(mir_HAVE_NETCDF)
    target_link_libraries(mir PRIVATE NetCDF::NetCDF_C)
endif()

if(mir_HAVE_PNG)
    target_link_libraries(mir PRIVATE PNG::PNG)
endif()

if (HAS_SYS_STAT_ST_CTIMESPEC)
    set_source_files_properties(
        caching/SharedMemoryKey.cc
        PROPERTIES
        COMPILE_DEFINITIONS HAS_SYS_STAT_ST_CTIMESPEC)
endif()
