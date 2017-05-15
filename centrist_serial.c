#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

#define COMMENT "Centrist Serial"
#define RGB_COMPONENT_COLOR 255

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

void census_transform(PPMImage *image, PPMImage *image_copy, float *hist) {
    int i, j, x, y;
    // convert image_copy to grayscale
    for(i = 0; i < image_copy -> y; i++)
      for(j = 0; j < image_copy -> x; j++)
      {
        int grayscale = (int)(image_copy -> data[(i * image_copy->x) + j].red*0.299 + image_copy -> data[(i * image_copy->x) + j].green*0.587 + image_copy -> data[(i * image_copy->x) + j].blue*0.114);
        image_copy -> data[(i * image_copy->x) + j].red = grayscale;
        image_copy -> data[(i * image_copy->x) + j].green = grayscale;
        image_copy -> data[(i * image_copy->x) + j].blue = grayscale;
      }

    //init histogram
    for(i = 0; i < 256; i++)
      hist[i] = 0;

    //compute centrist image
    image->x-=2, image->y-=2; //remove pixels of the border of image.
    for(i = 0; i < image -> y; i++)
      for(j = 0; j < image -> x; j++)
      {
        int center = image_copy -> data[((i+1) * image_copy->x) + (j+1)].red;
        int value = 0, k = 7;
        for(y = i; y <= i+2; y++)
          for(x = j; x <= j+2; x++)
          {
            if(y == i+1 && x == j+1) continue;
            if(image_copy -> data[(y * image_copy->x) + x].red >= center)
              value |= 1<<k;
            k--;
          }
        image -> data[(i * image->x) + j].red = value;
        image -> data[(i * image->x) + j].green = value;
        image -> data[(i * image->x) + j].blue = value;
        hist[value]++;
      }

    //end histogram
    for(i = 0; i < 256; i++)
        hist[i] /= (float)(image->x * image -> y);
}

void mod_CENTRIST(PPMImage *image, PPMImage *image_copy, float *hist) {
    int i, j, x, y;
    // convert image_copy to grayscale
    for(i = 0; i < image_copy -> y; i++)
      for(j = 0; j < image_copy -> x; j++)
      {
        int grayscale = (int)(image_copy -> data[(i * image_copy->x) + j].red*0.299 + image_copy -> data[(i * image_copy->x) + j].green*0.587 + image_copy -> data[(i * image_copy->x) + j].blue*0.114);
        image_copy -> data[(i * image_copy->x) + j].red = grayscale;
        image_copy -> data[(i * image_copy->x) + j].green = grayscale;
        image_copy -> data[(i * image_copy->x) + j].blue = grayscale;
      }

    //init histogram
    for(i = 0; i < 512; i++)
            hist[i] = 0;

    //compute centrist image
    image->x-=2, image->y-=2; //remove pixels of the border of image.
    for(i = 0; i < image -> y; i++)
      for(j = 0; j < image -> x; j++)
      {
        float mean = 0.0;
        for(y = i; y <= i + 2; y++)
          for(x = j; x <= j + 2; x++)
            mean += image_copy -> data[(y * image_copy->x) + x].red;
        mean /= 9.0;
        int value = 0, k = 8;
        for(y = i; y <= i+2; y++)
          for(x = j; x <= j+2; x++)
          {
            if(1.0*(image_copy -> data[(y * image_copy->x) + x].red) >= mean)
              value |= 1<<k;
            k--;
          }
        image -> data[(i * image->x) + j].red = value;
        image -> data[(i * image->x) + j].green = value;
        image -> data[(i * image->x) + j].blue = value;
        hist[value]++;
      }

          //end histogram
    for(i = 0; i < 512; i++)
        hist[i] /= (float)(image->x * image -> y);
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
    float* hist = (float *)(malloc(sizeof(float)*512)); //change to 256 for normal centrist

    t_start = rtclock();
    mod_CENTRIST(image_output, image, hist);
    t_end = rtclock();

    //writePPM(image_output);
    //fprintf(stdout, "\n%0.6lfs\n", t_end - t_start);
    for(i=0; i < 512; i++) printf("%.4f ", hist[i]); puts("");

    free(image);
    free(image_output);
}
