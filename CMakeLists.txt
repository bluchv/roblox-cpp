cmake_minimum_required(VERSION 3.20)
project(roblox-cpp LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find LLVM/Clang
message(STATUS "Looking for LLVM/Clang installation...")

# Prioritize system LLVM installation over MSYS2
if(WIN32)
    set(LLVM_ROOT "C:/Program Files/LLVM")
    
    find_path(LLVM_INCLUDE_DIR 
        NAMES clang-c/Index.h
        PATHS ${LLVM_ROOT}/include
        NO_DEFAULT_PATH
    )
    
    find_library(LIBCLANG_LIBRARY
        NAMES clang libclang
        PATHS ${LLVM_ROOT}/lib ${LLVM_ROOT}/bin
        NO_DEFAULT_PATH
    )
    
    if(LLVM_INCLUDE_DIR AND LIBCLANG_LIBRARY)
        message(STATUS "Found LLVM at: ${LLVM_ROOT}")
        include_directories(${LLVM_INCLUDE_DIR})
    else()
        message(STATUS "System LLVM not found, trying package discovery...")
        # Try to find LLVM package as fallback
        find_package(LLVM QUIET)
        if(LLVM_FOUND)
            message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
            include_directories(${LLVM_INCLUDE_DIRS})
            add_definitions(${LLVM_DEFINITIONS})
            
            # Find libclang
            find_library(LIBCLANG_LIBRARY
                NAMES clang libclang
                PATHS ${LLVM_LIBRARY_DIRS}
                NO_DEFAULT_PATH
            )
        endif()
    endif()
else()
    # Try to find LLVM package first on non-Windows systems
    find_package(LLVM QUIET)
    if(LLVM_FOUND)
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
        include_directories(${LLVM_INCLUDE_DIRS})
        add_definitions(${LLVM_DEFINITIONS})
        
        # Find libclang
        find_library(LIBCLANG_LIBRARY
            NAMES clang libclang
            PATHS ${LLVM_LIBRARY_DIRS}
            NO_DEFAULT_PATH
        )
    else()
        # Fallback to manual detection
        message(STATUS "LLVM package not found, trying manual detection...")
        set(LLVM_ROOT "/usr/lib/llvm-15" "/usr/lib/llvm-14" "/usr/lib/llvm-13")
        
        find_path(LLVM_INCLUDE_DIR 
            NAMES clang-c/Index.h
            PATHS ${LLVM_ROOT}/include
            NO_DEFAULT_PATH
        )
        
        find_library(LIBCLANG_LIBRARY
            NAMES clang libclang
            PATHS ${LLVM_ROOT}/lib ${LLVM_ROOT}/bin
            NO_DEFAULT_PATH
        )
        
        if(LLVM_INCLUDE_DIR)
            include_directories(${LLVM_INCLUDE_DIR})
        endif()
    endif()
endif()

# Check if we found libclang
if(LIBCLANG_LIBRARY)
    message(STATUS "Found libclang: ${LIBCLANG_LIBRARY}")
else()
    message(FATAL_ERROR "libclang not found. Please install LLVM/Clang development packages.")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/transpiler/transpiler.cpp
    src/transpiler/ast_visitor.cpp
    src/ast/luau_node.cpp
    src/ast/function_node.cpp
    src/ast/variable_node.cpp
    src/ast/statement_node.cpp
    src/ast/class_node.cpp
    src/codegen/luau_codegen.cpp
    src/utils/clang_utils.cpp
    src/utils/type_mapper.cpp
)

# Header files (for IDE organization)
set(HEADERS
    include/roblox_api.h
    src/transpiler/transpiler.h
    src/transpiler/ast_visitor.h
    src/ast/luau_node.h
    src/ast/function_node.h
    src/ast/variable_node.h
    src/ast/statement_node.h
    src/ast/class_node.h
    src/codegen/luau_codegen.h
    src/utils/clang_utils.h
    src/utils/type_mapper.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${LIBCLANG_LIBRARY})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
