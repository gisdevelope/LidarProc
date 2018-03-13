cmake_minimum_required(VERSION 3.7)
project(LidarProc)

find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 11)
set(Geometry_FILES Geometry.h GeometryAlgorithm.cpp GeometryAlgorithm.h)
set(UTILIS_FILES RTree.hpp tsmToUTM.cpp tsmToUTM.h)
set(LASPROC_FILES LASColorMap.cpp LASColorMap.h LASDangerPoints.cpp LASDangerPoints.h LASHeader.cpp LASHeader.h LASModel.cpp LASModel.h LASPoint.cpp LASPoint.h LASReader.cpp LASReader.h LASSimpleClassify.cpp LASSimpleClassify.h LASVectorClassify.cpp LASVectorClassify.h LASFormatTransform.cpp LASFormatTransform.h)
set(SOURCE_FILES LidarProc.cpp ${Geometry_FILES} ${UTILIS_FILES} ${LASPROC_FILES})

add_executable(LidarProc ${SOURCE_FILES})

target_link_libraries(LidarProc ${GDAL_LIBRARIES})