/**
 * @file sys_log.c
 * @brief Implementation of the universal logging system.
 *
 * Created on: 23 December 2024
 * Author: Nate Hunter
 */

#include "SysLog.h"
#include "cmsis_os.h"
#include "task.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include "fatfs.h"
#include "FreeRTOS.h"
#include "queue.h"

/** Task and Queue creation macros */
#define LOG_CREATE_SWV() xTaskCreate(LogTask_SWV, "LogSWV", 128, NULL, osPriorityNormal, &LogSWV_Handle); \
                         swvQueue = xQueueCreate(LOG_QUEUE_LENGTH, MAX_LOG_MESSAGE_SIZE);

#define LOG_CREATE_UDP() xTaskCreate(LogTask_UDP, "LogUDP", 448, NULL, osPriorityNormal, &LogUDP_Handle); \
                         udpQueue = xQueueCreate(LOG_QUEUE_LENGTH, MAX_LOG_MESSAGE_SIZE);

#define LOG_CREATE_MMC() xTaskCreate(LogTask_eMMC, "LogMMC", 544, NULL, osPriorityNormal, &LogMMC_Handle); \
                         mmcQueue = xQueueCreate(LOG_QUEUE_LENGTH, MAX_LOG_MESSAGE_SIZE);

/** Task and Queue deletion macros */
#define LOG_DELETE_SWV() vTaskDelete(LogSWV_Handle); \
                         vQueueDelete(swvQueue);

#define LOG_DELETE_UDP() vTaskDelete(LogUDP_Handle); \
                         vQueueDelete(udpQueue);

#define LOG_DELETE_MMC() vTaskDelete(LogMMC_Handle); \
                         vQueueDelete(mmcQueue);

/**
 * @brief Logger structure.
 */
typedef struct {
    uint8_t initialized;              /*!< Initialization flag */
    sys_log_settings_t settings;     /*!< Logger settings */
} sys_logger_t;

static sys_logger_t logger;

/** Queues for log messages */
static QueueHandle_t swvQueue = NULL;
static QueueHandle_t udpQueue = NULL;
static QueueHandle_t mmcQueue = NULL;

/** Task handles */
static TaskHandle_t LogSWV_Handle = NULL;
static TaskHandle_t LogUDP_Handle = NULL;
static TaskHandle_t LogMMC_Handle = NULL;

/**
 * @brief Thread for SWV log output.
 *
 * @param arg Unused parameter.
 */
void LogTask_SWV(void *arg);

/**
 * @brief Thread for UDP log output.
 *
 * @param arg Unused parameter.
 */
void LogTask_UDP(void *arg);

/**
 * @brief Thread for eMMC log output.
 *
 * @param arg Unused parameter.
 */
void LogTask_eMMC(void *arg);

/**
 * @brief Initialize the logging system.
 *
 * @param settings Pointer to the logging settings structure.
 */
void SYS_LOG_Init(sys_log_settings_t *settings) {
    if (logger.initialized) {
        return;
    }

    logger.settings = *settings;

    if (logger.settings.log_swv) {
        LOG_CREATE_SWV();
    }

    if (logger.settings.log_udp) {
        LOG_CREATE_UDP();
    }

    if (logger.settings.log_emmc) {
        LOG_CREATE_MMC();
    }

    if (!udpQueue || !mmcQueue || !swvQueue) {
        printf("Failed to create log queues\n");
        return;
    }

    logger.initialized = true;
}

/**
 * @brief Update logging methods based on new settings.
 *
 * @param newSettings Pointer to the new settings structure.
 */
