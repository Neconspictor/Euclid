#--Utilities for Visual Studio IDE

# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
function(assign_source_group)
    if (MSVC)
        foreach(_source IN ITEMS ${ARGN})
            if (IS_ABSOLUTE "${_source}")
                file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
            else()
                set(_source_rel "${_source}")
            endif()
            get_filename_component(_source_path "${_source_rel}" PATH)
            string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
            source_group("${_source_path_msvc}" FILES "${_source}")
        endforeach()
    endif()
endfunction(assign_source_group)

function(assign_source_group2 SOURCES SOURCE_DIR)
    if (MSVC)
        foreach(_source IN ITEMS ${SOURCES})
            if (IS_ABSOLUTE "${_source}")
                file(RELATIVE_PATH _source_rel "${SOURCE_DIR}" "${_source}")
            else()
                set(_source_rel "${_source}")
            endif()
            get_filename_component(_source_path "${_source_rel}" PATH)
            string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
            source_group("${_source_path_msvc}" FILES "${_source}")
        endforeach()
    endif()
endfunction(assign_source_group2)

function(get_library_path LIBRARY_NAME RETURN_VALUE)
    SET(${RETURN_VALUE} "${EUCLID_BASE_LIBRARY_FOLDER}/${LIBRARY_NAME}/${EUCLID_CANONICAL_LIBRARY_SUBPATH}" PARENT_SCOPE)
endfunction(get_library_path)

# Instructs the MSVC toolset to use the precompiled header PRECOMPILED_HEADER
# for each source file given in the collection named by SOURCE_VARIABLE_NAME.
function(enable_precompiled_headers PRECOMPILED_HEADER SOURCE_VARIABLE_NAME)
    if(MSVC)
        set(files ${${SOURCE_VARIABLE_NAME}})

        # Generate precompiled header translation unit
        get_filename_component(pch_basename ${PRECOMPILED_HEADER} NAME_WE)
        set(pch_abs ${CMAKE_CURRENT_SOURCE_DIR}/${PRECOMPILED_HEADER})
        set(pch_unity ${CMAKE_CURRENT_BINARY_DIR}/${pch_basename}.cpp)
        FILE(WRITE ${pch_unity} "// Precompiled header unity generated by CMake\n")
        FILE(APPEND ${pch_unity} "#include <${pch_abs}>\n")
        set_source_files_properties(${pch_unity}  PROPERTIES COMPILE_FLAGS "/Yc\"${pch_abs}\"")

        # Update properties of source files to use the precompiled header.
        # Additionally, force the inclusion of the precompiled header at beginning of each source file.
        foreach(source_file ${files} )
            set_source_files_properties(
                    ${source_file}
                    PROPERTIES COMPILE_FLAGS
                    "/Yu\"${pch_abs}\" /FI\"${pch_abs}\""
            )
        endforeach(source_file)

        # Finally, update the source file collection to contain the precompiled header translation unit
        set(${SOURCE_VARIABLE_NAME} ${pch_unity} ${${SOURCE_VARIABLE_NAME}} PARENT_SCOPE)
    endif(MSVC)
endfunction(enable_precompiled_headers)

#--Utilities for Visual Studio IDE

#check supported processor architectures for the target platform

if (NOT (CMAKE_SYSTEM_PROCESSOR MATCHES ".*AMD64.*"
        OR CMAKE_SYSTEM_PROCESSOR MATCHES ".*x86.*"
        OR CMAKE_SYSTEM_PROCESSOR MATCHES ".*x64.*"))
    message(FATAL_ERROR "processor type currently not supported: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# check supported operating system of the target platform
SET(EUCLID_SPECIAL_PLATFORM "")

if (WIN32)
    message(STATUS "Detected Windows target")
        SET(EUCLID_PLATFORM_NAME win32)
    if(MINGW)
        message(STATUS "Detected MinGW target")
        SET(EUCLID_SPECIAL_PLATFORM mingw)
    elseif(CYGWIN)
        message(STATUS "Detected Cygwin target")
        SET(EUCLID_SPECIAL_PLATFORM cygwin)
    endif()
elseif(LINUX)
    message(STATUS "Detected Linux target")
    SET(EUCLID_PLATFORM_NAME linux)
elseif(APPLE)
    message(STATUS "Detected Mac target")
    SET(EUCLID_PLATFORM_NAME macOS)
else()
    message(FATAL_ERROR "Not supported platform")
endif()

# detect 32 or 64 bit address model
if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    SET(EUCLID_ADDRESS_MODEL x86)
        if (NOT CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
            set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID x86)
        endif()
    message(STATUS "Detected 32 bit address model")
elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    SET(EUCLID_ADDRESS_MODEL x64)
    if (NOT CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
        set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID x64)
    endif()
    message(STATUS "Detected 64 bit address model")
else()
    message(FATAL_ERROR "Cannot derivate supported address model from pointer size: ${CMAKE_SIZEOF_VOID_P}")
endif()




if (MSVC)
    # visual studio specific stuff
    include(${CMAKE_CURRENT_LIST_DIR}/msvc.cmake)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # gcc specific stuff
    include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
else()
    # Currently, other compilers are not supported.
    message(FATAL_ERROR "Currently, the compiler ${CMAKE_CXX_COMPILER_ID} is not supported.")
endif()

# set base library folder
SET(EUCLID_BASE_LIBRARY_FOLDER ${CMAKE_SOURCE_DIR}/lib)

# The canonical library paths for each library is <platform>/<compiler-name>/<address-model>
if (EUCLID_SPECIAL_PLATFORM)
    SET(EUCLID_CANONICAL_LIBRARY_SUBPATH "${EUCLID_PLATFORM_NAME}/${EUCLID_SPECIAL_PLATFORM}/${EUCLID_COMPILER_FOLDER_NAME}/${EUCLID_ADDRESS_MODEL}")
else()
    SET(EUCLID_CANONICAL_LIBRARY_SUBPATH "${EUCLID_PLATFORM_NAME}/${EUCLID_COMPILER_FOLDER_NAME}/${EUCLID_ADDRESS_MODEL}")
endif()
message(STATUS "EUCLID_CANONICAL_LIBRARY_SUBPATH = ${EUCLID_CANONICAL_LIBRARY_SUBPATH}")

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message (STATUS "cxx Flags: " ${CMAKE_CXX_FLAGS})