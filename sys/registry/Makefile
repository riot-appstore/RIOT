MODULE = registry

DIRS += store

ifneq (,$(filter registry_store_dummy,$(USEMODULE)))
  DIRS += store/dummy
endif

ifneq (,$(filter registry_store_eeprom,$(USEMODULE)))
  USEMODULE += registry_store
  DIRS += store/eeprom
endif

ifneq (,$(filter registry_store_file,$(USEMODULE)))
  USEMODULE += registry_store
  DIRS += store/file
endif

include $(RIOTBASE)/Makefile.base
