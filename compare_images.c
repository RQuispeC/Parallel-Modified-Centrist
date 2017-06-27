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

void compare(PPMImage *image_1, PPMImage *image_2)
{
  if(image_1 -> x != image_2 -> x || image_1 -> y != image_2 -> y)
  {
    printf("The images have different dimensions\n");
    return;
  }
  for(int i = 0; i < image_1 -> y; i++)
    for(int j = 0; j < image_1 -> x; j++)
      if(image_1 -> data[i * image_1 -> x + j].red != image_2 -> data[i * image_2 -> x + j].red)
      {
          printf("%d %d -> \t%d %d --> \t%d %d --> \t%d : %d\n", i, j, i/32, j/32, i%32, j%32, image_1 -> data[i * image_1 -> x + j].red, image_2 -> data[i * image_2 -> x + j].red);
      }
}
int main(int argc, char *argv[]) {

    if( argc != 3 ) {
        printf("Too many or no one arguments supplied.\n");
    }

    double t_start, t_end;
    int i;

    char *filename_1 = argv[1]; //Recebendo o arquivo!;
    char *filename_2 = argv[2]; //Recebendo o arquivo!;
    PPMImage *image_1 = readPPM(filename_1);
    PPMImage *image_2 = readPPM(filename_2);

    t_start = rtclock();
    compare(image_1, image_2);
    t_end = rtclock();

    free(image_1);
    free(image_2);
}
