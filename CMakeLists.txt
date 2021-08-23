add_library(db SHARED
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
)

target_precompile_headers(db PRIVATE StdAfx.h)

target_link_libraries(db PRIVATE Blue MicrosoftSQLServer version)

target_include_directories(db
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(db PRIVATE _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
set_target_properties(db PROPERTIES OUTPUT_NAME "_db")
