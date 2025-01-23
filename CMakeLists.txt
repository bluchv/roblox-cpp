cmake_minimum_required(VERSION 3.30)
project(roblox-cpp)

set(CMAKE_CXX_STANDARD 23)

# Find libclang (try the package first for cross-platform portability)
find_package(clang QUIET) #Not using required to fall back to manual mode
if (CLANG_FOUND)
    message(STATUS "Found clang at: ${CLANG_INCLUDE_DIRS}")
    include_directories(${CLANG_INCLUDE_DIRS})
    message(STATUS "Using LibClang libraries ${CLANG_LIBRARIES}")
    target_link_libraries(${PROJECT_NAME} ${CLANG_LIBRARIES})
else () # Handle the manual approach if package is not found
    message(STATUS "Clang package not found, falling back to manual mode. Please ensure LLVM is installed")
    # Manually set include directory (Windows path)
    set(CLANG_INCLUDE_DIR "C:/Program Files/LLVM/include")
    include_directories(${CLANG_INCLUDE_DIR})

    # Attempt to locate the libclang library
    find_library(LIBCLANG_LIBRARY NAMES clang libclang PATHS "C:/Program Files/LLVM/bin" "C:/Program Files/LLVM/lib" NO_DEFAULT_PATH) #NO_DEFAULT_PATH to prevent conflicts with other libraries that could have the same name

    if (LIBCLANG_LIBRARY)
        message(STATUS "Found LibClang: ${LIBCLANG_LIBRARY}")
        link_libraries(${LIBCLANG_LIBRARY})
    else ()
        message(STATUS "Error, libclang not found, ensure correct install. Check C:/Program Files/LLVM/bin or C:/Program Files/LLVM/lib or similar")
    endif ()
endif ()

add_executable(roblox-cpp src/main.cpp
        include/roblox.h
        src/luau-code-gen/LuauCodeGen.h)
