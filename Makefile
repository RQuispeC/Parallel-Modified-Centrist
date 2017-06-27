nvcc = /usr/local/cuda-8.0/bin/nvcc
c = gcc -Wall -pg
clang_none = aclang -O3 -rtl-mode=profile -opt-poly=none
clang_tile = aclang -O3 -rtl-mode=profile -opt-poly=tile
clang_vect = aclang -O3 -rtl-mode=profile -opt-poly=vectorize
cpp = g++ --std=c++11

all: bin

bin: serial cuda clang join
	@echo ">>>>>>>>>>Files Compiled"

serial:
	$(c) centrist_serial.c -o serial

cuda:
	$(nvcc) centrist_cuda.cu -o paral_cuda

clang:
	$(clang_none) centrist_clang.c -o paral_clang_none
	$(clang_tile) centrist_clang.c -o paral_clang_tile
	$(clang_vect) centrist_clang.c -o paral_clang_vect

join:
	$(cpp) join.cpp -o join

clean: clean-bin clean-tmp

clean-bin:
	rm -f serial paral_clang* paral_cuda join kernel*

clean-tmp:
	rm -f serial_tmp.dat cuda_tmp.dat clang_none_tmp.dat clang_tile_tmp.dat clang_vect_tmp.dat output_tmp.dat tmp.dat

run:
	@./paral_cuda images/image0.ppm > cuda_tmp.dat
	@./paral_clang_none images/image0.ppm > clang_none_tmp.dat
	@./paral_clang_tile images/image0.ppm > clang_tile_tmp.dat
	@./paral_clang_vect images/image0.ppm > clang_vect_tmp.dat
	
	rm -f output.dat
	touch output.dat
	
	@echo ">>>>>>>>>>Running test on Image00"
	./serial images/image0.ppm > serial_tmp.dat
	./paral_cuda images/image0.ppm > cuda_tmp.dat
	./paral_clang_none images/image0.ppm > clang_none_tmp.dat
	./paral_clang_tile images/image0.ppm > clang_tile_tmp.dat
	./paral_clang_vect images/image0.ppm > clang_vect_tmp.dat
	./join Image00 > output_tmp.dat
	cat output.dat output_tmp.dat > tmp.dat
	cat tmp.dat > output.dat
	
	@echo ">>>>>>>>>>Running test on Image01"
	./serial images/image1.ppm > serial_tmp.dat
	./paral_cuda images/image1.ppm > cuda_tmp.dat
	./paral_clang_none images/image1.ppm > clang_none_tmp.dat
	./paral_clang_tile images/image1.ppm > clang_tile_tmp.dat
	./paral_clang_vect images/image1.ppm > clang_vect_tmp.dat
	./join Image01 > output_tmp.dat
	cat output.dat output_tmp.dat > tmp.dat
	cat tmp.dat > output.dat
	
	@echo ">>>>>>>>>>Running test on Image02"
	./serial images/image2.ppm > serial_tmp.dat
	./paral_cuda images/image2.ppm > cuda_tmp.dat
	./paral_clang_none images/image2.ppm > clang_none_tmp.dat
	./paral_clang_tile images/image2.ppm > clang_tile_tmp.dat
	./paral_clang_vect images/image2.ppm > clang_vect_tmp.dat
	./join Image02 > output_tmp.dat
	cat output.dat output_tmp.dat > tmp.dat
	cat tmp.dat > output.dat
	
	@echo ">>>>>>>>>>Running test on Image03"
	./serial images/image3.ppm > serial_tmp.dat
	./paral_cuda images/image3.ppm > cuda_tmp.dat
	./paral_clang_none images/image3.ppm > clang_none_tmp.dat
	./paral_clang_tile images/image3.ppm > clang_tile_tmp.dat
	./paral_clang_vect images/image3.ppm > clang_vect_tmp.dat
	./join Image03 > output_tmp.dat
	cat output.dat output_tmp.dat > tmp.dat
	cat tmp.dat > output.dat

	@echo ">>>>>>>>>>Running test on Image04"
	./serial images/image4.ppm > serial_tmp.dat
	./paral_cuda images/image4.ppm > cuda_tmp.dat
	./paral_clang_none images/image4.ppm > clang_none_tmp.dat
	./paral_clang_tile images/image4.ppm > clang_tile_tmp.dat
	./paral_clang_vect images/image4.ppm > clang_vect_tmp.dat
	./join Image04 > output_tmp.dat
	cat output.dat output_tmp.dat > tmp.dat
	cat tmp.dat > output.dat
		
	@echo ">>>>>>>>>>Cleaning temporary files"
	rm -f serial_tmp.dat cuda_tmp.dat clang_none_tmp.dat clang_tile_tmp.dat clang_vect_tmp.dat output_tmp.dat tmp.dat
	@echo ">>>>>>>>>>Results are in output.dat"

