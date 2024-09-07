
project(ohos_defs)

add_library(${PROJECT_NAME} OBJECT)
target_sources(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/ohos_init.c
    )
# back propagate library headers inc path to top level modules
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})