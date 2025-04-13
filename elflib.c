#include "elflib.h"

int map_file(const char *elf_path, char **addr_out, struct stat *st_out, int *fd_out, int write_mode)
{
    int fd;
    struct stat st;
    char *addr;
    
    fd = open(elf_path, write_mode ? O_RDWR : O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening file\n\n\n");
        return 0;
    }
    
    if (fstat(fd, &st) < 0)
    {
        perror("Error getting file stats\n\n\n");
        close(fd);
        return 0;
    }
    
    if ((size_t)st.st_size < sizeof(Elf64_Ehdr))
    {
        fprintf(stderr, "File too small to be a valid ELF file\n\n\n");
        close(fd);
        return 0;
    }
    
    int prot = PROT_READ | (write_mode ? PROT_WRITE : 0);
    int flags = write_mode ? MAP_SHARED : MAP_PRIVATE;
    addr = mmap(NULL, st.st_size, prot, flags, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("Error mapping file\n\n\n");
        close(fd);
        return 0;
    }
    
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)addr;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0)
    {
        fprintf(stderr, "Not a valid ELF file\n\n\n");
        munmap(addr, st.st_size);
        close(fd);
        return 0;
    }
    
    *addr_out = addr;
    *st_out = st;
    *fd_out = fd;
    
    return 1;
}

void unmap_file(char *addr, size_t size, int fd)
{
    if (addr && addr != MAP_FAILED)
        munmap(addr, size);
    if (fd >= 0)
        close(fd);
}

char *get_os_abi(Elf64_Ehdr *ehdr)
{
    switch (ehdr->e_ident[EI_OSABI])
    {
        case ELFOSABI_SYSV: return "UNIX System V"; break;
        case ELFOSABI_HPUX: return "HP-UX"; break;
        case ELFOSABI_NETBSD: return "NetBSD"; break;
        case ELFOSABI_LINUX: return "Linux"; break;
        case ELFOSABI_SOLARIS: return "Solaris"; break;
        case ELFOSABI_IRIX: return "IRIX"; break;
        case ELFOSABI_FREEBSD: return "FreeBSD"; break;
        case ELFOSABI_TRU64: return "TRU64 UNIX"; break;
        case ELFOSABI_ARM: return "ARM"; break;
        case ELFOSABI_STANDALONE: return "Standalone (embedded) application"; break;
        default: return "Unknown OS ABI"; break;
    }
}

char *get_object_type(Elf64_Ehdr *ehdr)
{
    switch (ehdr->e_type)
    {
        case ET_NONE: return "No file type"; break;
        case ET_REL: return "Relocatable file"; break;
        case ET_EXEC: return "Executable file"; break;
        case ET_DYN: return "Shared object file"; break;
        case ET_CORE: return "Core file"; break;
        default: return "Unknown type"; break;
    }
}

char *get_machine(Elf64_Ehdr *ehdr)
{
    switch (ehdr->e_machine)
    {
        case EM_X86_64: return "AMD x86-64 architecture"; break;
        case EM_386: return "Intel 80386"; break;
        case EM_ARM: return "ARM"; break;
        case EM_AARCH64: return "AArch64"; break;
        case EM_MIPS: return "MIPS"; break;
        case EM_PPC: return "PowerPC"; break;
        case EM_PPC64: return "PowerPC 64-bit"; break;
        case EM_SPARC: return "SPARC"; break;
        case EM_SPARCV9: return "SPARC v9 64-bit"; break;
        case EM_IA_64: return "Intel Itanium"; break;
        default: return "Unknown machine"; break;
    }
}

Elf64_Shdr *find_section_by_name(char *elf_start, const char *section_name)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_start;
    Elf64_Shdr *shdr = (Elf64_Shdr *)(elf_start + ehdr->e_shoff);
    
    // Get section header string table
    Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
    const char *sh_strtab_p = elf_start + sh_strtab->sh_offset;
    
    // Search through all sections
    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        const char *current_name = sh_strtab_p + shdr[i].sh_name;
        if (strcmp(current_name, section_name) == 0)
            return &shdr[i];
    }
    
    return NULL;
}

