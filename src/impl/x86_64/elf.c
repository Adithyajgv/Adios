#include "x86_64/elf.h"
#include "print.h"
#include "string.h"
#include "stdlib.h"
#include "ff.h" // For FatFs file operations

// External function to jump to an entry point
extern void jump_to_entry(uint64_t entry_point);

/**
 * Validates an ELF header for 64-bit x86-64 executables.
 * Performs comprehensive checks on magic number, class, encoding, type, and machine.
 */
bool elf_check_supported(Elf64_Ehdr *hdr) {
    // Check magic number
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 ||
        hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 ||
        hdr->e_ident[EI_MAG3] != ELFMAG3) {
        print_str("ELF: Invalid magic number\n");
        return false;
    }
    
    // Check class (64-bit)
    if (hdr->e_ident[EI_CLASS] != ELFCLASS64) {
        print_str("ELF: Not a 64-bit ELF\n");
        return false;
    }
    
    // Check data encoding (little-endian)
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        print_str("ELF: Not little-endian\n");
        return false;
    }
    
    // Check file type (executable)
    if (hdr->e_type != ET_EXEC) {
        print_str("ELF: Not an executable file\n");
        return false;
    }
    
    // Check machine type (x86-64)
    if (hdr->e_machine != EM_X86_64) {
        print_str("ELF: Not an x86-64 executable\n");
        return false;
    }
    
    return true;
}

/**
 * Loads an ELF file from memory and executes it.
 * This function assumes the entire ELF file is already loaded into memory.
 * 
 * @param file_data Pointer to the ELF file data in memory
 * @param execute If true, jumps to entry point after loading
 * @return true if successful, false otherwise
 */
bool elf_load(uint8_t *file_data, bool execute) {
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)file_data;
    
    // Validate the ELF file
    if (!elf_check_supported(hdr)) {
        print_str("ELF: File not supported\n");
        return false;
    }
    
    print_str("ELF: File supported, entry point: 0x");
    print_uint64_hex(hdr->e_entry);
    print_str("\n");
    
    // Iterate over program headers and load PT_LOAD segments
    Elf64_Phdr *phdr = (Elf64_Phdr *)(file_data + hdr->e_phoff);
    for (int i = 0; i < hdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            print_str("ELF: Loading PT_LOAD segment at VADDR 0x");
            print_uint64_hex(phdr[i].p_vaddr);
            print_str(" (file size: 0x");
            print_uint64_hex(phdr[i].p_filesz);
            print_str(", mem size: 0x");
            print_uint64_hex(phdr[i].p_memsz);
            print_str(")\n");
            
            void* load_address = (void*)phdr[i].p_vaddr;
            
            // Copy file data to memory
            memcpy(load_address, file_data + phdr[i].p_offset, phdr[i].p_filesz);
            
            // Zero out any remaining memory (BSS section)
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void*)((uintptr_t)load_address + phdr[i].p_filesz), 0,
                       phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }
    
    print_str("ELF: All segments loaded successfully\n");
    
    // Execute if requested
    if (execute) {
        print_str("ELF: Jumping to entry point...\n");
        jump_to_entry(hdr->e_entry);
        // Should not return
    }
    
    return true;
}

/**
 * Loads an ELF file from the filesystem and executes it.
 * This is a convenience function that reads the file from disk,
 * validates it, loads all PT_LOAD segments, and jumps to the entry point.
 * 
 * WARNING: This implementation is NOT suitable for production use.
 * A real OS would:
 * 1. Create a new address space (page tables) for the user process
 * 2. Map ELF segments into the user's virtual address space
 * 3. Set up a user-mode stack
 * 4. Switch to user mode with appropriate privilege levels
 * 
 * @param path Path to the ELF file on the filesystem
 * @return true if successful, false otherwise
 */
bool elf_load_and_execute(const char* path) {
    FIL file;
    FRESULT res;
    UINT bytes_read;
    Elf64_Ehdr header;
    
    print_str("Attempting to load ELF: ");
    print_str(path);
    print_char('\n');
    
    // Open the file
    res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        print_str("ELF: Failed to open file (FRESULT=");
        if (res >= 10) print_char('0' + (res / 10));
        print_char('0' + (res % 10));
        print_str(")\n");
        return false;
    }
    
    // Read ELF header
    res = f_read(&file, &header, sizeof(Elf64_Ehdr), &bytes_read);
    if (res != FR_OK || bytes_read != sizeof(Elf64_Ehdr)) {
        print_str("ELF: Failed to read ELF header or incomplete read\n");
        f_close(&file);
        return false;
    }
    
    // Validate ELF header
    if (!elf_check_supported(&header)) {
        f_close(&file);
        return false;
    }
    
    print_str("ELF header validated. Entry point: 0x");
    print_uint64_hex(header.e_entry);
    print_str("\nLoading segments...\n");
    
    // Read and load program headers
    Elf64_Phdr program_header;
    for (int i = 0; i < header.e_phnum; ++i) {
        // Seek to program header
        f_lseek(&file, header.e_phoff + i * header.e_phentsize);
        res = f_read(&file, &program_header, sizeof(Elf64_Phdr), &bytes_read);
        if (res != FR_OK || bytes_read != sizeof(Elf64_Phdr)) {
            print_str("ELF: Failed to read program header\n");
            f_close(&file);
            return false;
        }
        
        // Load PT_LOAD segments
        if (program_header.p_type == PT_LOAD) {
            print_str("ELF: Loading segment to VADDR 0x");
            print_uint64_hex(program_header.p_vaddr);
            print_str(" (file size: 0x");
            print_uint64_hex(program_header.p_filesz);
            print_str(", mem size: 0x");
            print_uint64_hex(program_header.p_memsz);
            print_str(")\n");
            
            void* load_address = (void*)program_header.p_vaddr;
            
            // Seek to segment data and read it
            f_lseek(&file, program_header.p_offset);
            res = f_read(&file, load_address, (UINT)program_header.p_filesz, &bytes_read);
            if (res != FR_OK || bytes_read != program_header.p_filesz) {
                print_str("ELF: Failed to load segment data\n");
                f_close(&file);
                return false;
            }
            
            // Zero out BSS section if needed
            if (program_header.p_memsz > program_header.p_filesz) {
                memset((void*)((uintptr_t)load_address + program_header.p_filesz), 0,
                       program_header.p_memsz - program_header.p_filesz);
            }
        }
    }
    
    f_close(&file);
    
    print_str("ELF: All segments loaded successfully\n");
    print_str("ELF: Jumping to entry point...\n");
    
    jump_to_entry(header.e_entry);
    
    return true; // Should not be reached if ELF executes
}