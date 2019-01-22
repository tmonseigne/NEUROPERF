#######################################################################
# Software License Agreement (AGPL-3 License)
#
# OpenViBE SDK Test Software
# Based on OpenViBE V1.1.0, Copyright (C) Inria, 2006-2015
# Copyright (C) Inria, 2015-2017,V1.0
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License version 3,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.
# If not, see <http://www.gnu.org/licenses/>.
#######################################################################
PROJECT(test_NEUROPERF)

ADD_EXECUTABLE(${PROJECT_NAME} test_NEUROPERF.cpp
    #Add you src files
	#../src/myAlgorithm.cpp
	)

SET_BUILD_PLATFORM()

#INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../src/boxes")

SET(GTEST_ROOT ${OV_DEPENDENCY_CMD_PATH}/${CMAKE_FIND_LIBRARY_PREFIXES}gtest)
FIND_PACKAGE(GTest REQUIRED)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES})
INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIRS})

INCLUDE("FindOpenViBE")
INCLUDE("FindOpenViBECommon")

ADD_TEST(NAME test_template COMMAND ${PROJECT_NAME})

OV_INSTALL_LAUNCH_SCRIPT(SCRIPT_PREFIX "${PROJECT_NAME}" EXECUTABLE_NAME  "${PROJECT_NAME}")
INSTALL(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${DIST_BINDIR}
	LIBRARY DESTINATION ${DIST_LIBDIR}
	ARCHIVE DESTINATION ${DIST_LIBDIR})
