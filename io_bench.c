/*
* AUTHOR: Hamza Ouarnoughi
* Description: un programme qui lit/ecrti un fichier #sequence fois en
*  sequentielle ou aleatoire
* USAGE: ./io_bench <file_path> <read/write> <seq/rnd> <#sequence>
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "io_bench.h"
#include <fcntl.h>

#define MIN_PG_SIZE 1024

int main (int argc, char* argv[])
{
  /* Parametre d'entree */
  int n = 1;
  char pattern[3] = "seq";
  char io_type[5] = "read";
  
  if (argc < 5)
  {
    fprintf(stderr,"USAGE:%s <file_path> <read/write> <seq/rnd> <#sequences>\n", argv[0]);
    return EXIT_FAILURE;
  }
  
  strcpy(io_type, argv[2]);
  strcpy(pattern, argv[3]);
  n = atoi(argv[4]);
  
  /* Verification de type de pattern d'operation */
  if ((strncmp(io_type, "write", sizeof("write")) == 0) || 
		(strncmp(io_type, "WRITE", sizeof("WRITE")) == 0))
  {
	  if ((strncmp(pattern, "rnd", sizeof("rnd")) == 0) || 
			(strncmp(pattern, "RND", sizeof("RND")) == 0))
	  {
		fprintf(stdout, "Random write\n");
		return rand_write(argv[1], n);  
	  }
	  else if ((strncmp(pattern, "seq", sizeof("seq")) == 0) || 
				(strncmp(pattern, "SEQ", sizeof("SEQ")) == 0))
	  {
		  fprintf(stdout, "Sequantial write\n");
		  return seq_write(argv[1], n);
	  }else
	  {
		  fprintf(stderr,"ERROR: unrecognized access pattern (use: seq or rnd)\n");
		  return EXIT_FAILURE;
	  }
  }
  else if ((strcmp(io_type, "read") == 0) || (strcmp(io_type, "READ") == 0))
  {
	  if ((strcmp(pattern, "rnd") == 0) || (strcmp(pattern, "RND") == 0))
	  {
		fprintf(stdout, "Random read\n");
		return rand_read(argv[1], n);  
	  }
	  else if ((strcmp(pattern, "seq") == 0) || (strcmp(pattern, "SEQ") == 0))
	  {
		  fprintf(stdout, "Sequantial read\n");
		  return seq_read(argv[1], n);
	  }
	  else
	  {
		  fprintf(stderr,"ERROR: unrecognized access pattern (use: seq or rnd)\n");
		  return EXIT_FAILURE;
	  }
  }
  else
  {
	  fprintf(stderr,"ERROR: unrecognized access type (use: read or write)\n");
	  return EXIT_FAILURE;
  }

}

/* Fonction de generation de donnees aleatoires */
void rand_str(char *dest, size_t length)
{
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
        length--;
    }
    *dest = '\0';
}

/* Ecrite sequentielle */
int seq_write(char* file_path, int seq)
{
	ssize_t pg_size, f_size, ret;
	struct timeval start, end;
	int fd = 0, j = 0;
	unsigned long nb_page = 0, i = 0;
	long t_time = 0 ,iops = 0;
	float bandwidth = .0, sec_time = .0;
	char *buffer = NULL;
	
	/* Ouvrir le fichier en ecrtiture seule */
	fd = open(file_path, O_WRONLY|O_SYNC);
	//~ fd = open(file_path, O_WRONLY);
	
	if (fd == -1)
	{
		fprintf(stderr,"ERROR while openning file for write\n");
		return EXIT_FAILURE;
	}
	
	/* La taille d'une page RAM */
	pg_size = getpagesize();
	
	/* Allouer de l'espace pour le buffer */
	buffer = (char*) malloc(sizeof(char)*pg_size);
	
	if (buffer == NULL)
	{
		fprintf(stderr,"ERROR: Could not allocate memory for the write\
				buffer\n");
		close(fd);
		return EXIT_FAILURE;
	}
	
	/* La taille du fichier en octets*/
	f_size = lseek(fd, 0L, SEEK_END);
	
	/* Pour resoudre le cas de petites pages */
	if (f_size < pg_size)
		pg_size = MIN_PG_SIZE;
	
	/* Nombre de pages */
	nb_page = f_size / pg_size;
	
	/* Remmetre le pointeur au debut du fichier*/
	lseek(fd, 0L, SEEK_SET);
	
	/* Generation d'un bloc de donnees aleatoires */
	rand_str(buffer, pg_size);
	
	/* Lancer la lecture sequentielle seq fois */
	for (j=0; j<seq; j++)
	{
		/* Initialiser le curseur au debut du fichier */
		lseek(fd, 0L, SEEK_SET);
		
		/* Start chrono */
		gettimeofday(&start, NULL);
		
		for(i=0; i<nb_page; i++)
		{
			//~ /* Generation d'un bloc de donnees aleatoires */
			//~ rand_str(buffer, pg_size);
			/* Deplacer le curseur a cette page */
			lseek(fd, (i*pg_size), SEEK_SET);
			
			/* Ecrire la page generee */
			ret = write(fd,buffer,pg_size);
			
			if (ret != pg_size)
				perror("seq write");
		}
		
		/* Stop chrono */
		gettimeofday(&end, NULL);
		
		t_time = t_time + ((end.tv_sec * 1000000 + end.tv_usec) - \
				(start.tv_sec * 1000000 + start.tv_usec));
	}
	
	/**************** Performances stats *******************/
	
	/* Temps d'execution en seconde */
	sec_time = (float)t_time / 1000000.0;
	
	/* Debit de transfert de donnees */
	bandwidth = (((pg_size / 1024) * nb_page) * seq) / sec_time;
	
	/* IOPS */
	iops = (nb_page * seq) / sec_time;
	
	/* resultats en csv */
	fprintf(stdout, "%d;%.3f;%zu;%.2f;%ld\n",seq,sec_time,(f_size/1024)\
			,bandwidth,iops);
	
	free(buffer);
	close(fd);
	
	return EXIT_SUCCESS;
}

