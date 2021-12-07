DEP_INCLUDES =  -I ../global
DEP_INCLUDES += -I ../lib/libc/includes
DEP_INCLUDES += -I ../lib/libapi/includes
DEP_INCLUDES += -I ../io/includes
DEP_INCLUDES += -I ../core/includes
DEP_INCLUDES += -I ../arch/cpu/includes

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../tests/includes
endif