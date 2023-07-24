cmake_minimum_required(VERSION 3.16)

project(dbTest VERSION 1.0.0)

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

target_precompile_headers(dbTest PRIVATE ../StdAfx.h)

add_compile_definitions(TEST)

target_include_directories(dbTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(dbTest PRIVATE Blue msoledbsql version)

target_compile_definitions(dbTest PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)

set_target_properties(dbTest PROPERTIES OUTPUT_NAME "_dbTest")
if(WIN32)
    set(PYTHON_EXECUTABLE "${BRANCH_ROOT_DIR}/eve/server/pythonInterpreter.bat")
    execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import sys;import unittest; print_suite = lambda x: (print(x._exception), sys.exit(1)) if hasattr(x, '_exception') else print(x.id()) if not hasattr(x, '__iter__') else [print_suite(x) for x in x];print_suite(unittest.defaultTestLoader.discover('.'))"
        OUTPUT_VARIABLE PYTHON_TESTS_STR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE PYTHON_TEST_DISCOVERY_RETVAL
        ERROR_VARIABLE PYTHON_TEST_DISCOVERY_ERRORS
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
        )
    if(${PYTHON_TEST_DISCOVERY_RETVAL})
        message(FATAL_ERROR "Error encountered during test discovery:\n=== stderr: ===\n${PYTHON_TEST_DISCOVERY_ERRORS}\n=== stdout: ===\n${PYTHON_TESTS_STR}")
    endif()
    separate_arguments(PYTHON_TESTS UNIX_COMMAND ${PYTHON_TESTS_STR})
    foreach(PYTHON_TEST ${PYTHON_TESTS})
        add_test(NAME ${PYTHON_TEST}
            COMMAND ${PYTHON_EXECUTABLE} -m unittest -v ${PYTHON_TEST}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
        )
        set_tests_properties(${PYTHON_TEST} PROPERTIES
            ENVIRONMENT "PYTHONPATH=${BRANCH_ROOT_DIR}/carbon/common/stdlib\\\;$<TARGET_FILE_DIR:dbTest>"
        )
    endforeach()
endif()