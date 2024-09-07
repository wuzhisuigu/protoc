
project(dfx_shell)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/source/shell.c
    ${CMAKE_CURRENT_LIST_DIR}/source/shell_cmd_list.c
    ${CMAKE_CURRENT_LIST_DIR}/source/shell_ext.c
    ${CMAKE_CURRENT_LIST_DIR}/source/shell_service.c
    )


# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/source
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include)

# this project depends on the following lib
target_link_libraries(${PROJECT_NAME} PUBLIC bsp hiview_lite samgr_lite)
target_link_symbol(${PROJECT_NAME} "DfxShellInit")
target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_DFX_SHELL)
#target_link_options(${PROJECT_NAME} PUBLIC "LINKER:SHELL:-u _DfxShellInit")