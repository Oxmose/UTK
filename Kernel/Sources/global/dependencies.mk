DEP_INCLUDES=

DEP_LIBS=  -larch -lcore -larch -lio -llib -ltime -ldrivers

DEP_MODULES = -L../arch/bin -L../lib/bin -L../io/bin -L../core/bin \
              -L../time/bin -L../drivers/bin

ifeq ($(TESTS), TRUE)
DEP_LIBS    += -ltests -larch -ldrivers -llib 
DEP_MODULES += -L../tests/bin
endif