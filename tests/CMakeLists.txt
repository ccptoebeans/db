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

target_link_libraries(dbTest PRIVATE Scheduler Blue msoledbsql version)

target_compile_definitions(dbTest PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)

set_target_properties(dbTest PROPERTIES OUTPUT_NAME "_dbTest")
if(WIN32)

    # Discover and run Python tests as a single CTest test.
    set(PYTHON_INTERPRETER_PATH "${BRANCH_ROOT_DIR}/eve/client/pythonInterpreter.bat")
    set(PYTHON_TEST PythonTests)
    add_test(NAME ${PYTHON_TEST}
        COMMAND ${PYTHON_INTERPRETER_PATH} -m unittest discover -v
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
        COMMAND_EXPAND_LISTS
    )
    set_tests_properties(${PYTHON_TEST} PROPERTIES ENVIRONMENT "PYTHONPATH=$<TARGET_FILE_DIR:dbTest>\;$<TARGET_FILE_DIR:db>;BUILDFLAVOR=$<LOWER_CASE:$<CONFIG>>")

endif()