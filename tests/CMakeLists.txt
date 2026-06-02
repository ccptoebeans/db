# Copyright © 2023 CCP ehf.
cmake_minimum_required(VERSION 3.16)

project(dbTest VERSION 1.0.0)

find_package(carbon-exefile-interpreter REQUIRED CONFIG)

set(dbTest_sources
    dbTest.cpp
    TestTools.cpp
    TestTools_Blue.cpp
    MockSqlCommand.cpp
    MockSqlCommand.h
    MockAccessor.cpp
    MockAccessor.h
    MockNSession.cpp
    MockNSession.h
    TestTools.h
)

set(db_sources
    ../Accessor.h
    ../BlockAllocator.cpp
    ../Connection.cpp
    ../Connection.h
    ../Connection_Blue.cpp
    ../ColumnDescriptor.cpp
    ../ColumnDescriptor.h
    ../DelayedException.cpp
    ../DelayedException.h
    ../NSession.cpp
    ../NSession.h
    ../OurAccessor.h
    ../PythonBuff.cpp
    ../PythonBuff.h
    ../PyTemplates.h
    ../Row.cpp
    ../Row.h
    ../RowDescriptor.cpp
    ../RowDescriptor.h
    ../SessionPool.cpp
    ../SessionPool.h
    ../StdAfx.cpp
    ../StdAfx.h
    ../StringStore.cpp
    ../StringStore.h
    ../SqlCommand.cpp
    ../SqlCommand.h
    ../TaskletBlockingIO.cpp
    ../TaskletBlockingIO.h
    ../TmpRowset.cpp
    ../TmpRowset.h
    ../Utils.cpp
    ../Utils.h
)

ccp_add_library(dbTest SHARED)

target_sources(dbTest PRIVATE ${db_sources})
target_sources(dbTest PRIVATE ${dbTest_sources})
        
source_group(db FILES ${db_sources})
        

target_include_directories(dbTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(dbTest PRIVATE ${CARBONIO_INCLUDE_DIR})

target_precompile_headers(dbTest PRIVATE ../StdAfx.h)

add_compile_definitions(TEST)

get_target_property(SCHEDULER_INCLUDE_DIR Scheduler INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(dbTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/.. ${SCHEDULER_INCLUDE_DIR})

target_link_libraries(dbTest PRIVATE Blue $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv> msoledbsql version)

target_compile_definitions(dbTest PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)

set_target_properties(dbTest PROPERTIES OUTPUT_NAME "_dbTest")

ccp_prepare_interpreter(
        TARGET dbTest
        BUILD_TYPE ${CMAKE_BUILD_TYPE}
        COMMAND_OUT PYTHON_INTERPRETER
        PYTHONPATH_OUT PYTHONPATH
        RUN_DIR $<TARGET_FILE_DIR:dbTest>
        BLUE_DIR_OUT BLUE_LOCATION_DIR
)

add_custom_command(
        TARGET dbTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:dbTest> $<TARGET_FILE_DIR:dbTest>
        COMMAND_EXPAND_LISTS
)

# Discover and run Python tests as a single CTest test.
set(PYTHON_INTERPRETER_PATH "${BRANCH_ROOT_DIR}/eve/client/pythonInterpreter.bat")
list(APPEND PYTHON_TESTS test_db.py)
list(APPEND PYTHON_TESTS test_parameters.py)
foreach(PYTHON_TEST ${PYTHON_TESTS})
    add_test(NAME ${PYTHON_TEST}
            COMMAND ${PYTHON_INTERPRETER} /inherit /buildflavor=$<LOWER_CASE:$<CONFIG>> /py ${PYTHON_TEST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/dbTest
    )
    set_tests_properties(${PYTHON_TEST} PROPERTIES ENVIRONMENT "PYTHONPATH=${PYTHONPATH}\;$<TARGET_FILE_DIR:db>\;${CMAKE_SOURCE_DIR}/python;BUILDFLAVOR=$<LOWER_CASE:$<CONFIG>>")
endforeach()