Elf64_Sym *find_symbol_by_name(char *elf_start, const char *symbol_name)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_start;
    Elf64_Shdr *shdr = (Elf64_Shdr *)(elf_start + ehdr->e_shoff);
    
    // Find the symbol table section
    Elf64_Shdr *symtab = NULL;
    Elf64_Shdr *strtab = NULL;
    
    // Get section header string table
    Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
    const char *sh_strtab_p = elf_start + sh_strtab->sh_offset;
    
    // Find symbol table and string table sections
    for (int i = 0; i < ehdr->e_shnum; i++)
    {
        const char *section_name = sh_strtab_p + shdr[i].sh_name;
        if (shdr[i].sh_type == SHT_SYMTAB)
            symtab = &shdr[i];
        else if (strcmp(section_name, ".strtab") == 0)
            strtab = &shdr[i];
        
        if (symtab && strtab)
            break;
    }
    
    if (!symtab || !strtab)
        return NULL;
    
    // Process symbol table
    Elf64_Sym *syms = (Elf64_Sym *)(elf_start + symtab->sh_offset);
    const char *str_tbl = elf_start + strtab->sh_offset;
    int sym_count = symtab->sh_size / symtab->sh_entsize;
    
    // Look for the symbol
    for (int i = 0; i < sym_count; i++)
    {
        const char *sym_name = str_tbl + syms[i].st_name;
        if (sym_name && strcmp(sym_name, symbol_name) == 0)
            return &syms[i];
    }
    
    return NULL;
}

int elf_28865_glava(const char *elf_path)
{
    struct stat st;
    int fd;
    char *addr;
    
    if (!map_file(elf_path, &addr, &st, &fd, 0))
        return 1;
    
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)addr;
    
    printf("  ELF header\n");
    printf("    Magic number:                      %02X %02X %02X %02X (0x%02X %c%c%c)\n", 
           ehdr->e_ident[EI_MAG0], ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3],
           ehdr->e_ident[EI_MAG0], ehdr->e_ident[EI_MAG1], ehdr->e_ident[EI_MAG2], ehdr->e_ident[EI_MAG3]);

    printf("    Class:                             %s\n", ehdr->e_ident[EI_CLASS] == ELFCLASS32 ? "32-bit" : "64-bit");
    printf("    Data:                              %s\n", ehdr->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" : "2's complement, big endian");
    printf("    Version:                           %d (%s)\n", ehdr->e_ident[EI_VERSION], ehdr->e_ident[EI_VERSION] == EV_CURRENT ? "current" : "unknown");
    printf("    OS ABI:                            %s\n", get_os_abi(ehdr));
    printf("    ABI version:                       %d\n", ehdr->e_ident[EI_ABIVERSION]);
    printf("    Type:                              %s\n", get_object_type(ehdr));
    printf("    Machine:                           %s\n", get_machine(ehdr));
    printf("    Version:                           %d (%s)\n", ehdr->e_version, ehdr->e_version == EV_CURRENT ? "current" : "unknown");
    printf("    Entry point address:               0x%lx\n", (unsigned long)ehdr->e_entry);
    printf("    Start of program headers:          %lu\n", (unsigned long)ehdr->e_phoff);
    printf("    Start of section headers:          %lu\n", (unsigned long)ehdr->e_shoff);
    printf("    Flags:                             0x%x\n", ehdr->e_flags);
    printf("    Size of this header:               %d B\n", ehdr->e_ehsize);
    printf("    Size of program headers:           %d B\n", ehdr->e_phentsize);
    printf("    Number of program headers:         %d\n", ehdr->e_phnum);
    printf("    Size of section headers:           %d B\n", ehdr->e_shentsize);
    printf("    Number of section headers:         %d\n", ehdr->e_shnum);
    printf("    Section header string table index: %d\n\n\n", ehdr->e_shstrndx);
    
    unmap_file(addr, st.st_size, fd);
    return 0;
}

