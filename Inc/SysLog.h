/**
 * @file sys_log.h
 * @brief Universal logging system header file.
 *
 * Created on: 23 December 2024
 * Author: Nate Hunter
 */

#ifndef LOG_SYS_LOG_H_
#define LOG_SYS_LOG_H_

#include "stdint.h"

/** @def LOG_MMC_DIR
 * @brief Directory path for log files on MMC.
 */
#define LOG_MMC_DIR "log/"

/** @def LOG_MMC_FILE_FORMAT
 * @brief Format of log filenames.
 */
#define LOG_MMC_FILE_FORMAT "SYS_log_%03u.log"

/** @def LOG_UDP_PORT
 * @brief UDP port for log output.
 */
#define LOG_UDP_PORT 20101

/** @def MAX_LOG_MESSAGE_SIZE
 * @brief Maximum characters per log message.
 */
#define MAX_LOG_MESSAGE_SIZE 128

/** @def LOG_QUEUE_LENGTH
 * @brief Maximum size of the log message queue.
 */
#define LOG_QUEUE_LENGTH MAX_LOG_MESSAGE_SIZE * 4

/** @def LOG_LOCAL_LEVEL
 * @brief Default log level for local messages.
 */
#define LOG_LOCAL_LEVEL SYS_LOG_INFO


/**
 * @brief Enumeration for log levels.
 */
typedef enum {
    SYS_LOG_NONE,     /*!< No log output */
    SYS_LOG_ERROR,    /*!< Critical errors, software module cannot recover on its own */
    SYS_LOG_WARN,     /*!< Errors with recovery measures taken */
    SYS_LOG_INFO,     /*!< Normal flow of events */
    SYS_LOG_DEBUG,    /*!< Extra information for debugging */
    SYS_LOG_VERBOSE   /*!< Frequent messages for detailed debugging */
} sys_log_level_t;

/**
 * @brief Structure for log settings.
 */
typedef struct {
    uint8_t log_swv;       /*!< Toggle SWV output */
    uint8_t log_emmc;      /*!< Toggle eMMC output */
    uint8_t log_udp;       /*!< Toggle UDP output */
    uint32_t max_files;    /*!< Maximum number of eMMC files */
    uint32_t file_size;    /*!< Maximum size of one eMMC file */
} sys_log_settings_t;

/** Log tags */
#define TAG_SYS  "SYS"

/** Log macros */
#define SYS_LOGE(tag, format, ...) SYS_LOG_LEVEL_LOCAL(SYS_LOG_ERROR,   tag, format, ##__VA_ARGS__)
#define SYS_LOGW(tag, format, ...) SYS_LOG_LEVEL_LOCAL(SYS_LOG_WARN,    tag, format, ##__VA_ARGS__)
#define SYS_LOGI(tag, format, ...) SYS_LOG_LEVEL_LOCAL(SYS_LOG_INFO,    tag, format, ##__VA_ARGS__)
#define SYS_LOGD(tag, format, ...) SYS_LOG_LEVEL_LOCAL(SYS_LOG_DEBUG,   tag, format, ##__VA_ARGS__)
#define SYS_LOGV(tag, format, ...) SYS_LOG_LEVEL_LOCAL(SYS_LOG_VERBOSE, tag, format, ##__VA_ARGS__)

#define SYS_LOG_LEVEL_LOCAL(level, tag, format, ...) do {               \
        if ( LOG_LOCAL_LEVEL >= level ) SYS_LOG_LEVEL(level, tag, format, ##__VA_ARGS__); \
    } while(0)

#define LOG_FORMAT(letter, format) letter " (%lu) %s: " format "\n"
#define LOG_FILENAME_LEN (sizeof(LOG_MMC_DIR LOG_MMC_FILE_FORMAT))

#define SYS_LOG_LEVEL(level, tag, format, ...) do {                     \
        if (level==SYS_LOG_ERROR )          { sys_log_write(SYS_LOG_ERROR,      tag, LOG_FORMAT("E", format), sys_log_timestamp(), tag, ##__VA_ARGS__); } \
        else if (level==SYS_LOG_WARN )      { sys_log_write(SYS_LOG_WARN,       tag, LOG_FORMAT("W", format), sys_log_timestamp(), tag, ##__VA_ARGS__); } \
        else if (level==SYS_LOG_DEBUG )     { sys_log_write(SYS_LOG_DEBUG,      tag, LOG_FORMAT("D", format), sys_log_timestamp(), tag, ##__VA_ARGS__); } \
        else if (level==SYS_LOG_VERBOSE )   { sys_log_write(SYS_LOG_VERBOSE,    tag, LOG_FORMAT("V", format), sys_log_timestamp(), tag, ##__VA_ARGS__); } \
        else                                { sys_log_write(SYS_LOG_INFO,       tag, LOG_FORMAT("I", format), sys_log_timestamp(), tag, ##__VA_ARGS__); } \
    } while(0)

#define SCSS_FAIL(x) ((x) ? "success" : "fail")

/**
 * @brief Get the current timestamp for logging.
 *
 * @return Current timestamp in milliseconds.
 */
uint32_t sys_log_timestamp(void);

/**
 * @brief Initialize the logging system.
 *
 * @param settings Pointer to the logging settings structure.
 */
void SYS_LOG_Init(sys_log_settings_t *settings);

/**
 * @brief Update logging methods based on new settings.
 *
 * @param newSettings Pointer to the new settings structure.
 */
void SYS_LOG_UpdateMethods(sys_log_settings_t *newSettings);

/**
 * @brief Write a log message.
 *
 * @param level Log level.
 * @param tag Log tag.
 * @param format Message format string.
 * @param ... Additional arguments for the format string.
 */
void sys_log_write(sys_log_level_t level, const char *tag, const char *format, ...);

#endif /* LOG_SYS_LOG_H_ */
