#Almost universal makefile

#directories with other modules (including itself)
WORKING_DIRS := ./ global/
#Name of directory where .o and .d files will be stored
OBJDIR := build
OBJ_DIRS := $(addsuffix $(OBJDIR),$(WORKING_DIRS))

CMD_DEL = rm -rf $(addsuffix /*,$(OBJ_DIRS))
CMD_MKDIR = mkdir -p $(OBJ_DIRS)

ASAN_FLAGS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

WARNING_FLAGS := -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion \
-Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd \
-Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn \
-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast \
-Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector

FORMAT_FLAGS := -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer

CUSTOM_DBG_FLAGS := -D_TREE_DUMP

override CFLAGS := -g -D _DEBUG -ggdb3 -std=c++17 -O0 $(CUSTOM_DBG_FLAGS) -Wall $(WARNING_FLAGS) $(FORMAT_FLAGS) -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla $(ASAN_FLAGS)

CFLAGS_RELEASE := -O3 -std=c++17 -DNDEBUG

BUILD = DEBUG

ifeq ($(BUILD),RELEASE)
	override CFLAGS := $(CFLAGS_RELEASE)
endif
#compilier
ifeq ($(origin CC),default)
	CC=g++
endif

#Name of compiled executable
NAME := ./diff.out
#Name of directory with headers
INCLUDEDIRS := include global/include cJson/include

LINK_LIBS	:= jsonParser

GLOBAL_SRCS     := $(addprefix global/source/, argvProcessor.cpp logger.cpp utils.cpp)
GLOBAL_OBJS     := $(subst source,$(OBJDIR), $(GLOBAL_SRCS:%.cpp=%.o))
GLOBAL_DEPS     := $(GLOBAL_OBJS:%.o=%.d)

# CONTAINER_SRCS  := $(addprefix containers/source/, tree.cpp cList.cpp)
# CONTAINER_OBJS  := $(subst source,$(OBJDIR), $(CONTAINER_SRCS:%.cpp=%.o))
# CONTAINER_DEPS  := $(CONTAINER_OBJS:%.o=%.d)

LOCAL_SRCS      := $(addprefix source/, main.c exprTree.c)
LOCAL_OBJS      := $(subst source,$(OBJDIR), $(LOCAL_SRCS:%.c=%.o))
LOCAL_DEPS      := $(LOCAL_OBJS:%.o=%.d)

#flag to tell compiler where headers are located
override CFLAGS += $(addprefix -I./,$(INCLUDEDIRS)) -L./cJson/build/
#Main target to compile executables
#Filtering other mains from objects
$(NAME): $(GLOBAL_OBJS) $(LOCAL_OBJS)
	$(CC) $(CFLAGS) $^ $(addprefix -l,$(LINK_LIBS)) -o $@

#Easy rebuild in release mode
RELEASE:
	make clean
	make BUILD=RELEASE

#Automatic target to compile object files
#$(OBJS) : $(CUR_DIR)/$(OBJDIR)/%.o : %.cpp
$(GLOBAL_OBJS)     : global/$(OBJDIR)/%.o : global/source/%.cpp
	$(CMD_MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LOCAL_OBJS)      : $(OBJDIR)/%.o : source/%.c
	$(CMD_MKDIR)
	$(CC) $(CFLAGS) -c $< -o $@

#Idk how it works, but is uses compiler preprocessor to automatically generate
#.d files with included headears that make can use
$(GLOBAL_DEPS)     : global/$(OBJDIR)/%.d : global/source/%.cpp
	$(CMD_MKDIR)
	$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

$(LOCAL_DEPS)      : $(OBJDIR)/%.d : source/%.c
	$(CMD_MKDIR)
	$(CC) -E $(CFLAGS) $< -MM -MT $(@:.d=.o) > $@

.PHONY:init
init:
	$(CMD_MKDIR)

#Deletes all object and .d files

.PHONY:clean
clean:
	$(CMD_DEL)

NODEPS = clean

#Includes make dependencies
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
include $(GLOBAL_DEPS)
include $(CONTAINERS_DEPS)
include $(LOCAL_DEPS)
endif