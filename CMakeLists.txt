cmake_minimum_required(VERSION 3.16)
project(db VERSION 1.0.0)

if(WIN32)
    if(NOT DEFINED ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
        message(FATAL_ERROR "Missing required environment variable CCP_EVE_PERFORCE_BRANCH_PATH")
    elseif(NOT IS_ABSOLUTE $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
        message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be an absolute path")
    elseif(NOT EXISTS $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
        message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH is pointing at a non-existing location $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
    elseif(NOT IS_DIRECTORY $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
        message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be a directory")
    else()
        message(STATUS "Using CCP_EVE_PERFORCE_BRANCH_PATH located at $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
    endif()

    file(TO_CMAKE_PATH "$ENV{CCP_EVE_PERFORCE_BRANCH_PATH}" BRANCH_ROOT_DIR)
    list(INSERT CMAKE_MODULE_PATH 0 ${BRANCH_ROOT_DIR}/cmake)
    include(CcpGlobalSettings)
    include(CcpPackageConfigHelpers)

    ccp_add_library(db SHARED
            BlockAllocator.cpp
            Connection.cpp
            Connection_Blue.cpp
            db.cpp
            DelayedException.cpp
            NSession.cpp
            SessionPool.cpp
            StdAfx.cpp
            StringStore.cpp
            TmpRowset.cpp
            Utils.cpp
    )
    target_sources(db PRIVATE
            autoversion.h
            Connection.h
            DelayedException.h
            NSession.h
            resource.h
            SessionPool.h
            StdAfx.h
            StringStore.h
            TmpRowset.h
            Utils.h
            version.h
            PyTemplates.h
    )

    target_precompile_headers(db PRIVATE StdAfx.h)

    find_package(Blue REQUIRED NO_CMAKE_PATH)
    find_package(msoledbsql REQUIRED NO_CMAKE_PATH)

    target_link_libraries(db PRIVATE Blue msoledbsql version)

    target_compile_definitions(db PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
    set_target_properties(db PROPERTIES OUTPUT_NAME "_db")

    # Provide an install target iff this is the top level project only
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
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

    endif()
else()
    message(FATAL_ERROR "db does not support platform specified.")
endif()