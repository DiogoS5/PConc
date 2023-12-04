/******************************************************************************
 * Programacao Concorrente
 * MEEC 21/22
 *
 * Projecto - Parte1
 *                           serial-complexo.c
 *
 * Compilacao: gcc serial-complexo -o serial-complex -lgd
 *
 *****************************************************************************/

#include <gd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include "image-lib.h"
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/* the directories wher output files will be placed */
#define OLD_IMAGE_DIR "./Old-image-dir/"

#define BUFFER_SIZE 100

typedef struct{
	char** files;
	short num_files;
	char* directory; //input files directory
	gdImagePtr in_texture_img;
} ThreadArgs;



char** getFileList(char* directory, int* files_number){
    char* file_list_path = malloc(strlen(directory) + strlen("/image-list.txt") + 1);
    strcpy(file_list_path, directory);
    strcat(file_list_path, "/image-list.txt");
    FILE* file_list = fopen(file_list_path, "r");
    if (file_list == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char* buffer = malloc(BUFFER_SIZE*sizeof(char));
    char** files = malloc(sizeof(char*));

    while(fgets(buffer, BUFFER_SIZE, file_list) != NULL){
        buffer[strcspn(buffer, "\n")] = 0;

        char* file = strdup(buffer);
        files = realloc(files, (*files_number + 1) * sizeof(char *)); //increase the size of the array

        files[*files_number] = file; // Add file to array

        (*files_number)++;
    }
    free(buffer);
    fclose(file_list);

    return files;
}

void* processImage(void* args)
{	
	/* input images */
	gdImagePtr in_img;
	/* output images */
	gdImagePtr out_smoothed_img;
	gdImagePtr out_contrast_img;
	gdImagePtr out_textured_img;
	gdImagePtr out_sepia_img;

	ThreadArgs* thread_args = (ThreadArgs*)args;

	for(int i=0; i<thread_args->num_files; i++){
		printf("image %s\n", thread_args->files[i]);

		/* load of the input file */
		char* full_file = malloc(sizeof(char) * (strlen(thread_args->directory) + strlen(thread_args->files[i]) + 1));
		sprintf(full_file, "%s/%s", thread_args->directory, thread_args->files[i]); //add directory before file name
		in_img = read_jpeg_file(full_file);
		if (in_img == NULL)
		{
			fprintf(stderr, "Impossible to read %s image\n", full_file);
			return NULL;
		}

		out_contrast_img = contrast_image(in_img);
		out_smoothed_img = smooth_image(out_contrast_img);
		out_textured_img = texture_image(out_smoothed_img, thread_args->in_texture_img);
		out_sepia_img = sepia_image(out_textured_img);

		/* save resized */
		char* out_file_name = malloc(sizeof(char) * (strlen(OLD_IMAGE_DIR) + strlen(thread_args->files[i]) + 1));
		sprintf(out_file_name, "%s%s", OLD_IMAGE_DIR, thread_args->files[i]);
		if (write_jpeg_file(out_sepia_img, out_file_name) == 0)
		{
			fprintf(stderr, "Impossible to write %s image\n", out_file_name);
		}

		gdImageDestroy(out_smoothed_img);
		gdImageDestroy(out_sepia_img);
		gdImageDestroy(out_contrast_img);
		gdImageDestroy(in_img);
	}
	free(args);

	pthread_exit(NULL);
}


/******************************************************************************
 * main()
 *
 * Arguments: (none)
 * Returns: 0 in case of sucess, positive number in case of failure
 * Side-Effects: creates thumbnail, resized copy and watermarked copies
 *               of images
 *
 * Description: implementation of the complex serial version
 *              This application only works for a fixed pre-defined set of files
 *
 *****************************************************************************/
int main(int argc, char** argv)
{
	struct timespec start_time_total, end_time_total;
	struct timespec start_time_seq, end_time_seq;
	struct timespec start_time_par, end_time_par;

	clock_gettime(CLOCK_MONOTONIC, &start_time_total);
	clock_gettime(CLOCK_MONOTONIC, &start_time_seq);

	// n insuficiente de argumentos na comm line
	if (argc < 2) {
		fprintf(stderr, "Insuficient number of arguments");
		exit(EXIT_FAILURE);
	}

    char* directory = argv[1];
    /* length of the files array (number of files to be processed	 */
    int files_number = 0;
	/* array containg the names of files to be processed	 */
	char **files = getFileList(directory, &files_number);

	int n_threads = atoi(argv[2]);
	
	// num de threads pedidas é insuficiente
	if (n_threads <= 0) {
		printf("Insuficient number of threads");
		exit(EXIT_FAILURE);
	}

	/* creation of output directories */
	if (create_directory(OLD_IMAGE_DIR) == 0)
		{
			fprintf(stderr, "Impossible to create %s directory\n", OLD_IMAGE_DIR);
			exit(-1);
		}
	
	gdImagePtr in_texture_img = read_png_file("./paper-texture.png");

	/* end of seq time */
	clock_gettime(CLOCK_MONOTONIC, &end_time_seq);
	clock_gettime(CLOCK_MONOTONIC, &start_time_par);

	pthread_t threads[n_threads];

	/* defines number of files each thread should process*/
	int files_per_thread[n_threads];

	for(int i=0; i<n_threads; i++){
		files_per_thread[i] = files_number/n_threads;
	}
	for(int i=0; i<files_number%n_threads; i++){
		files_per_thread[i]++;
	}

	int file_cnt = 0;
	for (int i=0; i<n_threads; i++)
	{	
		ThreadArgs* args = malloc(sizeof(ThreadArgs));
		args->files = malloc(sizeof(char*) * files_per_thread[i]);
		for(int j=0; j<files_per_thread[i]; j++){
			args->files[j] = files[file_cnt++];
		}
		args->num_files = files_per_thread[i];
		args->directory = directory;
		args->in_texture_img = in_texture_img;

		if (pthread_create(&threads[i], NULL, processImage, (void*)args) != 0)
		{
            fprintf(stderr, "Error creating thread for %s\n", files[i]);
			exit(EXIT_FAILURE);
        }
	}

	// Wait for all threads to finish
    for (int i = 0; i < n_threads; i++) 
	{
        if(pthread_join(threads[i], NULL) != 0) 
		{
			fprintf(stderr, "Error joining thread %d\n", i);
			exit(EXIT_FAILURE);
		}
    }

	clock_gettime(CLOCK_MONOTONIC, &end_time_par);
	clock_gettime(CLOCK_MONOTONIC, &end_time_total);

	struct timespec par_time = diff_timespec(&end_time_par, &start_time_par);
	struct timespec seq_time = diff_timespec(&end_time_seq, &start_time_seq);
	struct timespec total_time = diff_timespec(&end_time_total, &start_time_total);
	printf("\tseq \t %10jd.%09ld\n", seq_time.tv_sec, seq_time.tv_nsec);
	printf("\tpar \t %10jd.%09ld\n", par_time.tv_sec, par_time.tv_nsec);
	printf("total \t %10jd.%09ld\n", total_time.tv_sec, total_time.tv_nsec);

	exit(0);
}
