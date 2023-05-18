#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

// Function to apply Gaussian blur filter using MPI
void applyGaussianBlur(unsigned char* image, int width, int height, int kernelSize) {
    double sigma = 5.0; // Standard deviation for Gaussian blur
    double twoSigmaSquare = 2.0 * sigma * sigma;

    // Calculate kernel values
    double* kernel = (double*)malloc(kernelSize * kernelSize * sizeof(double));
    double sum = 0.0;
    int radius = kernelSize / 2;

    // Calculate the values for the Gaussian kernel
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            double distance = i * i + j * j;
            int index = (i + radius) * kernelSize + (j + radius);
            kernel[index] = exp(-distance / twoSigmaSquare) / (M_PI * twoSigmaSquare);
            sum += kernel[index];
        }
    }

    // Normalize the kernel by dividing each value by the sum
    for (int i = 0; i < kernelSize * kernelSize; i++) {
        kernel[i] /= sum;
    }

    // Create a temporary image for storing blurred pixels
    unsigned char* tempImage = (unsigned char*)malloc(width * height * sizeof(unsigned char));

    // Get MPI rank and size
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Calculate the number of rows each process will handle
    int rowsPerProcess = height / size;
    int extraRows = height % size;

    // Calculate the start and end row indices for each process
    int startRow = rank * rowsPerProcess;
    int endRow = startRow + rowsPerProcess - 1;

    // Adjust the end row for the last process to account for the extra rows
    if (rank == size - 1) {
        endRow += extraRows;
    }

    // Apply Gaussian blur to the assigned rows using MPI communication
    for (int y = startRow; y <= endRow; y++) {
        for (int x = 0; x < width; x++) {
            double blurredPixel = 0.0;

            // Convolve the kernel with the surrounding pixels for blurring
            for (int i = -radius; i <= radius; i++) {
                for (int j = -radius; j <= radius; j++) {
                    int imageX = x + j;
                    int imageY = y + i;

                    // Handle boundary cases by clamping pixel coordinates
                    if (imageX < 0)
                        imageX = 0;
                    else if (imageX >= width)
                        imageX = width - 1;

                    if (imageY < 0)
                        imageY = 0;
                    else if (imageY >= height)
                        imageY = height - 1;

                    // Apply the kernel to the pixel
                    int index = (i + radius) * kernelSize + (j + radius);
                    blurredPixel += image[imageY * width + imageX] * kernel[index];
                }
            }

            // Store the blurred pixel in the temporary image
