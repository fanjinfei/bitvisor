MINGW1=$(shell which amd64-mingw32msvc-cc 2> /dev/null)
#MINGW1=gcc
MINGW2=$(shell which x86_64-w64-mingw32-gcc 2> /dev/null)
ifneq ("$(MINGW1)","")
	EXE_CC=$(MINGW1)
else ifneq ("$(MINGW2)","")
	EXE_CC=$(MINGW2)
else ifndef EXE_CC
$(error MinGW not found)
endif
RM			= rm -f
EDK_DIR			= ../../edk

.PHONY : all
all : loadvmm.efi

.PHONY : clean
clean :
	$(RM) loadvmm.dll loadvmm.efi

# grub2 { insmod chain, chainloader $(efiroot)/loadvmm.efi }
loadvmm.efi : loadvmm.dll
	(dd bs=1 count=220 && echo && dd bs=1 count=1 of=/dev/null && cat) \
		< loadvmm.dll > loadvmm.efi
	cp loadvmm.dll loadvmm.efi

loadvmm.dll : loadvmm.c
	$(EXE_CC) -shared -nostdlib -e efi_main@8  \
		-Wl,-dll  -Wl,--subsystem,10 -lgcc\
		-I. \
		-I$(EDK_DIR)/Foundation/Efi/Include/ \
		-I$(EDK_DIR)/Foundation/Framework/Include/ \
		-I$(EDK_DIR)/Foundation/Include/ \
		-I$(EDK_DIR)/Foundation/Efi/ \
		-o loadvmm.dll loadvmm.c

test.iso : test.c
	$(EXE_CC) -shared -nostdlib -e efi_main@8  \
		-Wl,-dll  -Wl,--subsystem,10 -lgcc\
		-I. \
		-I$(EDK_DIR)/Foundation/Efi/Include/ \
		-I$(EDK_DIR)/Foundation/Framework/Include/ \
		-I$(EDK_DIR)/Foundation/Include/ \
		-I$(EDK_DIR)/Foundation/Efi/ \
		-o test.efi test.c
	dd if=/dev/zero of=fat.img bs=1k count=1440
	mformat -i fat.img -f 1440 ::
	mmd -i fat.img ::/EFI
	mmd -i fat.img ::/EFI/BOOT
	mcopy -i fat.img test.efi ::/EFI/BOOT
	rm -rf iso
	mkdir iso
	cp fat.img iso
	xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o cdimage.iso iso
