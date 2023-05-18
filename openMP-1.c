#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// Function to apply Gaussian blur filter using OpenMP
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

    // Apply Gaussian blur to each pixel in the image using OpenMP parallelism
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
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

                    // Update the blurred pixel value using atomic addition
                    #pragma omp atomic
                    blurredPixel += image[imageY * width + imageX] * kernel[index];
                }
            }

            // Store the blurred pixel in the temporary image
            tempImage[y * width + x] = (unsigned char)blurredPixel;
        }
    }

    // Copy the blurred pixels from the temporary image to the original image
    for (int i = 0; i < width * height; i++) {
        image[i] = tempImage[i];
    }

    // Free memory
    free(tempImage);
    free(kernel);
}

int main() {
    int width = 4000;
    int height = 4000;
    int kernelSize = 25;

   
