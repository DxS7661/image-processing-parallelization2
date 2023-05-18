#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// CUDA kernel to apply Gaussian blur filter
__global__ void applyGaussianBlur(unsigned char* image, unsigned char* blurredImage, int width, int height, int kernelSize) {
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

    // Calculate the pixel indices
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    // Apply Gaussian blur to each pixel
    if (x < width && y < height) {
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

        // Store the blurred pixel in the output image
        blurredImage[y * width + x] = (unsigned char)blurredPixel;
    }

    // Free memory
    free(kernel);
}

int main() {
    int width = 4000;
    int height = 4000;
    int kernelSize = 25;

    // Allocate memory for the input and output images
    unsigned char* image = (unsigned char*)malloc(width * height * sizeof(unsigned char));
    unsigned char* blurredImage = (unsigned char*)malloc(width * height * sizeof(unsigned char));

    // Assume you have already loaded the image into the 'image' array
    // with pixel values ranging from 0 to 255.

    // Allocate device memory for the input and output images
    unsigned char* d_image;
    unsigned char* d
