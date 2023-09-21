#ifndef CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_
#define CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_

namespace cps {
/**
 * Take a screenshot of the screen where the mouse is located
 * @param screenshot the pointer to screenshot bitmap data
 * @param width bitmap width.
 * @param height bitmap height.
 * @return
 */
bool GetScreenshotBitmap(unsigned char **screenshot, size_t *width, size_t *height);
}

#endif //CROSS_PLATFORM_SCREENSHOT_SRC_SCREENSHOT_H_
