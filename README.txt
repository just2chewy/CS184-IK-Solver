Names: Kevin Tseng, cs184-cp
Justin Chu, cs184-ac
Platform: Windows
Location of Source Code: cs184-ac/as6 (cs184-ac submitted the source code. cs184-cp submitted the README only)

Compile + Running Instructions:
- The files should be compiled using "make -f makefile.cygwin", which will produce an executable called glut_example. You can run the program by issuing the command "./bezier.exe teapot.bez 0.01 -a", using the appropriate parameters.
- Uniform subdivision is performed using the command "./bezier.exe filename subdivision -u". For example, "./bezier.exe teapot.bez 0.1 -u" will run the program on the teapot.bez input file, with a step size of 0.1. The "-u" parameter signifies that the program should apply uniform subdivision.
- Adaptive subdivision is performed using the command "./bezier.exe filename subdivision -a". For example, "./bezier.exe teapot.bez 0.1 -a" will run the program on the teapot.bez input file, with an allowed error of 0.1. The "-a" parameter signifies that the program should apply adaptive subdivision.
