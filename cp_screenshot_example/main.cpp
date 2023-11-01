#include "cp_screenshot/cp_screenshot.h"
#include <cstdio>
#include <ctime>

int main() {

  clock_t start = clock();

  unsigned char *screenshot_bytes;
  unsigned int width, height, length;
  cps::GetScreenshotImageByteData(&screenshot_bytes, &width, &height, &length);

  printf("get screenshot image data take about %f seconds.", ((double) (clock() - start)) / CLOCKS_PER_SEC);

  // write image data to file.
  FILE *file = fopen("screenshot_example.jpeg", "w+");
  fwrite(screenshot_bytes, length, 1, file);
  fclose(file);

}