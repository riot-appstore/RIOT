DEVELHELP=0
include ../Makefile.tests_common

USEMODULE += shell
USEMODULE += registry
USEMODULE += registry_store
USEMODULE += registry_store_file
#USEMODULE += registry_store_dummy
#USEMODULE += registry_store_eeprom

# Added for FS (to check)
CFLAGS += -DVFS_FILE_BUFFER_SIZE=72 -DVFS_DIR_BUFFER_SIZE=44
CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(4*THREAD_STACKSIZE_DEFAULT\)

# Use MTD (flash layer)
USEMODULE += mtd
USEMODULE += mtd_sdcard
USEMODULE += sdcard_spi
USEMODULE += auto_init_storage
USEMODULE += fatfs_vfs

# Use VFS
USEMODULE += vfs

# Use a file system
USEMODULE += fatfs

CFLAGS += -DCONFIG_REGISTRY_USE_INT64

include $(RIOTBASE)/Makefile.include
