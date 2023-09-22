#ifndef CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_
#define CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_

namespace cps {
/**
 * Take a screenshot of the screen where the mouse is located
 * @param image_bytes the pointer to screenshot bitmap data
 * @param width bitmap width.
 * @param height bitmap height.
 * @param bytes_len bitmap bytes length.
 * @param compress where compress the screenshot image.
 * @return
 */
bool GetScreenshotImageByteData(unsigned char **image_bytes,
                                size_t *width,
                                size_t *height,
                                size_t *bytes_len,
                                bool compress);
}

#endif //CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_
