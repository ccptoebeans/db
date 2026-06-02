# Copyright © 2021 CCP ehf.
cmake_minimum_required(VERSION 3.16)
project(db VERSION 1.0.0)

if (NOT WIN32)
    message(FATAL_ERROR "carbon-db only builds on windows")
endif()

include(cmake/CcpGlobalSettings.cmake)
include(cmake/CcpTargetConfigurations.cmake)
include(cmake/CcpPackageConfigHelpers.cmake)

ccp_add_library(db SHARED
        BlockAllocator.cpp
        Connection.cpp
        Connection_Blue.cpp
        ColumnDescriptor.cpp
        db.cpp
        DelayedException.cpp
        NSession.cpp
        PythonBuff.cpp
        Row.cpp
        RowDescriptor.cpp
        SessionPool.cpp
        StdAfx.cpp
        StringStore.cpp
        SqlCommand.cpp
        TaskletBlockingIO.cpp
        TmpRowset.cpp
        Utils.cpp
)
target_sources(db PRIVATE
        Accessor.h
        autoversion.h
        Connection.h
        ColumnDescriptor.h
        DelayedException.h
        NSession.h
        OurAccessor.h
        PythonBuff.h
        resource.h
        Row.h
        RowDescriptor.h
        SessionPool.h
        StdAfx.h
        StringStore.h
        SqlCommand.h
        TaskletBlockingIO.h
        TmpRowset.h
        Utils.h
        version.h
        PyTemplates.h
)

target_precompile_headers(db PRIVATE StdAfx.h)

find_package(carbon-blue CONFIG REQUIRED)
find_package(libuv CONFIG REQUIRED)
find_package(msoledbsql CONFIG REQUIRED)
find_package(carbon-scheduler CONFIG REQUIRED)
find_package(carbon-io CONFIG REQUIRED)

# We don't link against carbonio. We just need "socketmodule.h"
# to import the _socket module C API.
get_target_property(CARBONIO_INCLUDE_DIR _socket INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(SCHEDULER_INCLUDE_DIR Scheduler INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(db PRIVATE ${CARBONIO_INCLUDE_DIR} ${SCHEDULER_INCLUDE_DIR})

target_link_libraries(db PRIVATE Blue libuv::uv msoledbsql version)

target_compile_definitions(db PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
set_target_properties(db PROPERTIES OUTPUT_NAME "_db")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTING "Build tests. Enabled by default." ON)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(INSTALL_TO_MONOLITH "Install this package to a perforce branch vendor folder - provided for backwards compatability" OFF)
    if (INSTALL_TO_MONOLITH)
        # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
        install(
            TARGETS db
            EXPORT dbTargets
            CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
            LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
            ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
            RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
        # Install rule for available public headers
        install(FILES ${PUBLIC_HEADER_FILES} DESTINATION include)

        message(STATUS "lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/")

        configure_ccp_vendor_config_file(
            TARGET db
            DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dbConfig.cmake"
            DEPENDENCIES ""
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbConfig.cmake DESTINATION .)
    else()
        # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
        install(
                TARGETS db
                EXPORT dbTargets
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin
        )
        # Install rule for available public headers
        install(FILES ${PUBLIC_HEADER_FILES} DESTINATION include)

        install(EXPORT dbTargets
                DESTINATION share/carbon-db
                FILE carbon-dbConfig.cmake
        )
    endif()
endif()
