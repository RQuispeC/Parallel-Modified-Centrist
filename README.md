# PARALLELIZATION OF MODIFIED CENSUST TRANSFORMS APPLIED TO FACE RECOGNITION

This project contains the implementation of the Modified Census Transform Histogram. This includes a serial implementation  `centrist_serial.c`, a CUDA implementation `centrist_cuda.cu` and a [Clang](https://github.com/omp2ocl/aclang) implementation `centrist_clang.c`.

## Run Demo of Parallel Implementations

Before running the demo verifiy that commands `nvcc` and  `aclang` run in your system.  It is really important to have your images in `.ppm` format, some test images are available [here](https://drive.google.com/drive/folders/0B-W9xMTkAB8iMHg2Wml4cjlLcUU?usp=sharing). 

To compile the files use:

```shell
make
```

To run serial and parallel implementation on the test images (inside directory `images`) and create a file `output.dat` with the execution time and speed up for each image use:


```shell
make run
```

To remove GPU kernels, binarie and temporary files creates use:

```shell
make clean
```

## Face Recognition

To test face recognition using modified centrist you may have to download The ORL [Database](http://www.cl.cam.ac.uk/research/dtg/attarchive/facedatabase.html) of Faces, unzip it, remove its README file and run `convert_to_ppm.py` script to put images in the right format. 

Then dowload the [binaries](https://drive.google.com/drive/folders/0B-W9xMTkAB8iQU9zODBjeTF0MU0?usp=sharing) or just compile them from the source code.

To run the face recognition tool with serial centrist binaries: 

```shell
faceRecognition.py -p false
```
To run the face recognition tool with parallel centrist binaries: 

```shell
faceRecognition.py -p true
```

## Dependecies and Other Tools

### Parallel Modified Centrist

* CUDA, this implementation has been test on GPU Tesla k40.
* Clang, the repo is [here](https://github.com/omp2ocl/aclang).
* C
* C++ 11
* Make

### Face Recognition

* Python 2.7
* Sci-kit Learn
* Sci-kit Image
* Numpy

### Other Files

* `report.cpp`: Joins the results of the execution times in the test images.
* `join.cpp`: Joins the results of the execution time of the serial and parallel implementation to create `output.dat`.
* `conver_to_ppm.py`: Convert images form ORL Daset to `.ppm`, its README file must be remove before running the script.


This project was presented as part of the course of [Introduction to Parallel Programming](http://oxent2.ic.unicamp.br/node/44) at University of Campinas in the first semester of 2017.
