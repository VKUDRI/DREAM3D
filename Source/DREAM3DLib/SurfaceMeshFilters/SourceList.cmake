#--////////////////////////////////////////////////////////////////////////////
#-- Your License or copyright can go here
#--////////////////////////////////////////////////////////////////////////////
set(FILTER_CATEGORY "SurfaceMeshFilters")


# --------------------------------------------------------------------
# If you have more than a single filter add it to this list (headers)
SET (${FILTER_CATEGORY}_FILTERS_HDRS
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/CMUSurfaceMeshingFilter.h
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/CMUNodesTrianglesToVtk.h
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/CMUNodesTrianglesToStl.h
)

# --------------------------------------------------------------------
# If you have more than a single filter add it to this list (sources)
SET (DREAM3DLib_${FILTER_CATEGORY}_SRCS
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/CMUSurfaceMeshingFilter.cpp
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/CMUNodesTrianglesToVtk.cpp
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/CMUNodesTrianglesToStl.cpp
)

#-- Add in any additional NON Filter classes or filters that will NOT show up in the GUI filter list
SET (DREAM3DLib_${FILTER_CATEGORY}_HDRS
    ${${FILTER_CATEGORY}_FILTERS_HDRS}
#    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/SurfaceMeshGeneralMCABoundedMulti.cpp
#    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/SurfaceMeshGeneralMCABoundedMulti.h
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/SurfaceMeshStructs.h
    ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/StructArray.hpp
)

#set_source_files_properties( ${DREAM3DLib_SOURCE_DIR}/SurfaceMeshFilters/SurfaceMeshGeneralMCABoundedMulti.cpp
#                                 PROPERTIES HEADER_FILE_ONLY TRUE)

if (NOT DEFINED ONLY_FILTERS)
cmp_IDE_SOURCE_PROPERTIES( "DREAM3DLib/${FILTER_CATEGORY}" "${DREAM3DLib_${FILTER_CATEGORY}_HDRS}" "${DREAM3DLib_${FILTER_CATEGORY}_SRCS}" "0")
endif()

if ( ${PROJECT_INSTALL_HEADERS} EQUAL 1 )
    INSTALL (FILES ${DREAM3DLib_${FILTER_CATEGORY}_HDRS}
            DESTINATION include/DREAM3D/${FILTER_CATEGORY}
            COMPONENT Headers   )
endif()



