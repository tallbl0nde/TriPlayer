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
# DATA: Directory containing binary data
# LIBS: Libraries to link against
# LIBDIRS: Directories of libraries
#---------------------------------------------------------------------------------
TARGET		:=	ovl-TriPlayer
BUILD		:=	build
INCLUDES	:=	include build/hdrs ../Common/include libs/libTesla/include
SOURCES		:=	source ../Common/source
DATA		:=	data
LIBS		:=  -lnx -lpng -lz
LIBDIRS		:=	$(PORTLIBS) $(LIBNX) $(CURDIR)/../Common/libs/SQLite

#---------------------------------------------------------------------------------
# Options for .nacp information
#---------------------------------------------------------------------------------
APP_TITLE   := 	TriPlayer
APP_AUTHOR	:= 	tallbl0nde
APP_VERSION	:=	$(VER_MAJOR).$(VER_MINOR).$(VER_MICRO)

#---------------------------------------------------------------------------------
# Options for code generation
#---------------------------------------------------------------------------------
HEADDIR		:=  $(BUILD)/hdrs
OBJDIR		:=	$(BUILD)/objs
DEPDIR		:=	$(BUILD)/deps
ARCH		:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
INCLUDE		:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include)
ASFLAGS		:=	-g $(ARCH)
LD			:=	$(CXX)
LDFLAGS		:=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH)
LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
NROFLAGS	:=	--nacp=$(CURDIR)/$(TARGET).nacp
OUTPUT		:=	$(CURDIR)/$(TARGET)

#---------------------------------------------------------------------------------
# Flags to pass to compiler
#---------------------------------------------------------------------------------
DEFINES		:=	-D__SWITCH__ -D_OVERLAY_ -DVER_MAJOR=$(VER_MAJOR) -DVER_MINOR=$(VER_MINOR) -DVER_MICRO=$(VER_MICRO) -DVER_STRING=\"$(VER_MAJOR).$(VER_MINOR).$(VER_MICRO)\"
CFLAGS		:=	-g -Wall -Os -ffunction-sections $(ARCH) $(DEFINES) $(INCLUDE)
CXXFLAGS	:=	$(CFLAGS) -fno-exceptions -std=gnu++2a

#----------------------------------------------------------------------------------------------------------------------
# Definition of variables which store file locations
#----------------------------------------------------------------------------------------------------------------------
BINFILES	:= $(shell find $(DATA)/ -type f)
OFILES_BIN	:= $(addsuffix .o,$(BINFILES:$(DATA)/%=$(OBJDIR)/%))
HFILES_BIN	:= $(addsuffix .h,$(subst .,_,$(BINFILES:$(DATA)/%=$(HEADDIR)/%)))
CFILES		:= $(foreach dir,$(SOURCES),$(shell find $(dir)/ -name "*.c"))
CPPFILES	:= $(foreach dir,$(SOURCES),$(shell find $(dir)/ -name "*.cpp"))
OFILES		:= $(filter %.o, $(foreach dir,$(SOURCES),$(CPPFILES:$(dir)/%.cpp=$(OBJDIR)/%.o)))
OFILES		+= $(filter %.o, $(foreach dir,$(SOURCES),$(CFILES:$(dir)/%.c=$(OBJDIR)/%.o)))
DEPS		:= $(filter %.d, $(foreach dir,$(SOURCES),$(CPPFILES:$(dir)/%.cpp=$(DEPDIR)/%.d)))
TREE		:= $(sort $(patsubst %/,%,$(dir $(OFILES))))
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
.PHONY: all clean $(HEADDIR)
#----------------------------------------------------------------------------------------------------------------------
all: $(OUTPUT).ovl
$(OUTPUT).ovl:	$(OUTPUT).elf $(OUTPUT).nacp
	@elf2nro $< $@ $(NROFLAGS)
	@echo "Built: $(notdir $(OUTPUT).ovl)"
$(OUTPUT).elf:	$(OFILES)
	@echo Linking $(TARGET).elf
	@$(LD) $(LDFLAGS) $(OFILES) $(OFILES_BIN) $(LIBPATHS) $(LIBS) -o $@

$(OFILES): | $(OFILES_BIN)
$(OFILES_BIN): $(HFILES_BIN)
$(HFILES_BIN): | $(HEADDIR)

.SECONDEXPANSION:
define crule
$(OBJDIR)/%.o: $(1)/%.c | $$$$(@D)
	@echo Compiling $$*.o...
	@$(CC) -MMD -MP -MF $$(@:$$(OBJDIR)/%.o=$$(DEPDIR)/%.d) $$(CFLAGS) -o $$@ -c $$<
endef
define cpprule
$(OBJDIR)/%.o: $(1)/%.cpp | $$$$(@D)
	@echo Compiling $$*.o...
	@$(CXX) -MMD -MP -MF $$(@:$$(OBJDIR)/%.o=$$(DEPDIR)/%.d) $$(CXXFLAGS) -o $$@ -c $$<
endef

$(foreach source,$(SOURCES),$(eval $(call crule,$(source))))
$(foreach source,$(SOURCES),$(eval $(call cpprule,$(source))))

#----------------------------------------------------------------------------------------------------------------------
# 'clean' removes ALL overlay build files
#----------------------------------------------------------------------------------------------------------------------
clean:
	@echo Cleaning Overlay build files...
	@rm -rf $(BUILD) $(TARGET).ovl $(TARGET).nacp $(TARGET).elf

#----------------------------------------------------------------------------------------------------------------------
# Define rule recipe `$(TREE)` (creates directories for .o and .d files)
#----------------------------------------------------------------------------------------------------------------------
$(TREE): %:
	@mkdir -p $@
	@mkdir -p $(@:$(OBJDIR)%=$(DEPDIR)%)
$(HEADDIR):
	@mkdir -p $@

#----------------------------------------------------------------------------------------------------------------------
# Rules to convert .png in /data to .o
#----------------------------------------------------------------------------------------------------------------------
$(HEADDIR)/%_png.h:	$(DATA)/%.png
	@echo "#include <cstddef>\nextern const uint8_t" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' -e 's/[^A-Za-z0-9_]/_/g')`"_end[];" > $@
	@echo "extern const uint8_t" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' -e 's/[^A-Za-z0-9_]/_/g')`"[];" >> $@
	@echo "extern const uint32_t" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' -e 's/[^A-Za-z0-9_]/_/g')`_size";" >> $@

$(OBJDIR)/%.png.o:	$(DATA)/%.png
	@echo "Converting $<..."
	@bin2s $< | $(AS) -o $(@)