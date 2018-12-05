#if defined(MODULE_REGISTRY_STORE_FILE)
#include "registry_store_file.h"
#elif defined(MODULE_REGISTRY_STORE_EEPROM)
#include "registry_store_eeprom.h"
#elif defined(MODULE_REGISTRY_STORE_DUMMY)
#include "registry_store_dummy.h"
#endif