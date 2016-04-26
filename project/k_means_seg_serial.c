#include <cv.h>
// #include <imgproc.h>
#include <highgui.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
  /// Read Image
  IplImage* image = cvLoadImage("Lenna.png", 0);
  int rows = image.height;
  int cols = image.width;

  /// Reshape Image


  cvNamedWindow("Lala", 1);
  cvShowImage("Lala", image);
  cvWaitKey(0);

  /// Segment

  return 1;
}