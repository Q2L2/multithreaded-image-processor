#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

    typedef struct{
        int width;
        int height;
        int max_val;
        unsigned char* data;
    } PPMImage;  // RGB data size = width*height*3;

    typedef struct{
        PPMImage* img;
        int start;
        int end;
    }ThreadData;

PPMImage* load_ppm(const char* filename){
    FILE* F = fopen (filename, "rb"); //read in Binary Mode
    if (F == NULL){
         perror("File can't be opened");
         return NULL;
    }

    char magic[3]; //2 chars + null terminator
    fscanf(F, "%2s", magic); // reads up to 2 characters + '\n'
    if (magic[0] != 'P' || magic[1] != '6'){
        perror ("File type is not supported");
        return NULL;
    }

    //SKIPPING COMMENTS
    int c = fgetc(F); //current character to check
    while (c == '#')
    {
        while (c != '\n' && c != EOF){//E0F: end of file
            //Skip rest of comment line
            c = fgetc(F);
        }
        c=fgetc(F);
    }

    if (c!= EOF) ungetc (c, F);//push back the non-comment character

    int width, height, max_val;
    fscanf(F, "%d", &width);
    fscanf (F, "%d", &height);
    fscanf (F, "%d", &max_val);

    PPMImage* image = malloc(sizeof(PPMImage));
    if (image == NULL){
        perror ("Memory allocation failed");
        return NULL;
    }

    unsigned char* data = malloc(sizeof(unsigned char)*width*height*3);
    if (data == NULL){
        perror("Memory allocation failed");
        return NULL;
    }
    fread (data, sizeof(unsigned char), width*height*3, F);
    fclose(F);

    image->width = width;
    image->height = height;
    image->max_val = max_val;
    image->data = data;
    
    return image;
} 

void save_ppm(const char* filename, const PPMImage* img)
{
    FILE* S = fopen(filename,"wb"); //write in Binary Mode
    if (S == NULL){
        perror("File can't be opened");
        return;
    }

    fprintf(S, "P6\n");
    fprintf(S, "%d %d\n %d\n", img->width, img->height, 
    img->max_val);

    fwrite (img->data, sizeof(unsigned char), 
    img->width*img->height*3, S);
    fclose(S);
    return;
}

//Single-Threaded Filter
void apply_grayscale(PPMImage* img){
    if (img == NULL) return;
    int size = img->width*img->height*3;
    for (int i = 0; i< size; i+=3){
        //Applying gray = 0.3R + 0.59G + 0.11B
        float gray = (float)img->data[i]*0.3
        +(float)img->data[i+1]* 0.59 + (float)img->data[i+2]*0.11;

        img->data[i] = (int)gray;
        img->data[i+1] = (int)gray;
        img->data[i+2] = (int)gray;
    }
}

//Implement multi-threaded filter
void* thread_apply_grayscale(void* arg){
    if (arg == NULL) return NULL;
    ThreadData* D = (ThreadData*) arg;
    int start = D->start;
    int end = D->end;
    for (int i = start; i< end; i+=3){
        int red = D->img->data [i];
        int green = D->img->data [i+1];
        int blue = D->img->data [i+2];
        float gray = (float)red*0.3+(float)green*0.59+
        (float)blue*0.11;
        D->img->data[i]= (int)gray;
        D->img->data[i+1]= (int)gray;
        D->img->data[i+2]= (int)gray;
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    while (argc != 2){
        printf ("Provide PPM image name after the executable file name\n");
    }
    
    char* name = argv[1];
    int len1 = strlen ("../test_images/");
    int len2 = strlen (name);
    char fullname[len1+len2+1];
    strcat (fullname, "../test_images/");
    strcat(fullname, name);
    printf("full name %s\n", fullname);
    //LOADING FILE
     PPMImage* exp = load_ppm(fullname);
    if (exp == NULL){
        printf("Failed to load");
        return 1;
    } 
    printf("width %d, height %d, maxval %d\n", exp->width,
    exp->height, exp->max_val);

     for (int i = 0; i < 10*3; i += 3) {
        printf("Pixel number%d: R=%u G=%u B=%u\n", i/3,
               exp->data[i], exp->data[i+1], exp->data[i+2]);
    }

    struct timespec start1, end1;
    clock_gettime(CLOCK_MONOTONIC, &start1);
    apply_grayscale(exp);
    clock_gettime(CLOCK_MONOTONIC, &end1);

    printf("Processing without thread takes %f\n", (
        end1.tv_sec - start1.tv_sec) + (end1.tv_nsec-start1.tv_nsec)/1e9);

    //VERSION 1 SAVE FILE
    save_ppm ("../test_images/output_1.ppm", exp);

    PPMImage* exp2 = load_ppm("../test_images/west_1.ppm");
    if (exp2 == NULL){
        printf("Failed to load");
        return 1;
    } 
    printf("width %d, height %d, maxval %d\n", exp2->width,
    exp2->height, exp2->max_val);

     for (int i = 0; i < 10*3; i += 3) {
        printf("Pixel number%d: R=%u G=%u B=%u\n", i/3,
               exp2->data[i], exp2->data[i+1], exp2->data[i+2]);
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[8];
    int pixels_per_thread = exp2->width*exp2->height*3/8;
    ThreadData tdata[8];

    for (int t = 0; t<8; t++){
        tdata[t].img = exp2;
        tdata[t].start = t*pixels_per_thread;
        
        tdata[t].end = (t == 7)? exp2->width*exp2->height*3:
        (t+1)*pixels_per_thread;

        pthread_create (&threads[t], NULL, thread_apply_grayscale, &tdata[t]);
    }

    for (int s = 0; s<8; s++){
            pthread_join (threads[s], NULL);
        }
    clock_gettime (CLOCK_MONOTONIC, &end);
    printf("Processing with threads takes %f\n", 
    (end.tv_sec - start.tv_sec) + (end.tv_nsec-start.tv_nsec)/1e9);

    //VERSION 2 SAVE FILE
      save_ppm ("../test_images/output_2.ppm", exp2);

    free(exp->data);
    free(exp);
    free(exp2->data);
    free(exp2);
    printf("\nTesting successful");
    return 0;
}