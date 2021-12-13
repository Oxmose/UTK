DEP_INCLUDES = -I ../../global
DEP_INCLUDES += -I ../../core/includes
DEP_INCLUDES += -I ../../io/includes
DEP_INCLUDES += -I ../../arch/cpu/includes
DEP_INCLUDES += -I ../../arch/board/includes
DEP_INCLUDES += -I ../libc/includes
DEP_INCLUDES += -I ../libapi/includes
DEP_INCLUDES += -I includes

ifeq ($(TESTS), TRUE)
DEP_INCLUDES += -I ../tests/includes
endif