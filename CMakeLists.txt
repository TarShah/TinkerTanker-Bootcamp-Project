cmake_minimum_required(VERSION 3.28)
project(
  miningbots
  VERSION 0.0.1
  LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=address -lssl -lcrypto")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3")

set(BASE_SOURCES)
set(CLIENT_SOURCES src/client/connection.cc)

set(TEST_SOURCES test/test_1.cc)

add_executable(mb-client src/client_main.cc ${BASE_SOURCES} ${CLIENT_SOURCES})

add_executable(tests test/run_tests.cc ${BASE_SOURCES} ${TEST_SOURCES})

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG tags/v1.14.1
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG tags/v1.14.0
  GIT_SHALLOW TRUE)
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  glaze
  GIT_REPOSITORY https://github.com/stephenberry/glaze.git
  GIT_TAG v2.6.8
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glaze)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.10.5
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cpr)

set(USE_TLS TRUE)
FetchContent_Declare(
  ixwebsocket
  GIT_REPOSITORY https://github.com/machinezone/IXWebSocket
  GIT_TAG v11.4.5
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(ixwebsocket)

FetchContent_Declare(
  cpptrace
  GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
  GIT_TAG v0.6.1)
FetchContent_MakeAvailable(cpptrace)

find_package(OpenSSL REQUIRED)

include(CTest)
include(GoogleTest)
enable_testing()

set(BASE_LIBRARIES spdlog GTest::gtest_main glaze::glaze cpptrace::cpptrace)

set(CLIENT_LIBRARIES ixwebsocket cpr)

target_include_directories(mb-client PRIVATE external)
target_include_directories(mb-client PRIVATE include)
target_link_libraries(mb-client PRIVATE ${BASE_LIBRARIES} ${CLIENT_LIBRARIES})

target_include_directories(tests PRIVATE external)
target_include_directories(tests PRIVATE include)
target_link_libraries(tests PRIVATE ${BASE_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

gtest_discover_tests(tests)