int elf_28865_simboli(const char *elf_path)
{
    struct stat st;
    int fd;
    char *addr;
    
    if (!map_file(elf_path, &addr, &st, &fd, 0))
        return 1;
    
    // Find symbol table section
    Elf64_Shdr *symtab_section = find_section_by_name(addr, ".symtab");
    if (!symtab_section || symtab_section->sh_type != SHT_SYMTAB)
    {
        fprintf(stderr, "Symbol table not found in the ELF file\n\n\n");
        unmap_file(addr, st.st_size, fd);
        return 1;
    }
    
    // Find string table section
    Elf64_Shdr *strtab_section = find_section_by_name(addr, ".strtab");
    if (!strtab_section)
    {
        fprintf(stderr, "String table not found in the ELF file\n\n\n");
        unmap_file(addr, st.st_size, fd);
        return 1;
    }
    
    // Process symbol table
    Elf64_Sym *syms = (Elf64_Sym *)(addr + symtab_section->sh_offset);
    const char *str_tbl = addr + strtab_section->sh_offset;
    int sym_count = symtab_section->sh_size / symtab_section->sh_entsize;
    
    printf("  [Nr]   Value           Size    Name\n");
    int func_count = 0;
    
    for (int i = 0; i < sym_count; i++)
    {
        // Check if symbol is a function and is larger than 50 bytes
        if (ELF64_ST_TYPE(syms[i].st_info) == STT_FUNC && syms[i].st_size > 50)
        {
            const char *sym_name = str_tbl + syms[i].st_name;
            if (sym_name && sym_name[0] != '\0')
                printf("  [%02d]   0x%08lx      %-7lu %s\n", func_count++, (unsigned long)syms[i].st_value, (unsigned long)syms[i].st_size, sym_name);
        }
    }
    
    if (func_count == 0)
        printf("  No functions larger than 50 bytes found in the ELF file\n\n\n");
    else
        printf("\n\n");

    unmap_file(addr, st.st_size, fd);
    return 0;
}

int elf_28865_menjaj(char **argv)
{
    int add_to_values = -1;
    struct stat st;
    int fd;
    char *addr;
    const char *elf_path = argv[2];

    if (!map_file(elf_path, &addr, &st, &fd, 1))
        return 1;
    
    // Find the .data section
    Elf64_Shdr *data_section = find_section_by_name(addr, ".data");
    if (!data_section)
    {
        fprintf(stderr, ".data section not found\n\n\n");
        unmap_file(addr, st.st_size, fd);
        return 1;
    }
    
    printf("  IME     NASLOV     VREDNOST     NOVA_VREDNOST\n");
    
    // Process each variable specified on the command line
    for (int i = 3; argv[i] != NULL; i++)
    {
        const char *var_name = argv[i];
        
        // Find the symbol for this variable
        Elf64_Sym *sym = find_symbol_by_name(addr, var_name);
        if (!sym)
        {
            fprintf(stderr, "  Variable '%s' not found\n", var_name);
            continue;
        }
        
        // Check if this is a variable (object)
        if (ELF64_ST_TYPE(sym->st_info) != STT_OBJECT)
        {
            fprintf(stderr, "  '%s' is not a variable\n", var_name);
            continue;
        }
        
        // Calculate the offset in the file
        uint64_t offset = data_section->sh_offset + (sym->st_value - data_section->sh_addr);
        
        // Modify based on size
        if (sym->st_size == sizeof(int))
        {
            int *value = (int *)(addr + offset);
            int old_value = *value;
            *value += add_to_values;
            printf("  %-6s 0x%08lx    0x%-8x 0x%-8x\n", var_name, (unsigned long)sym->st_value, old_value, *value);
        }
        else if (sym->st_size == sizeof(long))
        {
            long *value = (long *)(addr + offset);
            long old_value = *value;
            *value += add_to_values;
            printf("  %-6s 0x%08lx    0x%-8lx 0x%-8lx\n", var_name, (unsigned long)sym->st_value, old_value, *value);
        }
        else if (sym->st_size == sizeof(char))
        {
            unsigned char *value = (unsigned char *)(addr + offset);
            unsigned char old_value = *value;
            *value += add_to_values;
            printf("  %-6s 0x%08lx    0x%-8x 0x%-8x\n", var_name, (unsigned long)sym->st_value, old_value, *value);
        }
        else if (sym->st_size == sizeof(short))
        {
            short *value = (short *)(addr + offset);
            short old_value = *value;
            *value += add_to_values;
            printf("  %-6s 0x%08lx    0x%-8x 0x%-8x\n", var_name, (unsigned long)sym->st_value, old_value, *value);
        }
        else
            fprintf(stderr, "  Unsupported variable size for '%s': %lu bytes\n", var_name, (unsigned long)sym->st_size);
    }
    printf("\n\n");
    
    unmap_file(addr, st.st_size, fd);
    return 0;
}