
add_subdirectory(samgr)
add_subdirectory(samgr_timer)

if (NOT ${MCU_CORE} STREQUAL "SINGLE_CORE")
add_subdirectory(samgr_rpc)
endif()