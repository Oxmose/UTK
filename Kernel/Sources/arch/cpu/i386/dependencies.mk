DEP_INCLUDES = -I ../../../lib/includes
DEP_INCLUDES += -I ../../../io/includes
DEP_INCLUDES += -I ../../../global
DEP_INCLUDES += -I includes
DEP_INCLUDES += -I ../../board/includes
DEP_INCLUDES += -I ../../cpu/includes
DEP_INCLUDES += -I ../../board/x86/includes
DEP_INCLUDES += -I ../../../core/includes
DEP_INCLUDES += -I ../../../memory/includes

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../../../tests/includes
endif

DEP_LIBS= 