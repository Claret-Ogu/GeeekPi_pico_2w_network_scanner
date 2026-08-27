#include "device_db.h"
#include <string.h>
#include <stdio.h>
#include "hardware/flash.h"

// Flash storage
#define FLASH_TARGET_OFFSET (256 * 1024) // 256KB offset for data
#define DEVICE_DB_MAGIC 0xDEADBEEF
#define MAX_DB_ENTRIES 64

// Database structure
typedef struct {
    uint32_t magic;
    uint32_t count;
    device_db_entry_t entries[MAX_DB_ENTRIES];
} device_db_t;

static device_db_t db;
static bool db_initialized = false;

void device_db_init(void) {
    if (db_initialized) return;
    
    memset(&db, 0, sizeof(db));
    
    // Try to load from flash
    if (!device_db_load()) {
        // Initialize empty database
        db.magic = DEVICE_DB_MAGIC;
        db.count = 0;
    }
    
    db_initialized = true;
}

bool device_db_add_device(ip_addr_t* ip, const uint8_t* mac, const char* hostname, const char* vendor) {
    if (!ip || !mac || db.count >= MAX_DB_ENTRIES) return false;
    
    // Check if device already exists
    for (uint32_t i = 0; i < db.count; i++) {
        if (ip_addr_cmp(&db.entries[i].ip, ip)) {
            // Update existing entry
            db.entries[i].last_seen = time_us_32() / 1000000;
            return true;
        }
    }
    
    // Add new entry
    device_db_entry_t* entry = &db.entries[db.count];
    ip_addr_copy(entry->ip, *ip);
    memcpy(entry->mac, mac, 6);
    
    if (hostname) {
        strncpy(entry->hostname, hostname, 31);
        entry->hostname[31] = '\0';
    }
    
    if (vendor) {
        strncpy(entry->vendor, vendor, 31);
        entry->vendor[31] = '\0';
    }
    
    entry->first_seen = time_us_32() / 1000000;
    entry->last_seen = entry->first_seen;
    entry->is_favorite = false;
    entry->is_known = true;
    
    db.count++;
    
    // Save to flash
    device_db_save();
    
    return true;
}

bool device_db_find_device(ip_addr_t* ip, device_db_entry_t* entry) {
    if (!ip || !entry) return false;
    
    for (uint32_t i = 0; i < db.count; i++) {
        if (ip_addr_cmp(&db.entries[i].ip, ip)) {
            memcpy(entry, &db.entries[i], sizeof(device_db_entry_t));
            return true;
        }
    }
    
    return false;
}

bool device_db_get_device(uint32_t index, device_db_entry_t* entry) {
    if (index >= db.count || !entry) return false;
    
    memcpy(entry, &db.entries[index], sizeof(device_db_entry_t));
    return true;
}

uint32_t device_db_get_count(void) {
    return db.count;
}

void device_db_clear(void) {
    db.count = 0;
    memset(db.entries, 0, sizeof(db.entries));
    device_db_save();
}

bool device_db_save(void) {
    // Prepare flash
    uint32_t ints = save_and_disable_interrupts();
    
    // Erase sector
    uint32_t flash_addr = XIP_BASE + FLASH_TARGET_OFFSET;
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    
    // Write data
    uint32_t data_size = sizeof(device_db_t);
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t*)&db, data_size);
    
    restore_interrupts(ints);
    
    return true;
}

bool device_db_load(void) {
    // Read from flash
    const device_db_t* flash_db = (const device_db_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
    
    if (flash_db->magic == DEVICE_DB_MAGIC) {
        memcpy(&db, flash_db, sizeof(device_db_t));
        return true;
    }
    
    return false;
}