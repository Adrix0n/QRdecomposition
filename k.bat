::nvcc %1.cu -o %1 -arch=sm_21
::nvcc %1.cu -o %1 -arch=sm_75
nvcc %1.cu -o %1 -arch=sm_50 -Xcompiler /openmp 
::nvcc %1.cu -o %1
 
@del %1.lib
@del %1.exp