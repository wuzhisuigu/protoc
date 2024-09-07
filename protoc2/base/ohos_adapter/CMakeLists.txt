
project(ohos_adapter)

add_library(${PROJECT_NAME} STATIC)


if (${TARGET_OS} STREQUAL "freertos")
    target_sources(${PROJECT_NAME} 
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/freertos/memory_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/freertos/queue_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/freertos/thread_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/freertos/time_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/freertos/timer_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/freertos/sem_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/freertos/mutex_adapter.c
    	)

#    ${CMAKE_CURRENT_LIST_DIR}/samgr/adapter/cmsis/memory_adapter.c
#    ${CMAKE_CURRENT_LIST_DIR}/samgr/adapter/cmsis/queue_adapter.c
#    ${CMAKE_CURRENT_LIST_DIR}/samgr/adapter/cmsis/thread_adapter.c
#    ${CMAKE_CURRENT_LIST_DIR}/samgr/adapter/cmsis/time_adapter.c

target_link_libraries(${PROJECT_NAME} PUBLIC ohos_defs PRIVATE freertos bsp)

else ()
    target_sources(${PROJECT_NAME} 
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/posix/memory_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/queue_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/thread_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/time_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/timer_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/lock_free_queue.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/sem_adapter.c
        ${CMAKE_CURRENT_LIST_DIR}/posix/mutex_adapter.c
    	)

    target_link_libraries(${PROJECT_NAME} PUBLIC ohos_defs PRIVATE pthread bsp)

endif ()
# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    )

# this project depends on the following lib
#target_link_libraries(${PROJECT_NAME} hilog_lite CMSIS_RTOS_V2)

