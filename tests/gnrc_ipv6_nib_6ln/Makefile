include ../Makefile.tests_common

BOARD_INSUFFICIENT_MEMORY := chronos nucleo-f030 nucleo-l053 nucleo32-f031 \
                             nucleo32-l031 nucleo32-f042 stm32f0discovery \
                             telosb wsn430-v1_3b wsn430-v1_4

USEMODULE += gnrc_ipv6
USEMODULE += gnrc_sixlowpan
USEMODULE += gnrc_sixlowpan_ctx
USEMODULE += gnrc_ipv6_nib_6ln
USEMODULE += gnrc_netif
USEMODULE += embunit
USEMODULE += netdev_ieee802154
USEMODULE += netdev_test

CFLAGS += -DGNRC_NETTYPE_NDP=GNRC_NETTYPE_TEST
CFLAGS += -DGNRC_PKTBUF_SIZE=512
CFLAGS += -DTEST_SUITES

TEST_ON_CI_WHITELIST += all

include $(RIOTBASE)/Makefile.include

test:
	tests/01-run.py
