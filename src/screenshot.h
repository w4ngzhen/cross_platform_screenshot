#ifndef CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_
#define CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_

namespace cps {
/**
 * Take a screenshot of the screen where the mouse is located
 * @param image_bytes the pointer to screenshot image data
 * @param width image width.
 * @param height image height.
 * @param bytes_len image bytes length.
 * @return
 */
bool GetScreenshotImageByteData(unsigned char **image_bytes,
                                unsigned int *width,
                                unsigned int *height,
                                unsigned int *bytes_len);
}

#endif //CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_
