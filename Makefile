.PHONY: all all-nxlink clean

all:
	@$(MAKE) -C Common/libs/SQLite
	@$(MAKE) -C Application/
	@$(MAKE) -C Overlay/
	@$(MAKE) -C Sysmodule/
	@mkdir -p sdcard

	@mkdir -p sdcard/switch/TriPlayer
	@cp Application/TriPlayer.nro sdcard/switch/TriPlayer

	@mkdir -p sdcard/switch/.overlays
	@cp Overlay/ovl-triplayer.ovl sdcard/switch/.overlays

	@mkdir -p sdcard/atmosphere/contents/01000054726900FF/flags
	@touch sdcard/atmosphere/contents/01000054726900FF/flags/boot2.flag
	@cp Sysmodule/sys-triplayer.nsp sdcard/atmosphere/contents/01000054726900FF/exefs.nsp

	@echo "Done!\nCopy the contents of ./sdcard to your sdcard :)"

all-nxlink:
	@$(MAKE) -C Common/libs/SQLite NXLINK=1
	@$(MAKE) -C Application/ NXLINK=1
	@$(MAKE) -C Overlay/ NXLINK=1
	@$(MAKE) -C Sysmodule/ NXLINK=1
	@mkdir -p sdcard

	@mkdir -p sdcard/switch/TriPlayer
	@cp Application/TriPlayer.nro sdcard/switch/TriPlayer

	@mkdir -p sdcard/switch/.overlays
	@cp Overlay/ovl-triplayer.ovl sdcard/switch/.overlays

	@mkdir -p sdcard/atmosphere/contents/01000054726900FF/flags
	@touch sdcard/atmosphere/contents/01000054726900FF/flags/boot2.flag
	@cp Sysmodule/sys-triplayer.nsp sdcard/atmosphere/contents/01000054726900FF/exefs.nsp

	@echo "Done!\nCopy the contents of ./sdcard to your sdcard :)"
	@echo "NXlink enabled!"

clean:
	@$(MAKE) -C Common/libs/SQLite clean
	@$(MAKE) -C Application/ clean
	@$(MAKE) -C Overlay/ clean
	@$(MAKE) -C Sysmodule/ clean
	@rm -rf sdcard