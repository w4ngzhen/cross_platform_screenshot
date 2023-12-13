#include "cp_screenshot/cp_screenshot.h"
#include <cstdio>
#include <ctime>
#include <fstream>

int main() {
  clock_t start = clock();

  unsigned char *screenshot_bytes;
  unsigned int width, height, length;
  if (!cps::GetScreenshotImageByteData(&screenshot_bytes, &width, &height, &length)) {
    printf("ERROR: get screenshot image data failed.");
    return -1;
  }

  printf("SUCCESS: get screenshot image data take about %f seconds.",
         static_cast<double>(clock() - start) / CLOCKS_PER_SEC);

  std::ofstream outfile("screenshot_example.jpeg", std::ofstream::binary);
  outfile.write((char *) screenshot_bytes, length);
  outfile.close();
}
