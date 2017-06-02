/*
 * Copyright (c) 2013 Igel Co., Ltd
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the University of Tsukuba nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <EfiCommon.h>
#include <EfiApi.h>
#include <Protocol/SimpleFileSystem/SimpleFileSystem.h>
#include <Protocol/LoadedImage/LoadedImage.h>

typedef int EFIAPI entry_func_t (uint32_t loadaddr, uint32_t loadsize,
				 EFI_SYSTEM_TABLE *systab, EFI_HANDLE image,
				 EFI_FILE_HANDLE file);

static EFI_GUID LoadedImageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_GUID FileSystemProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

static void
printhex (EFI_SYSTEM_TABLE *systab, uint64_t val, int width)
{
	CHAR16 msg[2];

	if (width > 1 || val >= 0x10)
		printhex (systab, val >> 4, width - 1);
	msg[0] = L"0123456789ABCDEF"[val & 0xF];
	msg[1] = L'\0';
	systab->ConOut->OutputString (systab->ConOut, msg);
}

static void
print (EFI_SYSTEM_TABLE *systab, CHAR16 *msg, uint64_t val)
{
	systab->ConOut->OutputString (systab->ConOut, msg);
	printhex (systab, val, 8);
	systab->ConOut->OutputString (systab->ConOut, L"\r\n");
}

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	void *tmp;
	uint32_t entry;
	UINTN readsize;
	int boot_error;
	EFI_STATUS status;
	entry_func_t *entry_func;
	EFI_FILE_HANDLE file, file2;
	static CHAR16 file_path[4096];
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileio;
	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
	EFI_PHYSICAL_ADDRESS paddr = 0x40000000;

	status = systab->BootServices->
		HandleProtocol (image, &LoadedImageProtocol, &tmp);
	if (EFI_ERROR (status)) {
		print (systab, L"LoadedImageProtocol ", status);
		return status;
	}
	loaded_image = tmp;
	status = systab->BootServices->
		HandleProtocol (loaded_image->DeviceHandle,
				&FileSystemProtocol, &tmp);
	if (EFI_ERROR (status)) {
		print (systab, L"FileSystemProtocol ", status);
		return status;
	}
    print (systab, L"Hello world ", 0);
    
    /* Now wait for a keystroke before continuing, otherwise your
       message will flash off the screen before you see it.
 
       First, we need to empty the console input buffer to flush
       out any keystrokes entered before this point */
    EFI_SYSTEM_TABLE *ST = systab;
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    Status = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(Status))
        return Status;
 
    /* Now wait until a key becomes available.  This is a simple
       polling implementation.  You could try and use the WaitForKey
       event instead if you like */
    while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY) ;

    return EFI_SUCCESS;
}

