CROSS_PATH := $(HOME)/opt/cross/bin

CC := $(CROSS_PATH)/x86_64-elf-gcc
LD := $(CROSS_PATH)/x86_64-elf-ld
CXX := $(CROSS_PATH)/x86_64-elf-g++
AS := $(CROSS_PATH)/x86_64-elf-as
OBJCOPY := $(CROSS_PATH)/x86_64-elf-objcopy

kernel_source_files := $(shell find src/impl/kernel -name *.c)
kernel_object_files := $(patsubst src/impl/kernel/%.c, build/kernel/%.o, $(kernel_source_files))

x86_64_c_source_files := $(shell find src/impl/x86_64 -name *.c)
x86_64_c_object_files := $(patsubst src/impl/x86_64/%.c, build/x86_64/%.o, $(x86_64_c_source_files))

x86_64_asm_source_files := $(shell find src/impl/x86_64 -name *.asm)
x86_64_asm_object_files := $(patsubst src/impl/x86_64/%.asm, build/x86_64/%.o, $(x86_64_asm_source_files))

x86_64_object_files := $(x86_64_c_object_files) $(x86_64_asm_object_files)

common_c_source_files := $(shell find src/impl -maxdepth 1 -name '*.c')
common_object_files := $(patsubst src/impl/%.c, build/%.o, $(common_c_source_files))

# FatFs: compile ff.c, ffunicode.c, ffsystem.c  (NOT diskio.c — we have our own)
fatfs_source_files := include/fatfs/ff.c include/fatfs/ffunicode.c include/fatfs/ffsystem.c
fatfs_object_files := $(patsubst include/fatfs/%.c, build/fatfs/%.o, $(fatfs_source_files))

# Common include flags used by all C files
CFLAGS := -c -ffreestanding -I src/intf -I include/fatfs


run: build-x86_64 rootfs attach-rootfs
	@echo -n ""

build/kernel/%.o: src/impl/kernel/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

build/x86_64/%.o: src/impl/x86_64/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

build/x86_64/%.o: src/impl/x86_64/%.asm
	mkdir -p $(dir $@)
	nasm -f elf64 $< -o $@

build/%.o: src/impl/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

build/fatfs/%.o: include/fatfs/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I include/fatfs -include src/intf/string.h $< -o $@

.PHONY: build-x86_64
build-x86_64: $(kernel_object_files) $(x86_64_object_files) $(common_object_files) $(fatfs_object_files)
	mkdir -p dist/x86_64
	$(LD) -n -o dist/x86_64/kernel.bin -T targets/x86_64/linker.ld \
		$(kernel_object_files) $(x86_64_object_files) $(common_object_files) $(fatfs_object_files)
	cp dist/x86_64/kernel.bin targets/x86_64/iso/boot/kernel.bin
	grub-mkrescue /usr/lib/grub/i386-pc -o dist/x86_64/kernel.iso targets/x86_64/iso
	$(MAKE) rootfs

# Build a FAT32 disk image with /bin/ populated.
# Attach dist/x86_64/rootfs.img as a second hard disk in VirtualBox,
# then at the AdiOS prompt run:  mount 0 /
#
# Requires: dd, mkfs.fat (dosfstools), mmd, mcopy (mtools)
.PHONY: rootfs
rootfs: user_apps
	mkdir -p dist/x86_64
	# Create a 32 MB blank image
	dd if=/dev/zero of=dist/x86_64/rootfs.img bs=1M count=64 status=none
	# Format as FAT32
	mkfs.fat -F 32 -n "ADIOS" dist/x86_64/rootfs.img
	# Create directory structure
	mmd -i dist/x86_64/rootfs.img ::/bin
	mmd -i dist/x86_64/rootfs.img ::/etc
	# Copy any files from rootfs_skel/ if it exists
	@if [ -d rootfs_skel ]; then \
		find rootfs_skel -type f | while read f; do \
			dest=$$(echo $$f | sed 's|^rootfs_skel||'); \
			mcopy -i dist/x86_64/rootfs.img $$f ::$$dest; \
		done; \
	fi
	@echo "rootfs image built: dist/x86_64/rootfs.img"
	VBoxManage closemedium disk dist/x86_64/rootfs.vdi 2>/dev/null || true
	rm -f dist/x86_64/rootfs.vdi
	VBoxManage convertfromraw dist/x86_64/rootfs.img dist/x86_64/rootfs.vdi --format VDI
	@echo "VDI built: dist/x86_64/rootfs.vdi"
	@echo "Attach it as a second hard disk (IDE) in VirtualBox."

VM_NAME := AdiOS
VBOX_STORAGECTL := IDE
VBOX_PORT := 0
VBOX_DEVICE := 1

# --- User-space applications ---

USER_APPS := $(patsubst user/%/,%,$(wildcard user/*/))
USER_EXECS := $(patsubst %,rootfs_skel/bin/%,$(USER_APPS))

USER_CFLAGS := -c -ffreestanding -nostdlib -I src/intf -I user/include
USER_CXXFLAGS := -c -ffreestanding -nostdlib -fno-exceptions -fno-rtti -I src/intf -I user/include

.PHONY: user_apps
user_apps: $(USER_EXECS)

define USER_APP_template
# List of object files for this app
$(1)_OBJS := $(patsubst user/$(1)/%.c,build/user/$(1)/%.o,$(wildcard user/$(1)/*.c)) \
             $(patsubst user/$(1)/%.cpp,build/user/$(1)/%.o,$(wildcard user/$(1)/*.cpp))

# Rule to link the application
# We add build/stdio.o explicitly here so every app gets your print functions
rootfs_skel/bin/$(1): $$($(1)_OBJS) build/stdio.o
	@echo "LD (user) -> $$@"
	mkdir -p $$(dir $$@)
	$(LD) -n -o $$@ -T user/linker.ld $$^

# App-specific C compilation rule
build/user/$(1)/%.o: user/$(1)/%.c
	@echo "CC (user) $$< -> $$@"
	mkdir -p $$(dir $$@)
	$(CC) $(USER_CFLAGS) $$< -o $$@

# App-specific C++ compilation rule
build/user/$(1)/%.o: user/$(1)/%.cpp
	@echo "CXX (user) $$< -> $$@"
	mkdir -p $$(dir $$@)
	$(CXX) $(USER_CXXFLAGS) $$< -o $$@
endef

$(foreach app,$(USER_APPS),$(eval $(call USER_APP_template,$(app))))

# --- End User-space applications ---

.PHONY: clean
clean:
	@rm -rf build dist
	@echo "Cleaned build and dist directories."

# Detach, deregister, and re-attach the rootfs VDI to the VM.
# Run this after 'make rootfs' if VirtualBox complains about a UUID mismatch.
.PHONY: attach-rootfs
attach-rootfs:
	VBoxManage storageattach "$(VM_NAME)" --storagectl "$(VBOX_STORAGECTL)" \
		--port $(VBOX_PORT) --device $(VBOX_DEVICE) --medium none 2>/dev/null || true
	VBoxManage closemedium disk dist/x86_64/rootfs.vdi 2>/dev/null || true
	VBoxManage storageattach "$(VM_NAME)" --storagectl "$(VBOX_STORAGECTL)" \
		--port $(VBOX_PORT) --device $(VBOX_DEVICE) \
		--medium dist/x86_64/rootfs.vdi --type hdd
	@echo "rootfs.vdi attached to $(VM_NAME) on $(VBOX_STORAGECTL) port $(VBOX_PORT) device $(VBOX_DEVICE)"