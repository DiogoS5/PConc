#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 100
#define directory "./image-list.txt"
int main(int argc, char** argv){
    FILE* image_list = fopen(argv[1], "r");
    if (image_list == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char* buffer = malloc(BUFFER_SIZE*sizeof(char));
    char** images = malloc(sizeof(char*));
    short num_images = 0;

    while(fgets(buffer, BUFFER_SIZE, image_list) != NULL){
        buffer[strcspn(buffer, "\n")] = 0;

        char* image = strdup(buffer);
        images = realloc(images, (num_images + 1) * sizeof(char *)); //increase the size of the array

        images[num_images] = image; // Add image to array

        num_images++;
    }
    free(buffer);
    fclose(image_list);

    for(int i = 0; i < num_images; i++){
        printf("%s\n", images[i]);
    }
}