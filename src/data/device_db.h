#ifndef DEVICE_DB_H
#define DEVICE_DB_H

#include <stdint.h>
#include <stdbool.h>
#include "lwip/ip_addr.h"

// Device database entry
typedef struct {
    ip_addr_t ip;
    uint8_t mac[6];
    char hostname[32];
    char vendor[32];
    char custom_name[32];
    uint32_t first_seen;
    uint32_t last_seen;
    bool is_favorite;
    bool is_known;
} device_db_entry_t;

// Function prototypes
void device_db_init(void);
bool device_db_add_device(ip_addr_t* ip, const uint8_t* mac, const char* hostname, const char* vendor);
bool device_db_find_device(ip_addr_t* ip, device_db_entry_t* entry);
bool device_db_get_device(uint32_t index, device_db_entry_t* entry);
uint32_t device_db_get_count(void);
void device_db_clear(void);
bool device_db_save(void);
bool device_db_load(void);

#endif