.DEFAULT_GOAL:=all

GCC = gcc
CFLAGS += -ffunction-sections -fPIC -Werror -Wno-format-truncation -Wall -Wextra -O2 -Isrc/
LDFLAGS = -shared

OUTPUTDIR := $(shell pwd)/output

LIB_NAME := ber_tlv_parser
LIB_SO := lib$(LIB_NAME).so
APP_BIN := ber_tlv_test

$(OUTPUTDIR):
	@mkdir -p $(OUTPUTDIR)

lib_tlv: $(OUTPUTDIR)
	$(GCC) -c -o src/$(LIB_NAME).o src/$(LIB_NAME).c
	$(GCC) $(LDFLAGS) -o $(OUTPUTDIR)/$(LIB_SO) src/$(LIB_NAME).o

app_tlv:
	$(GCC) $(CFLAGS) -c -o app/$(APP_BIN).o app/$(APP_BIN).c
	$(GCC) -o $(OUTPUTDIR)/$(APP_BIN) app/$(APP_BIN).o -L. -lber_tlv_parser

all: $(OUTPUTDIR)
	make lib_tlv
	make app_tlv

clean:
	@rm -rf $(OUTPUTDIR)/*
	@rm -f src/$(LIB_SO) src/*.o
	@rm -f app/$(APP_BIN) app/*.o
