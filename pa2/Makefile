obj-m:=simple_character_module.o

UNAME:=$(shell uname -r)
PWD:=$(shell pwd)

all:
	make -C /lib/modules/${UNAME}/build M=${PWD} modules

clean:
	make -C /lib/modules/${UNAME}/build M=${PWD} clean