/* Ecriture aleatoire */
int rand_write(char* file_path, int seq)
{
	ssize_t pg_size, f_size, r, ret;
	struct timeval start, end;
	int  fd = 0,j = 0;
	unsigned long  nb_page = 0, i = 0;
	long t_time = 0, iops = 0;
	float bandwidth = .0, sec_time = .0;
	char *buffer = NULL;
	
	/* Ouvrir le fichier en ecriture seule synchrone */
	fd = open(file_path, O_WRONLY|O_SYNC|O_DIRECT);
	if (fd == -1)
	{
		fprintf(stderr,"ERROR while openning file for write \n");
		return EXIT_FAILURE;
	}
	
	/* La taille d'une page RAM*/
	pg_size = getpagesize();
	
	/* Allouer de l'espace pour le buffer */
	buffer = (char*) malloc(sizeof(char)*pg_size);
	
	if (buffer == NULL)
	{
		fprintf(stderr,"ERROR: Could not allocate memory for the write\
				buffer\n");
		close(fd);
		return EXIT_FAILURE;
	}
	
	/* La taille du fichier en octets*/
	f_size = lseek(fd, 0L, SEEK_END);
	
	/* Pour resoudre le cas de petites pages */
	if (f_size < pg_size)
		pg_size = MIN_PG_SIZE;
	
	/* Nombre de pages */
	nb_page = f_size / pg_size;
	
	/* Initialiser le generateur de nombre aleatoire */
	srand(time(NULL));
	
	/* Start chrono */
	gettimeofday(&start, NULL);
	
	/* Lancer la lecture aleatoire */
	for (j=0; j<seq; j++)
	{
		/* Initialiser le debut du fichier */
		lseek(fd, 0L, SEEK_SET);
		
		/* Start chrono */
		gettimeofday(&start, NULL);
		
		for(i=0; i<nb_page; i++)
		{
			/* Generation d'un bloc de donnees aleatoires */
			rand_str(buffer, pg_size);
			
			/* Generer un nombre aleatoire entre 0 et le nombre da pages */
			r = rand()% nb_page;
			
			/* Deplacer le curseur a cette page */
			lseek(fd, (r*pg_size), SEEK_SET);
			
			/* Ecrire la page generee */
			ret = write(fd,buffer,pg_size);
			
			if (ret != pg_size)
				perror("rnd write");
			
		}
		
		/* Stop chrono */
		gettimeofday(&end, NULL);
		t_time = t_time + ((end.tv_sec * 1000000 + end.tv_usec) -\
							(start.tv_sec * 1000000 + start.tv_usec));
	}
	
	/**************** Performances stats *******************/
	
	/* Temps d'execution en secondes */
	sec_time = (float)t_time / 1000000.0;
	
	/* Debit de transfert de donnees */
	bandwidth = (((pg_size / 1024) * nb_page) * seq) / sec_time;
	
	/* IOPS */
	iops = (nb_page * seq) / sec_time;
	
	/* Resultats en CSV */
	fprintf(stdout, "%d;%.3f;%zu;%.2f;%ld\n",seq,sec_time,(f_size/1024)\
			,bandwidth,iops);
	
	free(buffer);
	close(fd);
	
	return EXIT_SUCCESS;
}

