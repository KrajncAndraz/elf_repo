#ifndef ELFLIB_H
#define ELFLIB_H

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

int map_file(const char *elf_path, char **addr_out, struct stat *st_out, int *fd_out, int write_mode);
void unmap_file(char *addr, size_t size, int fd);
char *get_os_abi(Elf64_Ehdr *ehdr);
char *get_object_type(Elf64_Ehdr *ehdr);
char *get_machine(Elf64_Ehdr *ehdr);
Elf64_Shdr *find_section_by_name(char *elf_start, const char *section_name);
Elf64_Sym *find_symbol_by_name(char *elf_start, const char *symbol_name);

int elf_28865_glava(const char *elf_path);
int elf_28865_simboli(const char *elf_path);
int elf_28865_menjaj(char **argv);

#endif