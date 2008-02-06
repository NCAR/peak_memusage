#
# $Id$
#
SOFTW=job_memusage
TARGET=spare
MYPATH=./
MYHOST=valerian.scd.ucar.edu
MYURL=/home/ddvento/subversion.ucar.edu_ddvento/src/c/job_memusage/
CCMPI=/usr/bin/mpcc_r -D COMPILE_MPI
CCOMP=xlc_r -U COMPILE_MPI -D COMPILE_OMP -qsmp=omp
CCCMD=xlc_r -U COMPILE_MPI

LSFPATH=/usr/local/lsf/7.0/aix5-64/bin/

all: clean build-cmd build-omp build-mpi build-target-omp build-target-cmd build-target-mpi

update:
	scp $(MYHOST):$(MYURL){*.c,Makefile,*.lsf} .

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

run-cmd:
	$(MYPATH)$(SOFTW)_cmd.exe $(TARGET)_cmd.exe
run-omp:
	$(MYPATH)$(SOFTW)_omp.exe $(TARGET)_omp.exe
run-mpi:
	$(LSFPATH)bsub < $(SOFTW).lsf
	$(LSFPATH)bjobs
	