/* Lecture sequentielle */
int seq_read(char* file_path, int seq)
{
	/* Pour etre sure de lire tout le fichier */
	size_t pg_size = 0, f_size = 0;
	struct timeval start, end;
	int fd = 0, j = 0;
	unsigned long nb_page = 0, i = 0;
	long t_time = 0 ,iops = 0;
	float bandwidth = .0, sec_time = .0;
	char *buffer = NULL;
	
	/* Ouvrir le fichier en lecture seule */
	fd = open(file_path, O_RDONLY|O_SYNC);
	if (fd == -1)
	{
		fprintf(stderr,"ERROR while openning file\n");
		return EXIT_FAILURE;
	}
	
	/* La taille d'une page RAM*/
	pg_size = getpagesize();
	
	/* Allouer de l'espace pour le buffer */
	buffer = (char*) malloc(sizeof(char)*pg_size);
	
	if (buffer == NULL)
	{
		fprintf(stderr,"ERROR: Could not allocate memory for the buffer\n");
		close(fd);
		return EXIT_FAILURE;
	}
	
	/* La taille du fichier en octets*/
	f_size = lseek(fd, 0L, SEEK_END);
	
	/* Pour resoudre le cas de petites pages */
	if (f_size < pg_size)
		pg_size = MIN_PG_SIZE;
	
	/* Nombre de pages */
	nb_page = f_size / pg_size;
	
	/* Remmetre le pointeur au debut du fichier*/
	lseek(fd, 0L, SEEK_SET);
	
	/* Lancer la lecture aleatoire */
	for (j=0; j<seq; j++)
	{
		/* Initialiser le debut du fichier */
		lseek(fd, 0L, SEEK_SET);
		
		/* Start chrono */
		gettimeofday(&start, NULL);
		
		for(i=0; i<nb_page; i++)
		{
			/* Lire la page courante */
			read(fd,buffer,pg_size);
		}
		
		/* Stop chrono */
		gettimeofday(&end, NULL);
		t_time = t_time + ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
	}
	
	/**************** Performances stats *******************/
	
	/* Temps d'execution en seconde */
	sec_time = (float)t_time / 1000000.0;
	
	/* Debit de transfert de donnees */
	bandwidth = (((pg_size / 1024) * nb_page) * seq) / sec_time;
	
	/* IOPS */
	iops = (nb_page * seq) / sec_time;
	
	/* resultats en csv */
	fprintf(stdout, "%d;%.3f;%zu;%.2f;%ld\n",seq,sec_time,(f_size/1024),bandwidth,iops);
	
	free(buffer);
	close(fd);
	
	return EXIT_SUCCESS;
}

/* Lecture aleatoire */
int rand_read(char* file_path, int seq)
{
	size_t pg_size = 0, f_size = 0 , r = 0;
	struct timeval start, end;
	int  fd = 0,j = 0;
	unsigned long  nb_page = 0, i = 0;
	long t_time = 0, iops = 0;
	float bandwidth = .0, sec_time = .0;
	char *buffer = NULL;
	
	/* Ouvrir le fichier en lecture seule */
	fd = open(file_path, O_RDONLY|O_SYNC);
	if (fd == -1)
	{
		fprintf(stderr,"ERROR while openning file\n");
		return EXIT_FAILURE;
	}
	
	/* La taille d'une page RAM*/
	pg_size = getpagesize();
	
	/* Allouer de l'espace pour le buffer */
	buffer = (char*) malloc(sizeof(char)*pg_size);
	
	if (buffer == NULL)
	{
		fprintf(stderr,"ERROR: Could not allocate memory for the buffer\n");
		close(fd);
		return EXIT_FAILURE;
	}
	
	/* La taille du fichier en octets*/
	f_size = lseek(fd, 0L, SEEK_END);
	
	/* Pour resoudre le cas de petites pages */
	if (f_size < pg_size)
		pg_size = MIN_PG_SIZE;
	
	/* Nombre de pages */
	nb_page = f_size / pg_size;
	
	/* Initialiser le generateur de nombre aleatoire */
	srand(time(NULL));
	
	/* Start chrono */
	gettimeofday(&start, NULL);
	
	/* Lancer la lecture aleatoire */
	for (j=0; j<seq; j++)
	{
		/* Initialiser le debut du fichier */
		lseek(fd, 0L, SEEK_SET);
		
		/* Start chrono */
		gettimeofday(&start, NULL);
		
		for(i=0; i<nb_page; i++)
		{
			/* Generer un nombre aleatoire entre 0 et le nombre da pages */
			r = rand()% nb_page;
			
			/* Deplacer le curseur a cette page */
			lseek(fd, (r*pg_size), SEEK_SET);
			
			/* Lire la page courante */
			read(fd,buffer,pg_size);
		}
		
		/* Stop chrono */
		gettimeofday(&end, NULL);
		t_time = t_time + ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
	}

	/**************** Performances stats *******************/
	
	/* Temps d'execution en secondes */
	sec_time = (float)t_time / 1000000.0;
	
	/* Debit de transfert de donnees */
	bandwidth = (((pg_size / 1024) * nb_page) * seq) / sec_time;
	
	/* IOPS */
	iops = (nb_page * seq) / sec_time;
	
	//~ fprintf(stdout, "%d;%.3f;%d;%.2f;%ld\n",seq,sec_time,((pg_size/1024)*nb_page),bandwidth,iops);
	fprintf(stdout, "%d;%.3f;%zu;%.2f;%ld\n",seq,sec_time,(f_size/1024),bandwidth,iops);
	
	free(buffer);
	close(fd);
	
	return EXIT_SUCCESS;
}

