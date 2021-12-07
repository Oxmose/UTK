DEP_INCLUDES = -I ../types/includes
DEP_INCLUDES += -I ../arch/board/includes
DEP_INCLUDES += -I ../lib/libc/includes
DEP_INCLUDES += -I ../lib/libapi/includes
DEP_INCLUDES += -I ../global

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../tests/includes
endif

DEP_LIBS=