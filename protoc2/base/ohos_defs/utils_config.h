/**
 * @addtogroup utils_config
 * @{
 *
 * @brief Represents the configuration file of the utils subsystem.
 *
 * Vendors can set their desired items in the configuration file.
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file utils_config.h
 *
 * @brief Represents the configuration file of the utils subsystem.
 *
 * Vendors can set their desired items in the configuration file.
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef UTILS_CONFIG_H
#define UTILS_CONFIG_H

/**
 * @brief Specifies whether the KV store supports cache.
 *
 * If this variable is defined, the KV store supports cache.
 * Vendors determine whether to enable this feature. It is enabled by default.
 */
#define FEATURE_KV_CACHE

/**
 * @brief Indicates the number of data items that can be stored in the KV store cache.
 *
 * Default value: 10
 */
#define MAX_CACHE_SIZE 10

/**
 * @brief Indicates the number of data items that can be stored for each application.
 *
 * Default value: 50
 */
#define MAX_KV_SUM 50

#endif // UTILS_CONFIG_H
       /** @} */