void SYS_LOG_UpdateMethods(sys_log_settings_t *newSettings) {
    logger.settings = *newSettings;

    if (logger.settings.log_swv && LogSWV_Handle == NULL) {
        LOG_CREATE_SWV();
    } else if (!logger.settings.log_swv && LogSWV_Handle != NULL) {
        LOG_DELETE_SWV();
    }

    if (logger.settings.log_emmc && LogMMC_Handle == NULL) {
        LOG_CREATE_MMC();
    } else if (!logger.settings.log_emmc && LogMMC_Handle != NULL) {
        LOG_DELETE_MMC();
    }

    if (logger.settings.log_udp && LogUDP_Handle == NULL) {
        LOG_CREATE_UDP();
    } else if (!logger.settings.log_udp && LogUDP_Handle != NULL) {
        LOG_DELETE_UDP();
    }
}

/**
 * @brief Get the current timestamp for logging.
 *
 * @return Current timestamp in milliseconds.
 */
uint32_t sys_log_timestamp(void) {
    return HAL_GetTick();
}

/**
 * @brief Write a log message.
 *
 * @param level Log level.
 * @param tag Log tag.
 * @param format Message format string.
 * @param ... Additional arguments for the format string.
 */
void sys_log_write(sys_log_level_t level, const char *tag, const char *format, ...) {
    if (!logger.initialized) {
        return;
    }

    char logMessage[MAX_LOG_MESSAGE_SIZE];
    va_list list;
    va_start(list, format);

    int length = vsprintf(logMessage, format, list);
    va_end(list);

    if (length < 0 || length >= MAX_LOG_MESSAGE_SIZE) {
        length = MAX_LOG_MESSAGE_SIZE - 1;
        logMessage[length] = '\0';
    }

    if (logger.settings.log_swv) {
        xQueueSend(swvQueue, logMessage, portMAX_DELAY);
    }

    if (logger.settings.log_udp) {
        xQueueSend(udpQueue, logMessage, portMAX_DELAY);
    }

    if (logger.settings.log_emmc) {
        xQueueSend(mmcQueue, logMessage, portMAX_DELAY);
    }
}

/**
 * @brief Send a string via ITM (Instrumentation Trace Macrocell) for SWV.
 *
 * @param str Pointer to the null-terminated string to send.
 */
void ITM_SendString(const char *str) {
    while (*str) {
        if (ITM->PORT[0].u32 == 1) {
            ITM->PORT[0].u32 = *str++;
        }
    }
}

/**
 * @brief Thread for sending logs using SWV.
 *
 * @param arg Unused parameter.
 */
void LogTask_SWV(void *arg) {
    char logMessage[MAX_LOG_MESSAGE_SIZE];

    while (1) {
        if (xQueueReceive(swvQueue, logMessage, portMAX_DELAY) == pdPASS) {
            ITM_SendString(logMessage);
        }
    }
}
/**
 * @brief Thread for sending logs using UDP.
 *
 * @param arg Unused parameter.
 */
void LogTask_UDP(void *arg) {
    struct sockaddr_in udp_log;
    int udp_sock;
    char logMessage[MAX_LOG_MESSAGE_SIZE];

    // Create UDP socket
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        vTaskDelete(NULL);
        return;
    }

    // Set up UDP address
    ip_addr_t ip_addr;
    IP_ADDR4(&ip_addr, 239, 255, 50, 50);

    udp_log.sin_family = AF_INET;
    udp_log.sin_port = htons(LOG_UDP_PORT);
    udp_log.sin_addr.s_addr = ip_addr.addr;

    while (1) {
        if (xQueueReceive(udpQueue, logMessage, portMAX_DELAY) == pdPASS) {
            lwip_sendto(udp_sock, logMessage, strlen(logMessage), 0, (struct sockaddr*)&udp_log, sizeof(udp_log));
        }
    }
}

/**
 * @brief Manage log files in a directory.
 *
 * @param new_name Pointer to store the valid filename.
 * @return FRESULT FatFS operation result.
 */
