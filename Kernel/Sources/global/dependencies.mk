DEP_INCLUDES=

DEP_LIBS=  -larch -lcore -larch -lio -llib -ltime

DEP_MODULES = -L../arch/bin -L../lib/bin -L../io/bin -L../core/bin -L../time/bin

ifeq ($(TESTS), TRUE)
DEP_LIBS    += -ltests -larch -llib
DEP_MODULES += -L../tests/bin
endif