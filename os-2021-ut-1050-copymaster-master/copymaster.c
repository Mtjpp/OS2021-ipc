#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "options.h"



void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);


int main(int argc, char* argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacov
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie
    
    PrintCopymasterOptions(&cpm_options);
    
    //-------------------------------------------------------------------
    // Osetrenie prepinacov pred kopirovanim
    //-------------------------------------------------------------------
    
    if (cpm_options.fast && cpm_options.slow) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    else if (cpm_options.create && cpm_options.overwrite){
        errno = 22;
        FatalError('c', "CHYBA PREPINACOV", 42);
    }
    else if (cpm_options.create && cpm_options.append){
        errno = 22;
        FatalError('c', "CHYBA PREPINACOV", 42);
    }
    else if (cpm_options.append && cpm_options.overwrite){
        errno = 22;
        FatalError('a', "CHYBA PREPINACOV", 42);
    }
    
    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...
    
    //-------------------------------------------------------------------
    // Kopirovanie suborov
    //-------------------------------------------------------------------
    
    // TODO Implementovat kopirovanie suborov
    
    //! INPUT
    int infile;
    infile = open(cpm_options.infile, O_RDONLY);

    if ( infile == -1 ){
        switch(errno){
            case 2:
                if ( cpm_options.link ){
                    FatalError('K', "VSTUPNY SUBOR EXISTUJE", 30);
                }
                else{
                    FatalError('O', "SUBOR NEEXISTUJE", 21);
                }
                break;
            default:
                FatalError('O', "INA CHYBA", 21);
                break;
        }
    }

    //! PRAVA VSTUPNEHO SUBORU
    struct stat st;
    stat(cpm_options.infile, &st);

    //! INODE
    if ( cpm_options.inode ){
        if( cpm_options.inode_number != st.st_ino ){
            FatalError('i', "ZLY INODE", 27);
        }
        if ( S_ISREG(st.st_mode) == 0 ){
            FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);
        }
        if ( errno != 0 ){
            FatalError('i', "INA CHYBA", 27);
        }
    }

    if ( cpm_options.link ){
        int lin = link(cpm_options.infile, cpm_options.outfile);
        if ( lin == -1 ){
            switch(errno){
                case 17:
                    FatalError('K', "VYSTUPNY SUBOR UZ EXISTUJE", 30);
                    break;
                default:
                    FatalError('K', "INA CHYBA", 30);
                    break;
            }
        }
    }

    if ( cpm_options.umask ){
        int change = 0;

        for ( int i = 0; i < 10; i++ ){
            if ( cpm_options.umask_options[i][0] == 0 ){
                break;
            }
            if ( cpm_options.umask_options[i][1] == '-' ){
                if ( cpm_options.umask_options[i][0] == 'u' ){
                    if ( cpm_options.umask_options[i][2] == 'r' ){
                        change = change + 400;
                    }
                    else if ( cpm_options.umask_options[i][2] == 'w' ){
                        change = change + 200;
                    }
                    else{
                        change = change + 100;
                    }
                }

                if ( cpm_options.umask_options[i][0] == 'g' ){
                    if ( cpm_options.umask_options[i][2] == 'r' ){
                        change = change + 40;
                    }
                    else if ( cpm_options.umask_options[i][2] == 'w' ){
                        change = change + 20;
                    }
                    else{
                        change = change + 10;
                    }
                }

                if ( cpm_options.umask_options[i][0] == 'o' ){
                    if ( cpm_options.umask_options[i][2] == 'r' ){
                        change = change + 4;
                    }
                    else if ( cpm_options.umask_options[i][2] == 'w' ){
                        change = change + 2;
                    }
                    else{
                        change = change + 1;
                    }
                }                
            }
        }
        umask(change);
    }

    //! OUTPUT
    int outfile;

    if ( argc == 3 ){
        outfile = open(cpm_options.outfile, O_CREAT | O_TRUNC | O_WRONLY, st.st_mode );
    }
    else if ( cpm_options.create ){
        outfile = open(cpm_options.outfile, O_CREAT | O_EXCL | O_WRONLY, cpm_options.create_mode);
        if ( outfile == -1 ){
            switch(errno){
                case 17:
                    FatalError('c', "SUBOR EXISTUJE", 23);
                    break;
                default:
                    FatalError('c', "INA CHYBA", 23);
                    break;
            }
        }
    }
    else if ( cpm_options.overwrite ){
        outfile = open(cpm_options.outfile, O_TRUNC | O_WRONLY);
        if ( outfile == -1 ){
            switch (errno){
                case 2:
                    FatalError('o', "SUBOR NEEXISTUJE", 24);
                    break;
                default:
                    FatalError('o', "INA CHYBA", 24);
                    break;
            }
        }
    }
    else if ( cpm_options.append ){
        outfile = open(cpm_options.outfile, O_APPEND | O_WRONLY);
        if ( outfile == -1 ){
            switch (errno){
                case 2:
                    FatalError('o', "SUBOR NEEXISTUJE", 22);
                    break;
                default:
                    FatalError('o', "INA CHYBA", 22);
                    break;
            }
        }
    }
    else if ( cpm_options.lseek ){
        outfile = open(cpm_options.outfile, O_CREAT | O_WRONLY);
        if ( outfile == -1 ){
            FatalError('l', "INA CHYBA", 33);
        }

    }
    else{
        outfile = open(cpm_options.outfile, O_CREAT | O_TRUNC | O_WRONLY);
    }

    if ( cpm_options.lseek ){
        long l;
        if ( lseek(infile, cpm_options.lseek_options.pos1, SEEK_SET) == -1 ){
            switch(errno){
                case 22:
                    FatalError('l', "CHYBA POZICIE infile", 33);
                    break;
                default:
                    FatalError('l', "INA CHYBA", 33);
                    break;
            }
        }

        switch( cpm_options.lseek_options.x ){
            case 0:
                if ( (l = lseek(outfile, cpm_options.lseek_options.pos2, cpm_options.lseek_options.x)) == -1 ){
                    switch(errno){
                        case 22:
                            FatalError('l', "CHYBA POZICIE outfile", 33);
                            break;
                        default:
                            FatalError('l', "INA CHYBA", 33);
                            break;
                    }
                }
                break;
            case 1:
                if ( (l = lseek(outfile, cpm_options.lseek_options.pos2, cpm_options.lseek_options.x)) == -1 ){
                    switch(errno){
                        case 22:
                            FatalError('l', "CHYBA POZICIE outfile", 33);
                            break;
                        default:
                            FatalError('l', "INA CHYBA", 33);
                            break;
                    }
                }
                break;
            case 2:
                if ( (l = lseek(outfile, cpm_options.lseek_options.pos2, cpm_options.lseek_options.x)) == -1 ){
                    switch(errno){
                        case 22:
                            FatalError('l', "CHYBA POZICIE outfile", 33);
                            break;
                        default:
                            FatalError('l', "INA CHYBA", 33);
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }

    if ( cpm_options.sparse ){
        ssize_t a;
        char buff;
        for ( int i = 0; i < st.st_size; i++ ){
            if (( a = read(infile, &buff, 1) ) > 0){
                if ( buff == '\0' ){
                    lseek( outfile, 1, SEEK_CUR );
                }
                else{
                    write( outfile, &buff, a );
                }
            }
            else{
                break;
            }
        }
        truncate( cpm_options.outfile, st.st_size );
    }

    //! KOPIROVANIE
    int number;
    if (cpm_options.fast){
        number = 1000000;
    }
    else if (cpm_options.slow){
        number = 1;
    }
    else{
        number = 1000;
    }

    char buffer[number];
    int wri;

    errno = 0;

    if ( cpm_options.lseek ){
        number = cpm_options.lseek_options.num;
        wri = read(infile, &buffer, number);
        write( outfile, &buffer, wri );
    }
    else{
        while (( wri = read(infile, &buffer, number)) > 0 ){
            write( outfile, &buffer, wri );
        }
    }
    
    if ( number == 1000000 || number == 1 ){
        if ( errno != 0 ){
            fprintf(stderr, "INA CHYBA");
        }
    }
    else{
        if ( errno != 0 ){
            fprintf(stderr, "INA CHYBA");
            exit(21);
        }
    }

    if ( cpm_options.chmod ){
        int change = fchmod(outfile, cpm_options.chmod_mode);
        if ( change == -1 ){
            // switch(errno){
            //     case 22:
            //! spytat sa na cviku v utorok!!!
            // }
            printf("afsd");
        }
    }

    if ( cpm_options.delete_opt ){
        if ( S_ISREG(st.st_mode) ){
            if ( cpm_options.create != 1 ){
                fchmod(outfile, st.st_mode );
            }
            int del = remove(cpm_options.infile);
            if ( del == -1 ){
                switch(errno){
                    case 13:
                        FatalError('d', "SUBOR NEBOL ZMAZANY", 26);
                        break;
                    default:
                        FatalError('d', "INA CHYBA", 26);
                        break;
                }
            }
        }
        else{
            FatalError('d', "INA CHYBA", 26);
        }
    }

    if ( cpm_options.truncate ){
        if ( S_ISREG(st.st_mode) ){
            int trun = truncate(cpm_options.infile, cpm_options.truncate_size);
            if ( trun == -1 ){
                switch(errno){
                    case 22:
                        FatalError('t', "ZAPORNA VELKOST", 31);
                        break;
                    default:
                        FatalError('t', "INA CHYBA", 31);
                        break;
                }
            }
        }
        else{
            FatalError('t', "INA CHYBA", 31);
        }
    }


    // cpm_options.infile
    // cpm_options.outfile
    
    //-------------------------------------------------------------------
    // Vypis adresara
    //-------------------------------------------------------------------
    
 
        
    //-------------------------------------------------------------------
    // Osetrenie prepinacov po kopirovani
    //-------------------------------------------------------------------
    
    // TODO Implementovat osetrenie prepinacov po kopirovani
    
    close (outfile);
    close (infile);

    return 0;
}


void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d:%s:%s\n", c, errno, strerror(errno), msg); 
    exit(exit_status);
}

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;
    
    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);
    
    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);
    
    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);
    
    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);
    
    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }
    
    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}
