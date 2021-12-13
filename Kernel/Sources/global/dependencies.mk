DEP_INCLUDES=

DEP_LIBS=  -larch -lstruct -lcore -lio -lapi -llibc -ltime -lfs

DEP_MODULES = -L../arch/bin -L../lib/bin -L../io/bin -L../core/bin \
              -L../time/bin -L../fs/bin

ifeq ($(TESTS), TRUE)
DEP_LIBS    += -ltests -lfs -llibc -larch -lstruct 
DEP_MODULES += -L../tests/bin
endif