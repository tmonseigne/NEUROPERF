PROJECT(openvibe-plugins-NEUROPERF)

SET(PROJECT_VERSION_MAJOR ${OV_GLOBAL_VERSION_MAJOR})
SET(PROJECT_VERSION_MINOR ${OV_GLOBAL_VERSION_MINOR})
SET(PROJECT_VERSION_PATCH ${OV_GLOBAL_VERSION_PATCH})
SET(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

FILE(GLOB_RECURSE source_files src/*.cpp src/*.hpp src/*.h src/*.inl src/*.c)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${source_files}
# "../../../contrib/packages/wavelet2d/wavelet2s.h" # If You need Some files, you can add it here and
)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
VERSION ${PROJECT_VERSION}
SOVERSION ${PROJECT_VERSION_MAJOR}
COMPILE_FLAGS "-DOVP_Exports -DOVP_Shared -D_LARGEFILE64_SOURCE -D_LARGEFILE_SOURCE")

#INCLUDE_DIRECTORIES("../../../contrib/packages/wavelet2d") # If You need Some directories, you can add it here and

# ---------------------------------
# OpenVibe Modules (uncomment usefull package)
# ---------------------------------
# OpenViBE Base
INCLUDE("FindOpenViBE")
INCLUDE("FindOpenViBECommon")
#INCLUDE("FindOpenViBEToolkit")

# OpenViBE Module
#INCLUDE("FindOpenViBEModuleSystem")
#INCLUDE("FindOpenViBEModuleXML")
#INCLUDE("FindOpenViBEModuleEBML")
#INCLUDE("FindOpenViBEModuleCSV")
#INCLUDE("FindOpenViBEModuleSocket")
#INCLUDE("FindOpenViBEModuleCommunication")
#INCLUDE("FindOpenViBEModuleFS")

# OpenViBE Third Party
#INCLUDE("FindThirdPartyEigen")
#INCLUDE("FindThirdPartyITPP")
#INCLUDE("FindThirdPartyFFTW3")
#INCLUDE("FindThirdPartyBoost")
#INCLUDE("FindThirdPartyBoost_FileSystem")
#INCLUDE("FindThirdPartyBoost_Chrono")
#INCLUDE("FindThirdPartyExpat")
#INCLUDE("FindThirdPartyXerces")
#INCLUDE("FindThirdPartyWinsock2")
#INCLUDE("FindThirdPartyFTDI")
#INCLUDE("FindThirdPartyGTK")

# ---------------------------------
# Target macros
# Defines target operating system
# Defines target architecture
# Defines target compiler
# ---------------------------------
SET_BUILD_PLATFORM()

# -----------------------------
# Install files (uncomment usefull line)
# -----------------------------
INSTALL(TARGETS ${PROJECT_NAME}
RUNTIME DESTINATION ${DIST_BINDIR}
LIBRARY DESTINATION ${DIST_LIBDIR}
ARCHIVE DESTINATION ${DIST_LIBDIR})

INSTALL(DIRECTORY bci-tutorials/ DESTINATION ${DIST_DATADIR}/openvibe/scenarios/bci-tutorials/)
INSTALL(DIRECTORY box-tutorials/ DESTINATION ${DIST_DATADIR}/openvibe/scenarios/box-tutorials/)
INSTALL(DIRECTORY signals/ DESTINATION ${DIST_DATADIR}/openvibe/scenarios/signals/)
INSTALL(DIRECTORY metaboxes/ DESTINATION ${DIST_DATADIR}/openvibe/metaboxes/)

# ---------------------------------
# Test applications (uncomment to enable your test directory)
# ---------------------------------
IF(OV_COMPILE_TESTS)
	ADD_SUBDIRECTORY(test)
ENDIF(OV_COMPILE_TESTS)