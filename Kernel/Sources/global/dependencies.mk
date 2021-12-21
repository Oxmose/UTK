DEP_INCLUDES=

DEP_LIBS=  -larch -lcore -lstruct -lio -llibc -lapi -ltime -lfs

DEP_MODULES = -L../arch/bin -L../lib/bin -L../io/bin -L../core/bin \
              -L../time/bin -L../fs/bin

ifeq ($(TESTS), TRUE)
DEP_LIBS    += -ltests -lapi -lfs -llibc -larch -lstruct -lapi
DEP_MODULES += -L../tests/bin
endif