#include <stdlib.h>
#ifndef H_IO_BENCH
#define H_IO_BENCH

/* Prototype de fonction de generation de donnees aleatoires */
void rand_str(char *dest, size_t length);

/* Prototype ecriture sequentielle */
int seq_write(char* file_path, int seq);

/* Prototype ecriture aleatoire */
int rand_write(char* file_path, int seq);

/* Prototype ecriture sequentielle */
int seq_read(char* file_path, int seq);

/* Prototype ecriture aleatoire */
int rand_read(char* file_path, int seq);

#endif /*H_IO_BENCH*/
