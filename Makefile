#
# $Id$
#
SOFTW=peak_memusage
TARGET=use_memory
CCMPI=mpicc -D COMPILE_MPI -U COMPILE_OMP 
CCOMP=gcc -U COMPILE_MPI -D COMPILE_OMP -fopenmp
CCCMD=gcc -U COMPILE_MPI -U COMPILE_OMP


build-all: build-serial build-omp build-mpi
build-serial: $(SOFTW).exe $(TARGET).exe
build-omp: $(SOFTW)_omp.exe $(TARGET)_omp.exe
build-mpi: $(SOFTW)_mpi.exe $(TARGET)_mpi.exe

clean:
	-rm -f *.exe

# BUILD

$(SOFTW).exe: $(SOFTW).c
	$(CCCMD) $? -o $@
$(SOFTW)_omp.exe: $(SOFTW).c
	$(CCOMP) $? -o $@
$(SOFTW)_mpi.exe: $(SOFTW).c
	$(CCMPI) $? -o $@
	
$(TARGET).exe: $(TARGET).c
	$(CCCMD) -g $? -o $@
$(TARGET)_omp.exe: $(TARGET).c
	$(CCOMP) -g $? -o $@
$(TARGET)_mpi.exe: $(TARGET).c
	$(CCMPI) -g $? -o $@

# RUN

run-serial: build-serial
	./$(SOFTW).exe ./$(TARGET).exe
run-omp: build-omp
	export OMP_NUM_THREADS=2; ./$(SOFTW)_omp.exe ./$(TARGET)_omp.exe
run-mpi: build-mpi
	# This is normal MPI approach and should work
	@if [ -z "$${MPIRUN}" ]; then \
            echo You must set \$$MPIRUN to something like mpirun -n 4; \
            exit 1; \
        else \
            echo $${MPIRUN} ./peak_memusage_mpi.exe ./use_memory_mpi.exe -s 10000000; \
            $${MPIRUN} ./peak_memusage_mpi.exe ./use_memory_mpi.exe -s 10000000; \
        fi
run-mpi-hack: build-serial
	# This is the MP_CHILD hack and may or may not work
	@if [ -z "$${MPIRUN}" ]; then \
            echo You must set \$$MPIRUN to something like mpirun -n 4; \
            exit 1; \
        else \
            echo $${MPIRUN} ./peak_memusage.exe ./use_memory_mpi.exe -s 10000000; \
            $${MPIRUN} ./peak_memusage.exe ./use_memory_mpi.exe -s 10000000; \
        fi
	
repeat:
	while [ 1 ]; do make -s run-serial;  sleep 3; done 
