#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

#define SLICES 3
#define K 2

std::vector<cv::Mat> read_images()
{
  std::vector<cv::Mat> images;
  // std::string file_name = "/home/thomai/Dropbox/KTH/4th Semester/Parallel Computations for Large - Scale Problems/project/disc1/OAS1_0003_MR1/RAW/OAS1_0003_MR1_mpr-";
  // std::string file_end = "_anon_sag_66.hdr";

  std::string file_name = "/home/thomai/Dropbox/KTH/4th Semester/Parallel Computations for Large - Scale Problems/project/dots_2";
  std::string file_end = ".png";

  for (int i = 1; i <= SLICES; i++)
  {
    std::ostringstream file;
    file << file_name << file_end;

    images.push_back(cv::imread(file.str().c_str(), 0));
  }
  return images;
}

std::vector<cv::Mat> three_d_connected_components(std::vector<cv::Mat> images)
{
  int label_count = K;
  /// First do connected components to each 2D image
  std::vector<cv::Mat> blobbed_image;
  for (size_t img = 0; img < images.size(); img++)
  {
    cv::Mat temp;
    images[img].copyTo(temp);
    blobbed_image.push_back(temp);
    for (int i = 0; i < blobbed_image[img].rows; i++)
    {
      for (int j = 0; j < blobbed_image[img].cols; j++)
      {
        if ((int)blobbed_image[img].at<int>(i, j) == 0)
        {
          cv::floodFill(blobbed_image[img], cv::Point(j, i),
                        cv::Scalar(label_count));
          label_count++;
        }
      }
    }
    blobbed_image[img].convertTo(blobbed_image[img], CV_8U);
  }

  for (size_t img = 1; img < images.size(); img++)
  {
    for (int i = 0; i < blobbed_image[img].rows; i++)
    {
      for (int j = 0; j < blobbed_image[img].cols; j++)
      {
        if ((int)blobbed_image[img - 1].at<uint8_t>(i, j) != 1)
        {
          blobbed_image[img].at<uint8_t>(i, j) =
           blobbed_image[img - 1].at<uint8_t>(i, j);
        }
      }
    }
  }
  return blobbed_image;
}


int main(int argc, char *argv[])
{
  /// Read Images
  std::vector<cv::Mat> images = read_images();

  for (size_t i = 1; i < images.size(); i++)
  {
    if (images[0].rows != images[i].rows || images[0].cols != images[i].cols)
    {
      std::cout << "ERROR: Images need to be of the same size\n";
      return -1;
    }
  }

  /// Reshape Image
  int number_of_pixels = 0;
  for (int i = 0; i < SLICES; i++)
  {
    number_of_pixels += images[i].rows * images[i].cols;
  }
  // cv::Mat pixel_values(number_of_pixels, 1, CV_8U);
  cv::Mat pixel_values;
  for (int i = 0; i < SLICES; i++)
  {
    cv::Mat temp;
    images[i].reshape(1, images[i].rows * images[i].cols).copyTo(temp);
    pixel_values.push_back(temp);
  }

  pixel_values.convertTo(pixel_values, CV_32F);

  /// Do k-means
  cv::Mat bestLabels;
  cv::kmeans(pixel_values, K, bestLabels, cv::TermCriteria(), 10,
   cv::KMEANS_RANDOM_CENTERS);

  std::vector<cv::Mat> clustered_images;

  /// bestLabels, contains the number of the cluster to which each pixel belongs
  int so_far = 0;
  for (int i = 0; i < SLICES; i++)
  {
    cv::Mat temp;
    bestLabels.rowRange(so_far, so_far + images[i].rows * images[i].cols)
     .copyTo(temp);
    so_far += images[i].rows * images[i].cols;

    // cv::Mat new_temp;
    temp = temp.reshape(1, images[i].rows);
    clustered_images.push_back(temp);

    std::ostringstream name1, name2;
    name1 << "Original " << i + 1;
    name2 << "Segmented " << i + 1;
    cv::namedWindow(name1.str().c_str(), 1);
    cv::imshow(name1.str().c_str(), images[i]);

    cv::namedWindow(name2.str().c_str(), 1);
    cv::imshow(name2.str().c_str(), temp);
  }

  std::vector<cv::Mat> final = three_d_connected_components(clustered_images);

  for (int i = 0; i < SLICES; i++)
  {
    std::ostringstream name;
    name << "Blobbed " << i + 1;
    cv::namedWindow(name.str().c_str(), 1);
    cv::imshow(name.str().c_str(), final[i]);
  }
  cv::waitKey(0);
  return 0;
}