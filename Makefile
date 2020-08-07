.PHONY: all clean

all:
	@echo -e '\033[1m>> Common (Splash)\033[0m'
	@$(MAKE) -s -C Common/libs/splash
	@echo -e '\033[1m>> Common (SQLite)\033[0m'
	@$(MAKE) -s -C Common/libs/SQLite
	@echo -e '\033[1m>> Application\033[0m'
	@$(MAKE) -s -C Application/
	@echo -e '\033[1m>> Overlay\033[0m'
	# @$(MAKE) -C Overlay/
	@echo -e '\033[1m>> Sysmodule\033[0m'
	@$(MAKE) -s -C Sysmodule/
	@echo -e '\033[1m>> SD Card\033[0m'
	@mkdir -p sdcard

	@mkdir -p sdcard/switch/TriPlayer
	@cp Application/TriPlayer.nro sdcard/switch/TriPlayer

	# @mkdir -p sdcard/switch/.overlays
	# @cp Overlay/ovl-triplayer.ovl sdcard/switch/.overlays

	@mkdir -p sdcard/atmosphere/contents/01000054726900FF/flags
	@touch sdcard/atmosphere/contents/01000054726900FF/flags/boot2.flag
	@cp Sysmodule/sys-triplayer.nsp sdcard/atmosphere/contents/01000054726900FF/exefs.nsp

	@echo -e '\033[1m>> Done! Copy ./sdcard to the root of your SD Card :)\033[0m'

clean:
	@echo -e '\033[1m>> Common (SQLite)\033[0m'
	@$(MAKE) -s -C Common/libs/SQLite clean
	@echo -e '\033[1m>> Common (Splash)\033[0m'
	@$(MAKE) -s -C Common/libs/splash clean
	@echo -e '\033[1m>> Application\033[0m'
	@$(MAKE) -s -C Application/ clean-all
	@echo -e '\033[1m>> Overlay\033[0m'
	# @$(MAKE) -C Overlay/ clean
	@echo -e '\033[1m>> Sysmodule\033[0m'
	@$(MAKE) -s -C Sysmodule/ clean
	@echo -e '\033[1m>> SD Card\033[0m'
	@rm -rf sdcard
	@echo -e '\033[1m>> Done!\033[0m'