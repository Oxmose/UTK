DEP_INCLUDES = -I ../../../lib/includes
DEP_INCLUDES += -I ../../../io/includes
DEP_INCLUDES += -I ../../../global
DEP_INCLUDES += -I ../../cpu/i386/includes
DEP_INCLUDES += -I includes
DEP_INCLUDES += -I ../includes

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../../../tests/includes
endif

DEP_LIBS=