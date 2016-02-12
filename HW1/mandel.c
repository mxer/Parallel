#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#define W 4096
#define H 4096
#define B 2
#define N 256

unsigned char cal_pixel(double r, double i) {
	unsigned char count = 0;
	double iz = 0, rz = 0, temp = 0;
	while ((sqrt(pow(rz, 2) + pow(iz, 2)) < B) && (count < (N - 1))) // N-1 to avoid overflow with unsigned char
	{
		temp = rz;
		rz = pow(rz, 2) - pow(iz, 2) + r;
		iz = (2 * iz * temp) + i;
		count++;
	}

	return count;
}

int main(int argc, char** argv) {
	int i, j, x, y;
	FILE *fp;
	int p, P, rc;
	MPI_Status status;

	rc = MPI_Init(&argc, &argv);
	rc = MPI_Comm_size(MPI_COMM_WORLD, &P);
	rc = MPI_Comm_rank(MPI_COMM_WORLD, &p);
	// printf("%d %d \n", P, p);
	// fflush(stdout);
	if (W % P == 0) //W divisible by P
	{
		unsigned char *color = malloc(W * H * sizeof(unsigned char));
		unsigned char *a = malloc(W * H * sizeof(unsigned char));

		double dx, dy, dreal, dimg;

		dx = (double)((2 * B) / (double)(8192 - 1)); // used to be W - 1, change back for full image
		dy = (double)((2 * B) / (double)(8192 - 1)); // used to be H - 1, change back for full image
		int wp = W / P;
		double xoff = (double)(p * W / (double)P);
		for (x = xoff; x < (p + 1)*wp; x++)
		{
			dreal = x * dx - B + 1.5; // remove the 1.5 to stop panning
			for (y = 0; y < H; y++) {
				dimg = y * dy - B + 1.5; // remove the 1.5 to stop panning
				color[x + y * W] = cal_pixel(dreal, dimg);
			}
		}


		if (p == 0) {
			for (i = 1; i < P; i++) {
				rc = MPI_Recv(a, W * H, MPI_UNSIGNED_CHAR, i, 100, MPI_COMM_WORLD, &status);

				for (x = i * wp; x < (i + 1) * wp; x++) {
					for (y = 0; y < H; y++)
						color[x + y * W] = a[x + y * W];
				}
			}


			fp = fopen("color.txt", "w");
			for (j = 0; j < H; j++) {
				for (i = 0; i < W; i++)
					fprintf(fp, "%hhu ", color[i + j * W]);
				fprintf(fp, "\n");
			}
			fclose(fp);
		} else {
			rc = MPI_Send(color, W * H, MPI_UNSIGNED_CHAR, 0, 100, MPI_COMM_WORLD);
		}

		MPI_Finalize();
		free(color);
		free(a);
		return 0;
	} else {
		if (p == 0)
			printf("W (%d) is not divisible by processor count (%d) \n", W, P);

		MPI_Finalize();
		return 1;
	}
}