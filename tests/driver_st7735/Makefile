APPLICATION = driver_st7735
include ../Makefile.tests_common

FEATURES_REQUIRED = periph_gpio periph_spi

USEMODULE += xtimer
USEMODULE += st7735

# set default device parameters in case they are undefined
TEST_ST7735_SPI   ?= SPI_DEV\(0\)
TEST_ST7735_CS    ?= GPIO_PIN\(PORT_A,11\)
TEST_ST7735_RESET ?= GPIO_PIN\(PORT_B,12\)
TEST_ST7735_MODE  ?= GPIO_PIN\(PORT_A,12\)

# export parameters
CFLAGS += -DTEST_ST7735_SPI=$(TEST_ST7735_SPI)
CFLAGS += -DTEST_ST7735_CS=$(TEST_ST7735_CS)
CFLAGS += -DTEST_ST7735_RESET=$(TEST_ST7735_RESET)
CFLAGS += -DTEST_ST7735_MODE=$(TEST_ST7735_MODE)

include $(RIOTBASE)/Makefile.include
