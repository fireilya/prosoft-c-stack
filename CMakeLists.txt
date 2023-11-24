cmake_minimum_required(VERSION 3.22)
project(cstack)
enable_testing()

option(WITH_TEST "Build test (GTest library required)" OFF)

# build library
add_library(cstack STATIC cstack.c)

# build test
if (WITH_TEST)
    enable_testing()
    add_executable(cstack_test test.cpp)
    target_link_libraries(cstack_test cstack)
    add_test(NAME cstack_test COMMAND cstack_test)

    #if(ASAN_ENABLED)
       # string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      #  message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}\n")
     #   add_compile_options(-fsanitize=address)
    #endif()
    # link with googletest
    #find_package(GTest)
    #if (NOT ${GTest_FOUND})
        
    #endif()
    include(cmake/gtest.cmake)
    target_link_libraries(cstack_test
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
    )
endif()

# compiler-specific project settings
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_FLAGS /WX)
    target_compile_options(cstack PRIVATE ${MSVC_FLAGS})
    target_link_options(cstack PRIVATE ${MSVC_FLAGS})
    if (WITH_TEST)
        #target_compile_options(cstack_test PRIVATE /fsanitize=address)
        target_compile_definitions(cstack_test PRIVATE _DISABLE_VECTOR_ANNOTATION)
    endif()
else()
    target_compile_options(cstack PRIVATE -Wall -Wextra -Wformat -Wformat-security -Werror)
    set(SANITIZERS_FLAGS -fsanitize=undefined -fsanitize=address)
    if (WITH_TEST)
        target_compile_options(cstack_test PRIVATE ${SANITIZERS_FLAGS})
        target_link_options(cstack_test PRIVATE ${SANITIZERS_FLAGS})
    endif()
endif()

