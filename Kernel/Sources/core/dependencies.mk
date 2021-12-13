DEP_INCLUDES = -I ../lib/libc/includes
DEP_INCLUDES += -I ../lib/libapi/includes
DEP_INCLUDES += -I ../lib/libstruct/includes
DEP_INCLUDES += -I ../arch/cpu/includes
DEP_INCLUDES += -I ../arch/board/includes
DEP_INCLUDES += -I ../global
DEP_INCLUDES += -I ../io/includes
DEP_INCLUDES += -I ../time/includes
DEP_INCLUDES += -I ../fs/includes

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../tests/includes
endif