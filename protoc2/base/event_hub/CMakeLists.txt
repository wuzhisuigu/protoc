
project(event_hub)
add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/source/event_hub_service.c
    ${CMAKE_CURRENT_LIST_DIR}/source/event_hub_api.c
    )

# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    )

# this project depends on the following lib
target_link_libraries(${PROJECT_NAME} samgr_lite)
# force the zinitcall section to be linked by setting undefined symbol to linker
target_link_symbol(${PROJECT_NAME} "EventHubInit")