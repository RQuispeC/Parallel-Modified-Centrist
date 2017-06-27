#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#define TILE_WIDTH 32

#define COMMENT "Centrist_GPU"
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
//create a thread per pixel
__global__ void mod_CENTRIST(PPMPixel *image_out, PPMPixel *image_cp, int columns, int rows, int *hist, int hist_len) {

	int col = TILE_WIDTH * blockIdx.x + threadIdx.x;
	int row = TILE_WIDTH * blockIdx.y + threadIdx.y;

  __shared__ int hist_private[512];
  int hist_index = (threadIdx.y*TILE_WIDTH + threadIdx.x); //get index in shared histogram
  if(hist_index < hist_len) hist_private[hist_index] = 0;
  __syncthreads();
	if(col < columns && row < rows)
	{
    //create and copy small chunks to shared memory
    __shared__ unsigned char image_cp_private[TILE_WIDTH][TILE_WIDTH];

    //convert to grayscale
		int img_index = row * columns + col; //get index in original image
    int grayscale = (image_cp[img_index].red*299 + image_cp[img_index].green*587 + image_cp[img_index].blue*114)/1000; //avoid float point errors

    image_cp_private[threadIdx.y][threadIdx.x] = grayscale;

    __syncthreads();
    if(col < columns - 2 && row < rows - 2) //ignore first/last row/column
    {
      int r, c, rr, cc;
      float mean = 0.0;
      for(r = threadIdx.y, rr = row; r <= threadIdx.y + 2; r++, rr++)
        for(c = threadIdx.x , cc = col; c <= threadIdx.x + 2; c++, cc++)
        {
          if(r < TILE_WIDTH && c < TILE_WIDTH)
          {
            mean += image_cp_private[r][c];
          }
          else
          {
            int grayscale_neigh = (image_cp[rr*columns + cc].red*299 + image_cp[rr*columns + cc].green*587 + image_cp[rr*columns + cc].blue*114)/1000;
            mean += grayscale_neigh;
          }
        }
      mean /= 9.0;
      int value = 0, k = 8;
      for(r = threadIdx.y, rr = row ; r <= threadIdx.y + 2; r++, rr++)
        for(c = threadIdx.x, cc = col ; c <= threadIdx.x + 2; c++, cc++)
        {
          if(r < TILE_WIDTH && c < TILE_WIDTH)
          {
            if(1.0*image_cp_private[r][c] >= mean)
              value |= 1<<k;
          }
          else
          {
            int grayscale_neigh = (image_cp[rr*columns + cc].red*299 + image_cp[rr*columns + cc].green*587 + image_cp[rr*columns + cc].blue*114)/1000;
            if(grayscale_neigh >= mean)
              value |= 1<<k;
          }
          k--;
        }
      int img_out_ind = row * (columns - 2) + col; //get index in ouput original
      image_out[img_out_ind].red = image_out[img_out_ind].blue = image_out[img_out_ind].green = value;
      atomicAdd(&(hist_private[value]), 1);
    }
    __syncthreads();
    if(hist_index == 0)
    {
      for(int i = 0; i < hist_len; i++)
        atomicAdd(&(hist[i]), hist_private[i]); //init shared histogram
    }
	}
}

int main(int argc, char *argv[]) {

    if( argc != 2 ) {
        printf("Too many or no one arguments supplied.\n");
    }

    double t_start, t_end;
    char *filename = argv[1]; //Recebendo o arquivo!;

    PPMImage *image = readPPM(filename);
    PPMImage *image_output = readPPM(filename);
    int *hist;

    //device data
  	PPMPixel *d_image_output;
    PPMPixel *d_image_copy;
    int *d_hist;

    //total excecution time
  	double offload=0.0 , kernel = 0.0;

  	int i_size = sizeof(PPMPixel) * image->x * image->y;
    int hist_len = 512;
    int hist_size = sizeof(int)*hist_len;

    hist =(int *)malloc(hist_size);
    int i;
    for(i = 0; i < hist_len; i++) hist[i] = 0;

  	// Allocate space for device copies of image and h
  	t_start = rtclock();
  	cudaMalloc(&d_image_output, i_size);
  	cudaMalloc(&d_image_copy, i_size);
    cudaMalloc(&d_hist, hist_size);
  	t_end = rtclock();

    fprintf(stdout, "CudaMalloc %0.6lfs\n", t_end - t_start);
  	offload +=  t_end - t_start;

    //copy inputs to device
  	t_start = rtclock();
  	cudaMemcpy(d_image_output, image_output->data, i_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_image_copy, image->data, i_size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_hist, hist, hist_size, cudaMemcpyHostToDevice);
  	t_end = rtclock();

  	fprintf(stdout, "CopyToDevice  %0.6lfs\n", t_end - t_start);
  	offload +=  t_end - t_start;

    //set grids size
  	dim3 dimGrid(ceil((float)image -> x / TILE_WIDTH), ceil((float)image -> y / TILE_WIDTH), 1);
  	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH, 1);

  	t_start = rtclock();
  	mod_CENTRIST<<<dimGrid, dimBlock>>>(d_image_output, d_image_copy, image-> x, image -> y, d_hist, hist_len);
  	cudaDeviceSynchronize();
  	t_end = rtclock();

  	fprintf(stdout, "Kernel  %0.6lfs\n", t_end - t_start);
  	kernel +=  t_end - t_start;

  	t_start = rtclock();
  	cudaMemcpy(image_output->data, d_image_output, i_size, cudaMemcpyDeviceToHost);
    cudaMemcpy(hist, d_hist, hist_size, cudaMemcpyDeviceToHost);
  	t_end = rtclock();

  	fprintf(stdout, "CopyFromDevice %0.6lfs\n", t_end - t_start);
  	offload +=  t_end - t_start;

    fprintf(stdout, "Offload %0.6lfs\n", offload);
    fprintf(stdout, "CudaTotal %0.6lfs\n", offload + kernel);

    image_output->x -= 2;
    image_output->y -= 2;
    float dim = (float)((image_output->x) * (image_output->y));

    //writePPM(image_output);
    //for(i=0; i < hist_len; i++) printf("%.4f ", hist[i]/dim);

    free(image);
    free(image_output);
    cudaFree(d_image_output), cudaFree(d_image_copy), cudaFree(d_hist);
}


