#include "elflib.h"
#include <stdio.h>
#include <string.h>

void print_usage()
{
    printf("elf_loader [-hlc] [OPTIONS] elf_path\n");
    printf("-h\n  izpis zaglavja zbirke podane v elf_path\n");
    printf("-l\n  izpis vseh funkcij, ki jih najdete v .text sekciji in imajo velikost večjo od 50 zlogov\n");
    printf("-c [spr1,spr2,spr3,...]\n  spreminjanje vrednosti vseh spremeljivk programa za -1, ki jih lahko najdete v programu v elf_path\n");
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        print_usage();
        return 1;
    }

    switch(argv[1][1])
    {
        case 'h':
            elf_28865_glava(argv[2]);
            break;

        case 'l':
            elf_28865_simboli(argv[2]);
            break;

        case 'c':
            elf_28865_menjaj(argv);
            break;

        default:
            print_usage();
            return 1;
    }

    return 0;
 }