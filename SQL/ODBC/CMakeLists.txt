# Sources
file(GLOB SRCS_G "src/*.cpp")
POCO_SOURCES_AUTO( ODBC_SRCS ${SRCS_G})

# Headers
file(GLOB_RECURSE HDRS_G "include/*.h" )
POCO_HEADERS_AUTO( ODBC_SRCS ${HDRS_G})

# Version Resource
if(MSVC AND NOT POCO_STATIC)
    source_group("Resources" FILES ${CMAKE_SOURCE_DIR}/DLLVersion.rc)
    list(APPEND ODBC_SRCS ${CMAKE_SOURCE_DIR}/DLLVersion.rc)
endif()

add_library(SQLODBC ${ODBC_SRCS} )
add_library(Poco::SQLODBC ALIAS SQLODBC)
set_target_properties(SQLODBC
    PROPERTIES
    VERSION ${SHARED_LIBRARY_VERSION} SOVERSION ${SHARED_LIBRARY_VERSION}
	OUTPUT_NAME PocoSQLODBC
    DEFINE_SYMBOL ODBC_EXPORTS
    )

target_link_libraries(SQLODBC PUBLIC Poco::SQL ODBC::ODBC)
target_include_directories(SQLODBC
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
target_compile_definitions(SQLODBC PUBLIC THREADSAFE)

POCO_INSTALL(SQLODBC)
POCO_GENERATE_PACKAGE(SQLODBC)

if (POCO_ENABLE_TESTS)
    add_subdirectory(testsuite)
endif ()
