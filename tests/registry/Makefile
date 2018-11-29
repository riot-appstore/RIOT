DEVELHELP=0
include ../Makefile.tests_common

USEMODULE += shell
USEMODULE += registry

DISABLE_MODULE += auto_init

# chronos is missing a getchar implementation
BOARD_BLACKLIST += chronos

TEST_ON_CI_WHITELIST += all

include $(RIOTBASE)/Makefile.include
