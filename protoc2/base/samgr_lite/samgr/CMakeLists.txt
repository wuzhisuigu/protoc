project(samgr_lite)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/source/common.c
    ${CMAKE_CURRENT_LIST_DIR}/source/feature.c
    ${CMAKE_CURRENT_LIST_DIR}/source/iunknown.c
    ${CMAKE_CURRENT_LIST_DIR}/source/message.c
    ${CMAKE_CURRENT_LIST_DIR}/source/samgr_lite.c
    ${CMAKE_CURRENT_LIST_DIR}/source/service.c
    ${CMAKE_CURRENT_LIST_DIR}/source/task_manager.c
    ${CMAKE_CURRENT_LIST_DIR}/source/service.c
    ${CMAKE_CURRENT_LIST_DIR}/registry/service_registry.c
    )

 
# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PRIVATE
    # ${CMAKE_CURRENT_LIST_DIR}/adapter
    ${CMAKE_CURRENT_LIST_DIR}/registry
    ${CMAKE_CURRENT_LIST_DIR}/source
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../interfaces/kits/registry
    ${CMAKE_CURRENT_LIST_DIR}/../interfaces/kits/samgr
    )

# this project depends on the following lib
#target_link_libraries(${PROJECT_NAME} hilog_lite CMSIS_RTOS_V2)
target_link_libraries(${PROJECT_NAME} hilog_lite ohos_adapter ohos_defs)