
set(PROJECT_NAME gpark)

add_compile_options(-std=c++1y)

set(INC_DIR 
	/usr/local/include
)

set(LNK_DIR
	/usr/local/lib
)

set(LNK_LIB
	ssl
	crypto
)

include_directories(${INC_DIR})
link_directories(${LNK_DIR})
link_libraries(${LNK_LIB})

aux_add_exe(${PROJECT_NAME} "project")
