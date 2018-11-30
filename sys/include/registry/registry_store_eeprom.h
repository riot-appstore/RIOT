#include "registry.h"

typedef struct {
    registry_store_t store;
} registry_eeprom_t;

/* EEPROM Registry Storage */
int registry_eeprom_src(registry_eeprom_t *eeprom);

int registry_eeprom_dst(registry_eeprom_t *eeprom);
