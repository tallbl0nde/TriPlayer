# This Makefile builds the untouched upstream TagLib as a Switch/libnx
# static library, since their CMake build config wasn't cooperating.
# That's why this Makefile is located outside of the TagLib submodule.
# It's written with as little as possible hard-coded so hopefully it
# will continue to work even if they shuffle around the files later.

#----------------------------------------------------------------------------------------------------------------------
# Default target is 'all'
#----------------------------------------------------------------------------------------------------------------------
.DEFAULT_GOAL := all
#----------------------------------------------------------------------------------------------------------------------

#----------------------------------------------------------------------------------------------------------------------
# Check if DEVKITPRO exists in current environment
#----------------------------------------------------------------------------------------------------------------------
ifndef DEVKITPRO
$(error DEVKITPRO is not present in your environment. This can be fixed by sourcing switchvars.sh from /opt/devkitpro/)
endif
#----------------------------------------------------------------------------------------------------------------------

#----------------------------------------------------------------------------------------------------------------------
# Include switch build toolchain file
#----------------------------------------------------------------------------------------------------------------------
include $(DEVKITPRO)/libnx/switch_rules
#----------------------------------------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Options for compilation
# TARGET: Name of the output file(s)
# BUILD: Directory where object files & intermediate files will be placed
# INCLUDES: List of directories containing header files
# SOURCES: List of directories containing source code
# LIB: Output directory for archive
# OUTPUT: Output file
#---------------------------------------------------------------------------------
TARGET		:=	Tag
BUILD		:=	build
INCLUDES 	:=	$(shell find taglib -type d)
INCLUDE		:=	-I3rdparty $(addprefix -I,$(INCLUDES))
SOURCE		:=	taglib
LIB			:=	lib
OUTPUT		:=	$(LIB)/lib$(TARGET).a

#---------------------------------------------------------------------------------
# Options for code generation
#---------------------------------------------------------------------------------
OBJDIR		:=	$(BUILD)/objs
DEPDIR		:=	$(BUILD)/deps
ARCH		:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
ASFLAGS		:=	-g $(ARCH)
LD			:=	$(CXX)
LDFLAGS		:=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH)

#---------------------------------------------------------------------------------
# Flags to pass to compiler
#---------------------------------------------------------------------------------
CFLAGS		:=	-g -Wall -O2 -ffunction-sections $(ARCH) $(INCLUDE)
# CXXFLAGS	:=	$(CFLAGS) -fno-rtti -std=gnu++17 -fno-exceptions
# NOTE: This library requires both RTTI and exceptions. It should be possible to
#       to remove that requirement if needed, but for now I've just removed the 
#       compiler flags to allow it to build.
CXXFLAGS	:=	$(CFLAGS) -std=gnu++17

#----------------------------------------------------------------------------------------------------------------------
# Definition of variables which store file locations
#----------------------------------------------------------------------------------------------------------------------
CPPFILES	:= $(shell find $(SOURCE)/ -name "*.cpp")
OBJS		:= $(filter %.o, $(CPPFILES:$(SOURCE)/%.cpp=$(OBJDIR)/%.o))
DEPS		:= $(filter %.d, $(CPPFILES:$(SOURCE)/%.cpp=$(DEPDIR)/%.d))
TREE		:= $(sort $(patsubst %/,%,$(dir $(OBJS))))
#----------------------------------------------------------------------------------------------------------------------

#----------------------------------------------------------------------------------------------------------------------
# Include dependent files if they already exist
#----------------------------------------------------------------------------------------------------------------------
ifeq "$(MAKECMDGOALS)" ""
-include $(DEPS)
endif
#----------------------------------------------------------------------------------------------------------------------

#----------------------------------------------------------------------------------------------------------------------
# Define few virtual make targets
#----------------------------------------------------------------------------------------------------------------------
.PHONY: all clean
#----------------------------------------------------------------------------------------------------------------------
all: $(SOURCE)/taglib_config.h $(OUTPUT)

$(SOURCE)/taglib_config.h:
	@cp $(SOURCE)/taglib_config.h.cmake $(SOURCE)/taglib_config.h

# Compiles each object file
.SECONDEXPANSION:
$(OBJDIR)/%.o: $(SOURCE)/%.cpp | $$(@D)
	@echo Compiling $*.o...
	@$(CXX) -MMD -MP -MF $(@:$(OBJDIR)/%.o=$(DEPDIR)/%.d) $(CXXFLAGS) -o $@ -c $<

# Builds the library archive from all .o files
$(OUTPUT): $(OBJS)
	@[ -d $(LIB) ] || mkdir -p $(LIB)
	@rm -rf $(OUTPUT)
	@echo Creating $(TARGET) library archive at $(OUTPUT)...
	@$(AR) -rc $(OUTPUT) $(OBJS)

#----------------------------------------------------------------------------------------------------------------------
# 'clean' removes ALL build files
#----------------------------------------------------------------------------------------------------------------------
clean:
	@echo Cleaning TagLib build files...
	@rm -f $(SOURCE)/taglib_config.h
	@rm -rf $(BUILD) $(LIB)

#----------------------------------------------------------------------------------------------------------------------
# Define rule recipe `$(TREE)` (creates directories for .o and .d files)
#----------------------------------------------------------------------------------------------------------------------
$(TREE): %:
	@mkdir -p $@
	@mkdir -p $(@:$(OBJDIR)%=$(DEPDIR)%)