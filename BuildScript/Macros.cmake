
include(CMakeParseArguments)
MESSAGE(STATUS "is build" ${CMAKE_BUILD_TYPE})

macro(add_lib target)
	cmake_parse_arguments(THIS "SHARED;INSTALL" "FOLDER" "SOURCES" ${ARGN})

	if (THIS_SHARED)
		SET(LIBRARY_OUTPUT_PATH ${BIN_PATH})
        add_library(${target} SHARED ${THIS_SOURCES})
    else()
		SET(LIBRARY_OUTPUT_PATH ${LIB_PATH})
        add_library(${target} ${THIS_SOURCES})
    endif()
	
	string(REPLACE "-" "_" NAME_UPPER "${target}")
    string(TOUPPER "${NAME_UPPER}" NAME_UPPER)
    set_target_properties(${target} PROPERTIES DEFINE_SYMBOL ${NAME_UPPER}_EXPORTS)

	if(OS_IS_MACOS)
		set_property(TARGET ${target} PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	endif()

	if (THIS_FOLDER)
		set_target_properties(${target} PROPERTIES FOLDER ${THIS_FOLDER})
	endif()
	
	if (THIS_INSTALL)
		if(OS_IS_WINDOWS)
			INSTALL(TARGETS ${target} RUNTIME DESTINATION ${BIN_PATH}${OUTPATH_SUFFIX})
		else()
			INSTALL(TARGETS ${target} LIBRARY DESTINATION ${BIN_PATH})
		endif()
	endif()
endmacro()

macro(add_exe target)
	cmake_parse_arguments(THIS "" "FOLDER" "SOURCES" ${ARGN})
	
	SET(EXECUTABLE_OUTPUT_PATH ${BIN_PATH})
		
	add_executable(${target} ${THIS_SOURCES})

	if (THIS_FOLDER)
		set_target_properties(${target} PROPERTIES FOLDER ${THIS_FOLDER})
	endif()

endmacro()

macro(aux_src)
	#cmake_parse_arguments(THIS "" "" "INCLUDE;EXCLUDE" ${ARGN})

	set(AUX_SRCS)
	set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")
	if(${ARGC} GREATER 0)
		foreach(_path IN ITEMS ${ARGN})
			file(
				GLOB _cur_source_list
				LIST_DIRECTORIES false
        		"${_src_root_path}/${_path}/*.c"
        		"${_src_root_path}/${_path}/*.cc"
        		"${_src_root_path}/${_path}/*.cpp"
        		"${_src_root_path}/${_path}/*.h"
        		"${_src_root_path}/${_path}/*.hpp"
        		"${_src_root_path}/${_path}/*.m"
        		"${_src_root_path}/${_path}/*.mm"
        		"${_src_root_path}/${_path}/*.inl"
    		)        
			list(APPEND _source_list ${_cur_source_list})
		endforeach()
	else()
		file(
			GLOB_RECURSE _source_list
			LIST_DIRECTORIES false
        	"${_src_root_path}/*.c"
        	"${_src_root_path}/*.cc"
        	"${_src_root_path}/*.cpp"
        	"${_src_root_path}/*.h"
        	"${_src_root_path}/*.hpp"
        	"${_src_root_path}/*.m"
        	"${_src_root_path}/*.mm"
        	"${_src_root_path}/*.inl"
    	)        
	endif()

    foreach(_source IN ITEMS ${_source_list})
        get_filename_component(_source_path "${_source}" PATH)
        file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		#include_directories(${_source_path_rel})
        string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
        source_group("${_group_path}" FILES "${_source}")
        file(RELATIVE_PATH _source_rel "${_src_root_path}" "${_source}")
             
		list(APPEND AUX_SRCS ${_source_rel})
    endforeach()
endmacro()

macro(aux_add_lib target the_folder)
	aux_src(${ARGN})
	add_lib(
		${target}
		FOLDER ${the_folder}
		SOURCES ${AUX_SRCS}
	)
endmacro()

macro(aux_add_dll target the_folder)
	aux_src(${ARGN})
    add_lib(
        ${target}
		SHARED
        FOLDER ${the_folder}
        SOURCES ${AUX_SRCS}
    )
endmacro()

macro(aux_add_exe target the_folder)
	aux_src(${ARGN})
    add_exe(
        ${target}
        FOLDER ${the_folder}
        SOURCES ${AUX_SRCS}
    )   
endmacro()

