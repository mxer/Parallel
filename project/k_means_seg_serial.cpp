#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
  /// Get K
  int k = atoi(argv[1]);

  /// Read Image
  cv::Mat image = cv::imread("image_1.png", 0);

  /// Reshape Image
  int number_of_pixels = image.rows * image.cols;
  cv::Mat pixel_values(number_of_pixels, 1, CV_8U);
  image.reshape(1, number_of_pixels).copyTo(pixel_values.col(0));
  pixel_values.convertTo(pixel_values, CV_32F);

  /// Do k-means
  cv::Mat bestLabels;
  cv::kmeans(pixel_values, k, bestLabels, cv::TermCriteria(), 10,
   cv::KMEANS_RANDOM_CENTERS);

  cv::Mat blobbed_image;
  bestLabels.copyTo(blobbed_image);
  // std::cout << bestLabels << "\n";

  /// bestLabels, contains the number of the cluster to which each pixel belongs
  cv::Mat segmented_image;
  cv::convertScaleAbs(bestLabels, segmented_image, int(255 / k));

  segmented_image = segmented_image.reshape(0, image.rows);
  blobbed_image = blobbed_image.reshape(0, image.rows);
  /// Do connected components
  int label_count = k;
  for (int i = 0; i < blobbed_image.rows; i++)
  {
    for (int j = 0; j < blobbed_image.cols; j++)
    {
      if ((int)blobbed_image.at<int>(i, j) == 1)
      {
        cv::floodFill(blobbed_image, cv::Point(j, i),
                      cv::Scalar(label_count));
        label_count++;
      }
    }
  }
  cv::Mat blob;
  cv::normalize(blobbed_image, blob, 0, 255, cv::NORM_MINMAX);
  blob.convertTo(blob, CV_8U);
  cv::namedWindow("Original", 1);
  cv::imshow("Original", image);

  cv::namedWindow("Segmented", 1);
  cv::imshow("Segmented", segmented_image);

  cv::namedWindow("Blobbed", 1);
  cv::imshow("Blobbed", blob);

  cv::waitKey(0);

  return 1;
}