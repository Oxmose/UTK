DEP_INCLUDES = -I ../lib/includes
DEP_INCLUDES += -I ../arch/cpu/includes
DEP_INCLUDES += -I ../arch/board/includes
DEP_INCLUDES += -I ../global
DEP_INCLUDES += -I ../io/includes

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../tests/includes
endif