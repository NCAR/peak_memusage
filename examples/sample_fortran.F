      program example

!     Read in some numbers and take the average
!     As written, if there are no data points, an average of zero is returned
!     While this may not be desired behavior, it keeps this example simple

      implicit none
      integer :: number_of_points=1000, nsteps=4, step=1
      real, dimension(:), allocatable :: points


!      write (*,*) "Input number of points to average:"
!      read (*,*) number_of_points

      call log_memusage_annotate("Annotating from Fortran...")
      allocate (points(number_of_points))
      points = 1.

!     call testfoo("test")
      call log_memusage_report()

      deallocate (points)

      call log_memusage_pause()
      call sleep(3)
      call log_memusage_resume()

      do step=1,nsteps
         number_of_points = 10*number_of_points
         call log_memusage_annotate("Annotating from Fortran...")
         allocate (points(number_of_points))
         points = step
         call log_memusage_report()

         call sleep(3)
         deallocate (points)
      end do

      end program example
