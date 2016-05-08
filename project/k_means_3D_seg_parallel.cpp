#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

#include <mpi.h>

#define K 2
#define filename "blobs.txt"

std::vector<cv::Mat> read_images(int I, int dot_idx)
{
  std::vector<cv::Mat> images;

  std::string file_name = "/cfs/klemming/nobackup/t/thomai/project/dots_";
  std::string file_end = ".png";
  std::ostringstream file;
  file << file_name << dot_idx << file_end;
  // Current processor only works with I images.
  // If we were actually using MRI slices, we would also need the indices at
  // this point. But now we only read I times the same dot image.
  for (int i = 0; i < I; i++)
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
  /// MPI-related variables
  int P, p, N, *Ip, I, dot_idx;
  double start_time, end_time;
  FILE *fp;

  /// Initialize MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &p);

  /// Problem size in this algorithm is No. of SLICES
  if (argc < 3)
  {
    std::cout <<
     "NOT ENOUGH INPUT PARAMETERS:\n./excutable image_num num_of_slices\n";
    exit(1);
  }
  dot_idx = atoi(argv[1]);
  N = atoi(argv[2]);

  start_time = MPI_Wtime();

  /// LOCAL SIZE ===============================================================
  Ip = new int[P]; // Number of elements of every processor
  for (int i = 0; i < P; i++)
  {
    Ip[i] = (N + P - i - 1) / P;
  }
  I = Ip[p]; // Number of elements for current processor

  /// READ IMAGES ==============================================================
  std::vector<cv::Mat> images = read_images(I, dot_idx);

  for (size_t i = 1; i < images.size(); i++)
  {
    if (images[0].rows != images[i].rows || images[0].cols != images[i].cols)
    {
      std::cout << "ERROR: Images need to be of the same size\n";
      return -1;
    }
  }

  if (p == 0)
  {
    printf ("%d, %d, %d\n", P, N, images[0].rows);
  }

  /// RESHAPE IMAGES ===========================================================
  int number_of_pixels = 0;
  for (int i = 0; i < I; i++)
  {
    number_of_pixels += images[i].rows * images[i].cols;
  }
  cv::Mat pixel_values;
  for (int i = 0; i < I; i++)
  {
    cv::Mat temp;
    images[i].reshape(1, images[i].rows * images[i].cols).copyTo(temp);
    pixel_values.push_back(temp);
  }
  pixel_values.convertTo(pixel_values, CV_32F);

  /// DO K-MEANS ===============================================================
  cv::Mat bestLabels;
  cv::kmeans(pixel_values, K, bestLabels, cv::TermCriteria(), 10,
   cv::KMEANS_RANDOM_CENTERS);

  std::vector<cv::Mat> clustered_images;

  /// bestLabels, contains the number of the cluster to which each pixel belongs
  int so_far = 0;
  for (int i = 0; i < I; i++)
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

  /// SHOW IMAGES (only for debugging purposes) ================================
  // for (int i = 0; i < SLICES; i++)
  // {
  //   /// Draw rectangles on image
  //   for (size_t r = 0; r < image_count.size(); r++)
  //   {
  //     if (image_count[r] == i)
  //     {
  //       rectangle(images[i], blobs[r], cv::Scalar(0, 0, 0), 1);
  //       std::ostringstream txt;
  //       txt << blob_count[r];
  //       cv::Point origin(blobs[r].x + blobs[r].width / 2,
  //                        blobs[r].y + blobs[r].height / 2);
  //       putText(images[i], txt.str().c_str(), origin,
  //               cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 1);
  //     }
  //   }
  //   std::ostringstream name1;
  //   name1 << "Original " << i + 1;
  //   cv::namedWindow(name1.str().c_str(), 1);
  //   cv::imshow(name1.str().c_str(), images[i]);
  // }
  // cv::waitKey(0);
  /// ==========================================================================

  /// STITCH IT TOGETHER =======================================================
  if (p == 0)
  {
    /// First write everything to files
    /// Create file
    fp = fopen(filename, "w");

    int last_image_count = 0, blob_counter = 0;
    int *last_temp, *last;
    last_temp = new int[blobs.size() * 4 + blob_count.size()];

    for (size_t i = 0; i < blobs.size(); i++)
    {
      if (blob_count[i] > blob_counter)
      {
        blob_counter = blob_count[i];
      }
      fprintf(fp, "%d %d %d %d %d %d\n", image_count[i], blob_count[i],
              blobs[i].x, blobs[i].y, blobs[i].height, blobs[i].width);
      /// Create array for last image
      if (image_count[i] == I - 1)
      {
        last_temp[last_image_count * 5] = blob_count[i];
        last_temp[last_image_count * 5 + 1] = blobs[i].x;
        last_temp[last_image_count * 5 + 2] = blobs[i].y;
        last_temp[last_image_count * 5 + 3] = blobs[i].height;
        last_temp[last_image_count * 5 + 4] = blobs[i].width;
        last_image_count++;
      }
    }
    if (last_image_count == 0)
    {
      last = new int[1];
      last[0] = -1;
    }
    else
    {
      last = new int[last_image_count * 5];
      memcpy(last_temp, last, last_image_count * 5 * sizeof(int));
    }

    // Close file
    fclose(fp);

    // Send last slide to next process
    if (P > 1)
    {
      MPI_Send(&blob_counter, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
      MPI_Send(&last_image_count, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
      MPI_Send(&last, last_image_count * 5, MPI_INT, 1, 0, MPI_COMM_WORLD);
    }
  }
  else
  {
    // Wait for signal from previous process
    int num_blobs = 0, *previous, previous_blob_counter = 0;
    MPI_Recv(&previous_blob_counter, 1, MPI_INT, p - 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Recv(&num_blobs, 1, MPI_INT, p - 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    previous = new int[num_blobs * 5];
    MPI_Recv(previous, num_blobs * 5, MPI_INT, p - 1, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    std::vector<int> old_blob_count;
    for (size_t blob = 0; blob < blob_count.size(); blob++)
    {
      blob_count[blob] += 100;
      old_blob_count.push_back(blob_count[blob]);
    }

    if (num_blobs != 0)
    {
      std::vector<cv::Rect> prev_blobs;
      std::vector<int> prev_blob_count;
      for (int i = 0; i < num_blobs; i++)
      {
        cv::Rect temp;
        prev_blob_count.push_back(previous[i * 5]);
        temp.x = previous[i * 5 + 1];
        temp.y = previous[i * 5 + 2];
        temp.height = previous[i * 5 + 3];
        temp.width = previous[i * 5 + 4];
        prev_blobs.push_back(temp);
      }

      for (int blob = 0; blob < num_blobs; blob++)
      {
        for (size_t i = 0; i < image_count.size(); i++)
        {
          if (image_count[i] == 0)
          {
            /// The first slice needs to be compared with the previous one
            cv::Rect intersection = blobs[i] & prev_blobs[blob];
            if (intersection.height != 0 && intersection.width != 0)
            {
              blob_count[i] = prev_blob_count[blob];
            }
          }
        }
      }
      /// At this point the first slice has the same labels as the previous one
      /// We need to replace the blob labels
      for (size_t i = 0; i < blob_count.size(); i++)
      {
        if (image_count[i] == 0)
        {
          int new_label = blob_count[i];
          int old_label = old_blob_count[i];
          for (size_t j = 0; j < blob_count.size(); j++)
          {
            if (blob_count[j] == old_label)
            {
              blob_count[j] = new_label;
            }
          }
        }
      }
    }

    /// Change all labels to be the increment of the previous slide
    for (size_t  i = 0; i < blob_count.size(); i++)
    {
      if (blob_count[i] > previous_blob_counter)
      {
        int old_label = blob_count[i];
        for (size_t j = i; j < blob_count.size(); j++)
        {
          if (blob_count[j] == old_label)
          {
            previous_blob_counter++;
            blob_count[j] = previous_blob_counter;
          }
        }
      }
    }

    /// Open file to append
    fp = fopen(filename, "a");
    int last_image_count = 0;
    int *last_temp, *last;
    last_temp = new int[blobs.size() * 4 + blob_count.size()];

    for (size_t i = 0; i < blobs.size(); i++)
    {
      /// Create array for last image
      if (image_count[i] == I - 1)
      {
        last_temp[last_image_count * 5] = blob_count[i];
        last_temp[last_image_count * 5 + 1] = blobs[i].x;
        last_temp[last_image_count * 5 + 2] = blobs[i].y;
        last_temp[last_image_count * 5 + 3] = blobs[i].height;
        last_temp[last_image_count * 5 + 4] = blobs[i].width;
        last_image_count++;
      }
      /// Fix image counter
      int im_count;
      im_count = p * ((int) (N / P)) + std::min(P, (N % P) + image_count[i]);
      fprintf(fp, "%d %d %d %d %d %d\n", im_count, blob_count[i],
              blobs[i].x, blobs[i].y, blobs[i].height, blobs[i].width);
    }
    if (last_image_count == 0)
    {
      last = new int[1];
      last[0] = -1;
    }
    else
    {
      last = new int[last_image_count * 5];
      memcpy(last_temp, last, last_image_count * 5 * sizeof(int));
    }

    // Close file
    fclose(fp);

    // Send signal to next process
    if (p != P - 1)
    {
      MPI_Send(&previous_blob_counter, 1, MPI_INT, p + 1, 0, MPI_COMM_WORLD);
      MPI_Send(&last_image_count, 1, MPI_INT, p + 1, 0, MPI_COMM_WORLD);
      MPI_Send(&last, last_image_count * 5, MPI_INT, p + 1, 0, MPI_COMM_WORLD);
    }
  }

  end_time = MPI_Wtime();
  if (p == 0) {
    printf ("%e", end_time - start_time);
  }
  /* That's it */
  MPI_Finalize();
  exit(0);
  return 0;
}