DEVELHELP=0
include ../Makefile.tests_common

USEMODULE += shell
USEMODULE += registry
#USEMODULE += eepreg

CFLAGS += -DBYTES_LENGTH=16

#DISABLE_MODULE += auto_init

# chronos is missing a getchar implementation
BOARD_BLACKLIST += chronos

TEST_ON_CI_WHITELIST += all

# Added for FS (to check)
CFLAGS += -DVFS_FILE_BUFFER_SIZE=72 -DVFS_DIR_BUFFER_SIZE=44
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

include $(RIOTBASE)/Makefile.include
