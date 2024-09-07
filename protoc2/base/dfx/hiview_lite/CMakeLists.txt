project(hiview_lite)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hiview_cache.c
    ${CMAKE_CURRENT_LIST_DIR}/hiview_config.c
    ${CMAKE_CURRENT_LIST_DIR}/hiview_service.c
    ${CMAKE_CURRENT_LIST_DIR}/hiview_util.c
    ${CMAKE_CURRENT_LIST_DIR}/hiview_file.c
    )


target_compile_definitions(${PROJECT_NAME}   PRIVATE
    OUTPUT_OPTION=1
    OUTPUT_LEVEL=1
    HILOG_LITE_SWITCH=1
    DUMP_LITE_SWITCH=0
    HIEVENT_LITE_SWITCH=0
    LOG_OUTPUT_MODULE=65535
    )

# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

# this project depends on the following lib
target_link_libraries(${PROJECT_NAME} PUBLIC samgr_lite samgr_timer ohos_adapter bsp)
target_link_symbol(${PROJECT_NAME} "HiviewConfigInit")
target_link_symbol(${PROJECT_NAME} "HiviewServiceInit")