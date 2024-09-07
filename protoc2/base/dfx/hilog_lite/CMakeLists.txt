
project(hilog_lite)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/frameworks/mini/hiview_log_limit.c
    ${CMAKE_CURRENT_LIST_DIR}/frameworks/mini/hiview_log.c
    )


target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/frameworks/mini/hiview_output_log.c
    ${CMAKE_CURRENT_LIST_DIR}/frameworks/mini/hiview_fifo_log.c
    )


# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/frameworks/mini
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/interfaces/native/kits/hilog_lite)

# this project depends on the following lib
target_link_libraries(${PROJECT_NAME} hiview_lite samgr_lite ring_buffer)
target_link_symbol(${PROJECT_NAME} "HiLogInit")