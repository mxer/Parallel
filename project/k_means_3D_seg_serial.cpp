#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

#define SLICES 6
#define K 2

std::vector<cv::Mat> read_images()
{
  std::vector<cv::Mat> images;

  std::string file_name = "/home/thomai/Dropbox/KTH/4th Semester/Parallel Computations for Large - Scale Problems/Parallel/project/dots_4";
  std::string file_end = ".png";
  std::ostringstream file;
  file << file_name << file_end;
  for (int i = 1; i <= SLICES; i++)
  {
    images.push_back(cv::imread(file.str().c_str(), 0));
  }
  return images;
}

void three_d_connected_components(std::vector<cv::Mat> images,
                                  std::vector<cv::Rect> &blobs,
                                  std::vector<int> &image_count,
                                  std::vector<int> &blob_count)
{
  int label_count = K;
  /// First do connected components to each 2D image
  for (size_t img = 0; img < images.size(); img++)
  {
    cv::Mat temp;
    cv::Rect temp_rect[1];
    images[img].copyTo(temp);
    for (int i = 0; i < temp.rows; i++)
    {
      for (int j = 0; j < temp.cols; j++)
      {
        if ((int)temp.at<int>(i, j) == 0)
        {
          image_count.push_back(static_cast<int>(img));
          cv::floodFill(temp, cv::Point(j, i), cv::Scalar(label_count),
                        temp_rect);
          blobs.push_back(temp_rect[0]);
          blob_count.push_back(label_count);
          label_count++;
        }
      }
    }
  }

  for (size_t i = 0; i < image_count.size(); i++)
  {
    int first = image_count[i];
    for (size_t j = i; j < image_count.size(); j++)
    {
      int second = image_count[j];
      if (second == first + 1)
      {
        /// Only checking consecutive images
        cv::Rect intersection = blobs[i] & blobs[j];
        if (intersection.height != 0 && intersection.width != 0)
        {
          blob_count[j] = blob_count[i];
        }
      }
    }
  }
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
  }
  std::vector<cv::Rect> blobs;
  std::vector<int> blob_count;
  std::vector<int> image_count;
  three_d_connected_components(clustered_images, blobs, image_count,
                               blob_count);

  for (size_t i = 0; i < blob_count.size(); i++)
  {
    blob_count[i] -= K;
  }

  /// Show images
  for (int i = 0; i < SLICES; i++)
  {
    /// Draw rectangles on image
    for (size_t r = 0; r < image_count.size(); r++)
    {
      if (image_count[r] == i)
      {
        rectangle(images[i], blobs[r], cv::Scalar(0, 0, 0), 1);
        std::ostringstream txt;
        txt << blob_count[r];
        cv::Point origin(blobs[r].x + blobs[r].width / 2,
                         blobs[r].y + blobs[r].height / 2);
        putText(images[i], txt.str().c_str(), origin,
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 1);
      }
    }
    std::ostringstream name1;
    name1 << "Original " << i + 1;
    cv::namedWindow(name1.str().c_str(), 1);
    cv::imshow(name1.str().c_str(), images[i]);
  }
  cv::waitKey(0);
  return 0;
}