FRESULT ManageLogFiles(char *new_name) {
    FRESULT res;
    DIR dir;
    FILINFO fno;
    char old_name[LOG_FILENAME_LEN];
    uint16_t file_numbers[128] = { 0 }; // Assumes no more than 128 files
    uint16_t file_count = 0;

    // Open directory
    res = f_opendir(&dir, LOG_MMC_DIR);
    if (res != FR_OK) {
        return res;
    }

    // Read all files matching the pattern
    while ((res = f_readdir(&dir, &fno)) == FR_OK && fno.fname[0] != 0) {
        unsigned int num;
        if (sscanf(fno.fname, LOG_MMC_FILE_FORMAT, &num) == 1) {
            if (file_count < sizeof(file_numbers) / sizeof(file_numbers[0])) {
                file_numbers[file_count++] = num;
            }
        }
    }
    f_closedir(&dir);

    if (res != FR_OK) {
        return res;
    }

    uint16_t i;
    uint16_t deleted_files = 0;
    // Delete oldest files if file count exceeds limit
    if (file_count > logger.settings.max_files - 1) {
        for (i = 0; i < file_count - logger.settings.max_files + 1; i++) {
            snprintf(old_name, LOG_FILENAME_LEN, LOG_MMC_DIR LOG_MMC_FILE_FORMAT, file_numbers[i]);
            f_unlink(old_name);
            deleted_files++;
        }

        file_count = logger.settings.max_files - 1; // Update count after deletion
    }

    // Rename files to maintain sequential numbering
    for (i = 0; i < file_count; i++) {
        snprintf(old_name, LOG_FILENAME_LEN, LOG_MMC_DIR LOG_MMC_FILE_FORMAT, file_numbers[i + deleted_files]);
        snprintf(new_name, LOG_FILENAME_LEN, LOG_MMC_DIR LOG_MMC_FILE_FORMAT, i);
        if (strcmp(old_name, new_name) != 0) {
            res = f_rename(old_name, new_name);
            if (res != FR_OK) {
                return res;
            }
        }
    }

    // Create new file with the next available number
    snprintf(new_name, LOG_FILENAME_LEN, LOG_MMC_DIR LOG_MMC_FILE_FORMAT, file_count);
    return res;
}

/**
 * @brief Thread for storing logs in MMC Chip.
 *
 * @param arg Unused parameter.
 */
void LogTask_eMMC(void *arg) {
    FIL logFile;
    char filePath[LOG_FILENAME_LEN];
    char logMessage[MAX_LOG_MESSAGE_SIZE];
    uint32_t currentFileSize = 0;        // Track current file size

    // Check if LOG_DIR exists & try to create it
    DIR dir;
    FRESULT res = f_opendir(&dir, LOG_MMC_DIR);
    if (res == FR_NO_PATH) {
        res = f_mkdir(LOG_MMC_DIR);
        if (res != FR_OK) {
            ITM_SendString("MMC: Create log dir error\n");
            vTaskDelete(NULL);
        }
    } else if (res != FR_OK) {
        ITM_SendString("MMC: Log dir open error\n");
        vTaskDelete(NULL);
    } else {
        f_closedir(&dir); // All OK
    }

    // Initialize file management
    if (ManageLogFiles(filePath) != FR_OK) {
        ITM_SendString("MMC: Log manager failed\n");
    }

    // Open file, check & save initial size (must be 0)
    res = f_open(&logFile, filePath, FA_CREATE_NEW | FA_WRITE);
    if (res != FR_OK) {
        ITM_SendString("MMC: Log file open failed\n");
    } else {
        currentFileSize = f_size(&logFile);
    }
    f_close(&logFile);

    while (1) {
        if (xQueueReceive(mmcQueue, logMessage, portMAX_DELAY) == pdPASS) {
            if (f_open(&logFile, filePath, FA_WRITE | FA_OPEN_APPEND) == FR_OK) {
                UINT bytesWritten;
                f_write(&logFile, logMessage, strlen(logMessage), &bytesWritten);
                currentFileSize += bytesWritten;
                f_close(&logFile);
                if (currentFileSize >= logger.settings.file_size) {
                    if (ManageLogFiles(filePath) != FR_OK) ITM_SendString("MMC: Log manager failed\n");
                    else currentFileSize = 0;
                }
            }
        }
    }
}
