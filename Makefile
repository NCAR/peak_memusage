#
# $Id$
#
SOFTW=job_memusage
TARGET=spare
#CCMPI=/usr/bin/mpcc_r -D COMPILE_MPI
#CCOMP=xlc_r -U COMPILE_MPI -D COMPILE_OMP -qsmp=omp
#CCCMD=xlc_r -U COMPILE_MPI
CCMPI=mpicc -D COMPILE_MPI -U COMPILE_OMP # eventually it should use mpigcc
CCOMP=gcc -U COMPILE_MPI -D COMPILE_OMP -fopenmp
CCCMD=gcc -U COMPILE_MPI -U COMPILE_OMP


all: clean build-cmd build-omp build-mpi build-target-omp build-target-cmd build-target-mpi

clean:
	-rm -f *.exe

# BUILD

build-cmd:
	$(CCCMD) $(SOFTW).c -o $(SOFTW)_cmd.exe
build-omp:
	$(CCOMP) $(SOFTW).c -o $(SOFTW)_omp.exe
build-mpi:
	$(CCMPI) $(SOFTW).c -o $(SOFTW)_mpi.exe
	
build-target-cmd:
	$(CCCMD) -g $(TARGET).c -o $(TARGET)_cmd.exe
build-target-omp:
	$(CCOMP) -g $(TARGET).c -o $(TARGET)_omp.exe
build-target-mpi:
	$(CCMPI) -g $(TARGET).c -o $(TARGET)_mpi.exe

# RUN

run-cmd: build-target-cmd build-cmd
	./$(SOFTW)_cmd.exe ./$(TARGET)_cmd.exe
run-omp: build-target-omp build-omp
	export OMP_NUM_THREADS=2; ./$(SOFTW)_omp.exe ./$(TARGET)_omp.exe
run-mpi:
	$(LSFPATH)bsub < $(SOFTW).lsf
	$(LSFPATH)bjobs
	
repeat:
	while [ 1 ]; do make -s run-cmd;  sleep 3; done 
