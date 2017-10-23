WICKR_SDK = wickr-sdk
LOCALREPO = localRepo/$(WICKR_SDK)
SDK_BRANCH = v4.9

ifeq ($(OS),Windows_NT)
    DIR := $(subst C:,,${CURDIR})
    HEADERDIR = "$(subst /,\\,$(DIR)/export/Wickr)"
else
    DIR := ${CURDIR}
    HEADERDIR = $(DIR)/export/Wickr
endif
HEADER_START = "*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*"

default: all
	@echo $(DIR)

update: sdk.update

clean: osx.clean
	rm -rf $(HEADERDIR)

$(WICKR_SDK)/wickr-sdk.pro:
	@echo $(HEADER_START)
	@echo "Starting to install wickr-sdk from GIT"
	git status
	git submodule init wickr-sdk
	git submodule update --recursive wickr-sdk
	cd $(WICKR_SDK); git fetch --all; git checkout $(SDK_BRANCH); git pull
	cd $(WICKR_SDK); make
	@echo "Finished to install wickr-sdk from GIT"
	@echo $(HEADER_END)

sdk.update:
	@echo $(HEADER_START)
	@echo "Update the submodules"
	@echo "Starting to update wickr-sdk"
	cd $(WICKR_SDK); git fetch --all; git checkout $(SDK_BRANCH); git pull; make; make update
	@echo $(HEADER_END)

##########################################################
# OSX build

osx:
	cd $(WICKR_SDK); make osx

osx.release:
	cd $(WICKR_SDK); make osx.release

osx.clean:
	cd $(WICKR_SDK); make osx.clean

osx.install: osx
	@cd $(WICKR_SDK); make osx.install

osx.release.install: osx.release
	@cd $(WICKR_SDK); make osx.release.install


##########################################################
# Windows 32 build

win32:
	cd $(WICKR_SDK); make win32

win32.release:
	cd $(WICKR_SDK); make win32.release

win32.install: win32
	@cd $(WICKR_SDK); make win32.install

win32.release.install: win32.release
	@cd $(WICKR_SDK); make win32.release.install

win32.clean:
	cd $(WICKR_SDK); make win32.clean
	# TODO: Remove the libraries

##########################################################
# Linux build

linux:
	cd $(WICKR_SDK); make linux

linux.release:
	cd $(WICKR_SDK); make linux.release

linux.clean:
	cd $(WICKR_SDK); make linux.clean

linux.install: linux
	@cd $(WICKR_SDK); make linux.install

linux.release.install: linux.release
	@cd $(WICKR_SDK); make linux.release.install


##########################################################
all: $(WICKR_SDK)/wickr-sdk.pro
	@echo done!
