# FindPoco.cmake

# 查找Poco库的头文件和库文件
find_path(Poco_INCLUDE_DIR
    NAMES Poco/Poco.h
    PATHS
    ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/include
    /usr/include
    /usr/local/include
)

# 查找Foundation库
find_library(Poco_Foundation_LIBRARY
    NAMES PocoFoundation
    PATHS
    ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib
    /usr/lib
    /usr/local/lib
)

# 查找Net库
find_library(Poco_Net_LIBRARY
    NAMES PocoNet
    PATHS
    ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib
    /usr/lib
    /usr/local/lib
)

# 查找JSON库
find_library(Poco_JSON_LIBRARY
    NAMES PocoJSON
    PATHS
        ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib
        /usr/lib
        /usr/local/lib
)

# 设置Poco_FOUND
if(Poco_INCLUDE_DIR AND Poco_Foundation_LIBRARY AND Poco_Net_LIBRARY AND Poco_JSON_LIBRARY)
    set(Poco_FOUND TRUE)
    set(Poco_INCLUDE_DIRS ${Poco_INCLUDE_DIR})
    set(Poco_LIBRARIES 
        ${Poco_Foundation_LIBRARY} 
        ${Poco_Net_LIBRARY}
        ${Poco_JSON_LIBRARY}
    )
    
    # 创建导入目标
    if(NOT TARGET Poco::Foundation)
        add_library(Poco::Foundation UNKNOWN IMPORTED)
        set_target_properties(Poco::Foundation PROPERTIES
            IMPORTED_LOCATION "${Poco_Foundation_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Poco_INCLUDE_DIR}"
        )
    endif()
    
    if(NOT TARGET Poco::Net)
        add_library(Poco::Net UNKNOWN IMPORTED)
        set_target_properties(Poco::Net PROPERTIES
            IMPORTED_LOCATION "${Poco_Net_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Poco_INCLUDE_DIR}"
        )
    endif()
    
    if(NOT TARGET Poco::JSON)
        add_library(Poco::JSON UNKNOWN IMPORTED)
        set_target_properties(Poco::JSON PROPERTIES
            IMPORTED_LOCATION "${Poco_JSON_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Poco_INCLUDE_DIR}"
        )
    endif()
    
    message(STATUS "Found Poco:")
    message(STATUS "  Include path: ${Poco_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${Poco_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find Poco")
endif()

mark_as_advanced(
    Poco_INCLUDE_DIR
    Poco_Foundation_LIBRARY
    Poco_Net_LIBRARY
    Poco_JSON_LIBRARY
) 