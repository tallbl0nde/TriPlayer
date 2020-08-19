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
# ROMFS: Directory containing data to be added to RomFS
# LIBS: Libraries to link against
# LIBDIRS: Directories of libraries
#---------------------------------------------------------------------------------
TARGET		:=	TriPlayer
BUILD		:=	build
INCLUDES	:=	include ../Common/include ../Common/libs/splash/splash/include libs/dtl/dtl
SOURCES		:=	source	../Common/source
ROMFS		:=	romfs
LIBS		:=  -lAether -lcurl -lmpg123 -lnx -lSQLite `sdl2-config --libs` -lSDL2_ttf `freetype-config --libs` -lSDL2_gfx -lSDL2_image -lSplash -lpng -ljpeg -lwebp
LIBDIRS		:=	$(PORTLIBS) $(LIBNX) $(CURDIR)/libs/Aether $(CURDIR)/libs/json $(CURDIR)/../Common/libs/SQLite $(CURDIR)/../Common/libs/splash

#---------------------------------------------------------------------------------
# Application version
#---------------------------------------------------------------------------------
VER_MAJOR	:= 1
VER_MINOR	:= 0
VER_MICRO	:= 0

#---------------------------------------------------------------------------------
# Options for .nacp information
#---------------------------------------------------------------------------------
APP_TITLE   := 	TriPlayer
APP_AUTHOR	:= 	tallbl0nde
APP_VERSION	:=	$(VER_MAJOR).$(VER_MINOR).$(VER_MICRO)
ICON 		:= 	icon.jpg

#---------------------------------------------------------------------------------
# Options for code generation
#---------------------------------------------------------------------------------
OBJDIR		:=	$(BUILD)/objs
DEPDIR		:=	$(BUILD)/deps
ARCH		:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
INCLUDE		:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include)
ASFLAGS		:=	-g $(ARCH)
LD			:=	$(CXX)
LDFLAGS		:=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH)
LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)
NROFLAGS	:=	--icon=$(CURDIR)/$(ICON) --nacp=$(CURDIR)/$(TARGET).nacp --romfsdir=$(CURDIR)/$(ROMFS)
OUTPUT		:=	$(CURDIR)/$(TARGET)

#---------------------------------------------------------------------------------
# Flags to pass to compiler
#---------------------------------------------------------------------------------
DEFINES		:=	-D__SWITCH__ -D_APPLICATION_ -DVER_MAJOR=$(VER_MAJOR) -DVER_MINOR=$(VER_MINOR) -DVER_MICRO=$(VER_MICRO) -DVER_STRING=\"$(VER_MAJOR).$(VER_MINOR).$(VER_MICRO)\"
CFLAGS		:=	-g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES) $(INCLUDE)
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -std=gnu++2a

#----------------------------------------------------------------------------------------------------------------------
# Definition of variables which store file locations
#----------------------------------------------------------------------------------------------------------------------
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
.PHONY: all clean-all clean libs
#----------------------------------------------------------------------------------------------------------------------
all: $(OUTPUT).nro
$(OUTPUT).nro:	$(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf:	$(OFILES)
	@echo Linking $(TARGET).elf
	@$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@
$(OFILES): | libs

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
# 'libs' compiles all libraries
#----------------------------------------------------------------------------------------------------------------------
libs:
ifeq ($(wildcard $(CURDIR)/libs/Aether/LICENSE),)
	@$(error "'libs/Aether' missing: Please run 'git submodule update --init!")
endif
	@echo --- Compiling Aether ---
	@$(MAKE) -s -C $(CURDIR)/libs/Aether -f $(CURDIR)/libs/Aether/Makefile
	@echo --- Aether Compiled! ---

#----------------------------------------------------------------------------------------------------------------------
# 'clean-all' removes ALL build files
#----------------------------------------------------------------------------------------------------------------------
clean-all: clean
	@echo --- Cleaning Aether ---
	@$(MAKE) --silent -C $(CURDIR)/libs/Aether -f $(CURDIR)/libs/Aether/Makefile clean
	@echo --- Aether Cleaned! ---

#----------------------------------------------------------------------------------------------------------------------
# 'clean' removes ALL app build files (but not libraries)
#----------------------------------------------------------------------------------------------------------------------
clean:
	@echo Cleaning Application build files...
	@rm -rf $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf

#----------------------------------------------------------------------------------------------------------------------
# Define rule recipe `$(TREE)` (creates directories for .o and .d files)
#----------------------------------------------------------------------------------------------------------------------
$(TREE): %:
	@mkdir -p $@
	@mkdir -p $(@:$(OBJDIR)%=$(DEPDIR)%)