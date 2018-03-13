NOFORM = "\033[0m"

BLACK  = "\033[1;30m"
RED    = "\033[1;31m"
GREEN  = "\033[1;32m"
YELLOW = "\033[1;33m"
BLUE   = "\033[1;34m"
PURPLE = "\033[1;35m"
CYAN   = "\033[1;36m"
GREY   = "\033[1;37m"

LRED    = "\033[1;91m"
LGREEN  = "\033[1;92m"
LYELLOW = "\033[1;93m"
LBLUE   = "\033[1;94m"
LPURPLE = "\033[1;95m"
LCYAN   = "\033[1;96m"
WHITE   = "\033[1;97m"

BOLD  = "\033[1m"

#--------------------------------options--------------------------------
# the extension of the files that will be compiled
#don't use "" for extension
EXTENSION = c
# the editor you use (only important if you care about make open)
# NOTE: ?= command only defines the variable if not yet defined, so calling   \
        "make version=..." allows you to change the version used without the  \
        need to edit the makefile. This is true for all variables defined like\
        this
EDITOR ?= "code"
# the compiler used
CC = gcc
# directory for .o and dependency files
OBJ_DIR = ./bin/
# version of c++ to use (for deafult c++ define as c++98)

version ?= c11
# final name for the executable
EXECUTABLE = ./prog.exe
# LIB_FILES := ...
# LIB_INCLUDES := ...

# for default compiling with root flags set this to a value different from 0
#root ?= 0

# keep commented for compiling with debbuging flag. Define as: \
	0 for no optimization \
	1 for small optimization \
	2 for normal optimization (recomended) \
	3 for extreme optimization \
	s for size optimization \
  for more information:
#   https://gcc.gnu.org/onlinedocs/gcc-3.2/gcc/Optimize-Options.html
# optimize ?=

#---------------------------------files---------------------------------
C_FILES := $(wildcard ./*.$(EXTENSION))
OBJ_FILES := $(addprefix $(OBJ_DIR),$(notdir $(addsuffix .o, $(basename $(C_FILES)))))
DEP_FILES := $(addprefix $(OBJ_DIR),$(notdir $(addsuffix .d, $(basename $(C_FILES)))))

#---------------------------------flags---------------------------------

ifndef optimize
OPTIMIZATION = -g
else
OPTIMIZATION = -O$(optimize)
endif

#ifneq ($(root),0)
#LD_ROOT = $(shell root-config --libs)
#CC_ROOT = $(shell root-config --cflags)
#endif
# warning flags for linking and compiling
LD_WARNINGS = -Wall -Wextra
CC_WARNINGS = $(LD_WARNINGS)

LD_FLAGS := $(LD_WARNINGS) $(OPTIMIZATION) -std=$(version) $(LD_ROOT)
CC_FLAGS := -MMD $(CC_WARNINGS) $(OPTIMIZATION) -std=$(version) $(CC_ROOT)
# NOTE: -MMD flags creates .d files in the same folder as the .o which contain\
        makefile code with the dependencies for the the compiled file (aka the\
        included files)

#-------------------------------compiling-------------------------------
$(EXECUTABLE): $(OBJ_FILES) $(LIB_FILES)
	@echo $(BOLD) linking for$(NOFORM) $(notdir $@) $(BOLD)depending$(NOFORM) $(notdir $^)
	@$(CC) $(OBJ_FILES) -o $@ $(LD_FLAGS)

$(OBJ_DIR)%.o: ./%.$(EXTENSION)
	@echo $(BOLD) compiling$(NOFORM) $(notdir $@)
	@$(CC) -c -o $@ $< $(CC_FLAGS)

# NOTE: adds the code in the dependency files to this makefile, causing the   \
        C_FILES to depend on the included files and be compiled if one is   \
        changed. the - means this command is silent and will not cause the    \
        make to stop if it fails (which it will if a file has yet to be       \
        compiled and does not have a corresponding .d)
-include $(DEP_FILES)

# Creates OBJ_DIR if does not exist yet
$(OBJ_FILES): | $(OBJ_DIR)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)
	@echo $(CYAN) created $(OBJ_DIR) $(NOFORM)
# NOTE: for information on what the | is doing over there:
# https://www.gnu.org/software/make/manual/make.html#Prerequisite-Types

clean:
	@echo "" cleaning $(RED)  $(notdir $(filter $(EXECUTABLE) $(OBJ_FILES) $(DEP_FILES),$(wildcard ./*) $(wildcard $(OBJ_DIR)*))) $(NOFORM)
	@rm -rf $(OBJ_DIR)
	@rm -f $(EXECUTABLE)

open:
	@echo $(CYAN) opening all files
	@find ./ -type f \( -iname \*.$(EXTENSION) -o -iname \*.h \) -exec $(EDITOR) {} + &
