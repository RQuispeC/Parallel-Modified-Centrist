#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

#define COMMENT "Centrist Clang"
#define RGB_COMPONENT_COLOR 255
#define GPU 1

typedef struct {
    unsigned char red, green, blue;
} PPMPixel;

typedef struct {
    int x, y;
    PPMPixel *data;
} PPMImage;

double rtclock()
{
    struct timezone Tzp;
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, &Tzp);
    if (stat != 0) printf("Error return from gettimeofday: %d",stat);
    return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}


static PPMImage *readPPM(const char *filename) {
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    if (buff[0] != 'P' || buff[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    img = (PPMImage *) malloc(sizeof(PPMImage));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n')
            ;
        c = getc(fp);
    }

    ungetc(c, fp);
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n",
                filename);
        exit(1);
    }

    if (rgb_comp_color != RGB_COMPONENT_COLOR) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n')
        ;
    img->data = (PPMPixel*) malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}

void writePPM(PPMImage *img) {

    fprintf(stdout, "P6\n");
    fprintf(stdout, "# %s\n", COMMENT);
    fprintf(stdout, "%d %d\n", img->x, img->y);
    fprintf(stdout, "%d\n", RGB_COMPONENT_COLOR);
  
    fwrite(img->data, 3 * img->x, img->y, stdout);
    fclose(stdout);
}

void fromPPMToArray(PPMImage *image, int* image_array)
{
    int it;
    for(it = 0; it < (image -> y)*(image -> x); it++)
    {
      image_array[it*3] = image -> data[it].red;
      image_array[it*3 + 1] = image -> data[it].green;
      image_array[it*3 + 2] = image -> data[it].blue;
    }
}

void fromArrayToPPM(PPMImage *image, int* image_array)
{
    int it;
    for(it = 0; it < (image -> y)*(image -> x); it++)
    {
      image -> data[it].red = image_array[it*3];
      image -> data[it].green = image_array[it*3 + 1];
      image -> data[it].blue = image_array[it*3 + 2];
    }
}

void mod_CENTRIST(int *image, int *image_copy, float *hist, int image_x, int image_y) {
    int i, j, x, y, it;
    int size = image_x * image_y * 3;
    
    #pragma omp target device (GPU) map(tofrom:image_copy[:size])
    #pragma omp parallel for collapse(1)
    for(it = 0; it < image_y*image_x; it++) //convert to grayscale
    { 
      int ind = it*3;
      int grayscale = (image_copy[ind]*299 + image_copy[ind + 1]*587 + image_copy[ind + 2]*114)/1000;
      image_copy[ind] = grayscale;
      image_copy[ind + 1] = grayscale;
      image_copy[ind + 2] = grayscale;
    }
    
    #pragma omp target device (GPU) map(tofrom:hist[:512])
    #pragma omp parallel for collapse(1)
    for(i = 0; i < 512; i++) //init histogram
      hist[i] = 0;

    image_x-=2, image_y-=2; //remove pixels of the border of image.
    
    #pragma omp target device (GPU) map(to:image_copy[:size]) map(tofrom:image[:size])
    #pragma omp parallel for collapse(2)
    for(i = 0; i < image_y; i++) //compute centrist image
      for(j = 0; j < image_x; j++)
      {

        float mean = 0.0;
        for(y = i; y <= i + 2; y++)
          for(x = j; x <= j + 2; x++)
            mean += image_copy[((y * (image_x + 2)) + x)*3];
        mean /= 9.0;
        int value = 0, k = 8;
        for(y = i; y <= i+2; y++)
          for(x = j; x <= j+2; x++)
          {
            if(1.0*(image_copy[((y * (image_x + 2)) + x)*3]) >= mean)
              value |= 1<<k;
            k = k - 1;
          }
        image[((i * image_x) + j)*3] = value;
        image[((i * image_x) + j)*3 + 1] = value;
        image[((i * image_x) + j)*3 + 2] = value;
        /*#pragma omp atomic
        hist[value]++;*/
      }
    for(it = 0; it < image_y*image_x; it++) hist[image[it*3]]++;
    
}

int main(int argc, char *argv[]) {

    if( argc != 2 ) {
        printf("Too many or no one arguments supplied.\n");
    }

    double t_start, t_end;
    int i;

    char *filename = argv[1]; //Recebendo o arquivo!;
    PPMImage *image = readPPM(filename);
    PPMImage *image_output = readPPM(filename);
    float *hist = (float *)(malloc(sizeof(float)*512)); //change to 256 for normal centrist
    int *c_image_output = (int *)(malloc(sizeof(int)*(image -> y)*(image -> x)*3));
    int *c_image = (int *)(malloc(sizeof(int)*(image -> y)*(image -> x)*3));

    fromPPMToArray(image, c_image);

    t_start = rtclock();
    mod_CENTRIST(c_image_output, c_image, hist, image_output -> x, image_output -> y);
    t_end = rtclock();
    
    image_output -> x -= 2, image_output -> y -= 2;
    fromArrayToPPM(image_output, c_image_output);
    //writePPM(image_output);

    float dim = (float)((image_output->x) * (image_output->y));
    fprintf(stdout, "Total %0.6lfs\n", t_end - t_start);
    
    //for(i=0; i < 512; i++) printf("%.4f ", hist[i]/dim);

    free(image);
    free(image_output);
    free(c_image);
    free(c_image_output);
}

//https://github.com/omp2ocl/aclang/wiki/2.-Compiler-Options-to-accelerate-the-Applications

