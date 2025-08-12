if(NOT TARGET Destiny)
    find_dependency(BlueExposure REQUIRED CONFIG NO_CMAKE_PATH)
    set(_IMPORT_PREFIX ${CMAKE_CURRENT_LIST_DIR})
    add_library(Destiny INTERFACE IMPORTED)
    set_target_properties(Destiny PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
        INTERFACE_LINK_LIBRARIES "BlueExposure"
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
    )
endif()
