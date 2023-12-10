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
#include <unistd.h>

/* the directories wher output files will be placed */
#define OLD_IMAGE_DIR "./Old-image-dir/"

#define BUFFER_SIZE 100 //size of the buffer used to read the file list

/* Structure containing the variables needed for the thread function */
typedef struct{
	char** files;
	short num_files;
	char* directory; //input files directory
	gdImagePtr in_texture_img;
} ThreadArgs;


/**
 * @brief Retrieves the list of files from the user specified directory.
 *
 * @param directory The directory path.
 * @param files_number Pointer to an integer to store the number of files found.
 * @return An array of strings containing the file names.
 */
char** getFileList(char* directory, int* files_number){

    char* file_list_path = malloc(strlen(directory) + strlen("/image-list.txt") + 1); //allocate memory for full path
    strcpy(file_list_path, directory); //copy directory path to full path
    strcat(file_list_path, "/image-list.txt"); //copy file name to full path

    FILE* file_list = fopen(file_list_path, "r");
	//verify if file is correctly opened 
    if (file_list == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char* buffer = malloc(BUFFER_SIZE*sizeof(char));
    char** files = malloc(sizeof(char*)); //initialize array of files

    while(fgets(buffer, BUFFER_SIZE, file_list) != NULL){
        buffer[strcspn(buffer, "\n")] = 0; //remove newline character

        char* file = strdup(buffer); //duplicate buffer string
        files = realloc(files, (*files_number + 1) * sizeof(char *)); //increase the size of the array

        files[*files_number] = file; // add file to array

        (*files_number)++;
    }
	//frees
    free(buffer);
    fclose(file_list);

    return files;
}


/**
 * @brief Processes the input images assigned to a thread.
 * 
 * @param args Pointer to the ThreadArgs structure containing the variables needed for te function.
 * @return void
 */
void* processImage(void* args)
{	
	/* input images */
	gdImagePtr in_img;
	/* output images */
	gdImagePtr out_smoothed_img;
	gdImagePtr out_contrast_img;
	gdImagePtr out_textured_img;
	gdImagePtr out_sepia_img;

	ThreadArgs* thread_args = (ThreadArgs*)args; //cast void* to ThreadArgs*

	for(int i=0; i<thread_args->num_files; i++) 
	{
		printf("image %s\n", thread_args->files[i]);

		/* Verifying existance of file in directory */
		char *verif_name = malloc(sizeof(char)*(strlen(OLD_IMAGE_DIR) + strlen(thread_args->files[i]) + 1)); //allocate memory for full name
		//verificação do malloc
		if (verif_name == NULL){
			printf("Error allocating memory for verif_name\n");
			exit(EXIT_FAILURE);
		}
		// filename with directory
		sprintf(verif_name, "%s%s", OLD_IMAGE_DIR, thread_args->files[i]);
		
		if(access(verif_name, F_OK) != -1) // verificar se existe
		{
			// o ficheiro existe, vamos saltar este processamento
			printf("Processed photo (%s) already exists.\n" , thread_args->files[i]);
			free(verif_name);
			continue;
		}

		/* load of the input file */
		char* full_file = malloc(sizeof(char) * (strlen(thread_args->directory) + strlen(thread_args->files[i]) + 1));
		sprintf(full_file, "%s/%s", thread_args->directory, thread_args->files[i]); //add directory before file name

		in_img = read_jpeg_file(full_file);
		if (in_img == NULL)
		{
			free(verif_name);
			free(full_file);
			continue;
		}

		/* create processed image versions*/
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



/**
 * @brief The main function of the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return The exit status of the program.
 */
int main(int argc, char** argv)
{
	char* directory = argv[1];
	int n_threads = atoi(argv[2]);

	struct timespec start_time_total, end_time_total;
	struct timespec start_time_seq, end_time_seq;
	struct timespec start_time_par, end_time_par;
	struct timespec start_time_threads[n_threads], end_time_threads[n_threads];

	clock_gettime(CLOCK_MONOTONIC, &start_time_total);
	clock_gettime(CLOCK_MONOTONIC, &start_time_seq);

	// Wrong number of arguments
	if (argc < 3) {
		fprintf(stderr, "Wrong number of arguments\n");
		exit(EXIT_FAILURE);
	}
	// Insuficient number of threads
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

    /* length of the files array (number of files to be processed	 */
    int files_number = 0;
	/* array containg the names of files to be processed	 */
	char **files = getFileList(directory, &files_number);

	FILE* timing_file;
	
	/* timing file path */
	char timing_path[40];
    sprintf(timing_path, "%stiming_%d.txt", OLD_IMAGE_DIR, n_threads);
    timing_file = fopen(timing_path, "w");
    
	if (timing_file == NULL) { //verification
        fprintf(stderr, "Error opening timing file for writing\n");
        exit(EXIT_FAILURE);
    }

	
	
	gdImagePtr in_texture_img;
	if(access("./paper-texture.png", F_OK) != -1){
		in_texture_img = read_png_file("./paper-texture.png");
	}
	else{
		char* texture_path = malloc(sizeof(char) * (strlen(directory) + strlen("/paper-texture.png") + 1));
		sprintf(texture_path, "%s/paper-texture.png", directory);
		if(access(texture_path, F_OK) != -1){
			in_texture_img = read_png_file(texture_path);
		}
		else{
			fprintf(stderr, "Impossible to find paper-texture.png\n");
			exit(EXIT_FAILURE);
		}
		free(texture_path);
	}

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

	/* end of seq time, start of par time */
	clock_gettime(CLOCK_MONOTONIC, &end_time_seq);
	clock_gettime(CLOCK_MONOTONIC, &start_time_par);

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

		clock_gettime(CLOCK_MONOTONIC, &start_time_threads[i]);

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

		clock_gettime(CLOCK_MONOTONIC, &end_time_threads[i]);
    }

	clock_gettime(CLOCK_MONOTONIC, &end_time_par);
	clock_gettime(CLOCK_MONOTONIC, &end_time_total);

	struct timespec par_time = diff_timespec(&end_time_par, &start_time_par);
	struct timespec seq_time = diff_timespec(&end_time_seq, &start_time_seq);
	struct timespec total_time = diff_timespec(&end_time_total, &start_time_total);
	printf("\tseq \t %10jd.%09ld\n", seq_time.tv_sec, seq_time.tv_nsec);
	printf("\tpar \t %10jd.%09ld\n", par_time.tv_sec, par_time.tv_nsec);
	printf("total \t %10jd.%09ld\n", total_time.tv_sec, total_time.tv_nsec);
	
	fprintf(timing_file, "total \t %10jd.%02ld\n", total_time.tv_sec, total_time.tv_nsec);
	for (int i = 0; i < n_threads; i++) 
	{
		struct timespec thread_time = diff_timespec(&end_time_threads[i], &start_time_threads[i]);
		printf("Thread_%d: \t %10jd.%09ld\n", i, thread_time.tv_sec, thread_time.tv_nsec);
		fprintf(timing_file, "Thread_%d	\t %jd.%09ld\n", i, thread_time.tv_sec, thread_time.tv_nsec);
	}

	free(files);
	fclose(timing_file);

	exit(0);
}