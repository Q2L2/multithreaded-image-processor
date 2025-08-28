# Multithreaded Image Processor

A work-in-progress multithreaded image processing project for the PPM format images implemented in C, designed to demonstrate data structures, algorithms, and learned in coursework and multithreading learned online.

## Features
- Processes images using multiple threads determined by the number of CPU cores in the computer.
- Provides timing of processing images with a single thread vs. multiple threads to demonstrate improvement of on larger files.

## Known Issues
- Very large images may fail due to memory limitations.

- Some functions are experimental and may not cover all edge cases.

## Future Improvements

- Optimize multithreading thresholds for different image sizes.

- Add proper memory leak handling and testing.

- Extend support for additional image formats and operations.

## Acknowledgements
Developed as part of undergraduate coursework in C programming, data structures, and algorithms.
