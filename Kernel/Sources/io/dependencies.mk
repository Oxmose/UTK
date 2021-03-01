DEP_INCLUDES = -I ../types/includes
DEP_INCLUDES += -I ../arch/board/includes
DEP_INCLUDES += -I ../lib/includes/
DEP_INCLUDES += -I ../global

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../tests/includes
endif

DEP_LIBS=