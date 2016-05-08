2d:
	mpicxx -g -Wall -o k_means_seg_serial k_means_seg_serial.cpp `pkg-config opencv --cflags opencv` `pkg-config --libs opencv` -lm -lopencv_highgui -lopencv_imgproc

3d:
	mpicxx -g -Wall -Og -o k_means_3D_seg_serial k_means_3D_seg_serial.cpp `pkg-config opencv --cflags opencv` `pkg-config --libs opencv` -lm -lopencv_highgui -lopencv_imgproc

3d_par:
	mpicxx -g -Wall -O3 -o k_means_3D_seg_parallel k_means_3D_seg_parallel.cpp `pkg-config opencv --cflags opencv` `pkg-config --libs opencv` -lm -lopencv_highgui -lopencv_imgproc

k_means_test:
	mpicxx -g -Wall -Og -o k_means_test k_means_test.cpp `pkg-config opencv --cflags opencv` `pkg-config --libs opencv` -lm -lopencv_highgui -lopencv_imgproc
