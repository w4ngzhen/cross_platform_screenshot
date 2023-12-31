#import <Foundation/Foundation.h>
#import <CoreGraphics/CGWindow.h>
#import <CoreImage/CoreImage.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSScreen.h>
#include "cp_screenshot/cp_screenshot.h"

CGImageRef GetMouseScreenImage() {
  // get the mouse location.
  auto mouse_location = [NSEvent mouseLocation];

  // list all screens
  NSArray *screens = [NSScreen screens];
  NSScreen *foundScreen = nil;
  for (NSScreen *screen in screens) {
    if (NSPointInRect(mouse_location, screen.frame)) {
      foundScreen = screen;
      break;
    }
  }

  if (!foundScreen) {
    return nullptr;
  }

  // get the display ID from screen info.
  NSDictionary *desc = [foundScreen deviceDescription];
  NSNumber *screenId = [desc objectForKey:@"NSScreenNumber"];
  CGDirectDisplayID displayId = [screenId unsignedIntValue];

  // create image from the display.
  CGImageRef img = CGDisplayCreateImage(displayId);
  return img;
}

bool GetCGImageByteData(CGImageRef cg_image,
                        unsigned char **image_bytes,
                        unsigned int *width,
                        unsigned int *height,
                        unsigned int *bytes_len) {
  if (cg_image == nullptr) {
    return false;
  }
  // get width
  *width = CGImageGetWidth(cg_image);
  *height = CGImageGetHeight(cg_image);

  // get image data
  CFMutableDataRef data = CFDataCreateMutable(kCFAllocatorDefault, 0);
  // kUTTypeJPEG -> PNG format.
  CGImageDestinationRef data_dst = CGImageDestinationCreateWithData(data, kUTTypeJPEG, 1, nullptr);
  CGImageDestinationAddImage(data_dst, cg_image, nullptr);
  CGImageDestinationFinalize(data_dst);
  CFIndex data_len = CFDataGetLength(data);
  auto *temp_bytes = (unsigned char *) malloc(data_len);
  CFDataGetBytes(data, CFRangeMake(0, data_len), temp_bytes);
  *bytes_len = data_len;
  *image_bytes = temp_bytes;
  return true;
}

namespace cps {
bool GetScreenshotImageByteData(unsigned char **image_bytes,
                                unsigned int *width,
                                unsigned int *height,
                                unsigned int *bytes_len) {
  CGImageRef p_cg_image = GetMouseScreenImage();
  return GetCGImageByteData(p_cg_image, image_bytes, width, height, bytes_len);
}
}