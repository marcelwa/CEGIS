cmake_minimum_required(VERSION 3.2)
project(CEGIS)

set(CMAKE_CXX_STANDARD 11)

# Find source files
file(GLOB SOURCES src/*.cpp)

# Include header files
include_directories(include)

# Require Boost
find_package(Boost 1.58.0 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Require Python interpreter
find_package(PythonInterp REQUIRED)

# Clone, build and install Z3
include(ExternalProject)

set(Z3_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/z3/)
set(Z3_SRC    ${Z3_PREFIX}/src/z3/)
set(Z3_CMAKE  ${Z3_SRC}/contrib/cmake/)
set(Z3_BUILD  ${Z3_PREFIX}/src/build/)

ExternalProject_Add(
        z3
        PREFIX              ${Z3_PREFIX}
        GIT_REPOSITORY      https://github.com/Z3Prover/z3.git
        GIT_TAG             z3-4.5.0
        CONFIGURE_COMMAND   ${PYTHON_EXECUTABLE} ${Z3_CMAKE}/bootstrap.py create
        BINARY_DIR          ${Z3_BUILD}
        BUILD_COMMAND       cmake ../z3/ && make -j4
        INSTALL_COMMAND     sudo make install
        UPDATE_DISCONNECTED 1
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION lib/)

# Install library headers
file(GLOB HEADERS include/*.h)
install(FILES ${HEADERS} DESTINATION include/)
