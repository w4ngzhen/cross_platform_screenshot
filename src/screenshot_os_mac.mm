#import <Foundation/Foundation.h>
#import <CoreGraphics/CGWindow.h>
#import <CoreImage/CoreImage.h>
#import <AppKit/NSBitmapImageRep.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSScreen.h>
#include "screenshot.h"

CGImageRef GetMouseScreenImage() {
  // get the mouse location.
  auto mouse_location = [NSEvent mouseLocation];
  id screenId;
  // find the screen where mouse located.
  for (id screen in [NSScreen screens]) {
    if (NSMouseInRect(mouse_location, [screen frame], NO)) {
      screenId = screen;
      break;
    }
  }
  if (!screenId) {
    return nullptr;
  }
  auto screen_rect = [screenId frame];
  CGImageRef img = CGWindowListCreateImage(
      screen_rect,
      kCGWindowListOptionAll,
      kCGNullWindowID,
      kCGWindowImageBoundsIgnoreFraming);
  return img;
}

bool GetCGImageByteData(CGImageRef cg_image, size_t *width, size_t *height, uint8_t **bitmap) {
  if (cg_image == nullptr) {
    return false;
  }
  *width = CGImageGetWidth(cg_image);
  *height = CGImageGetHeight(cg_image);
  NSBitmapImageRep *bitmap_rep = [[NSBitmapImageRep alloc] initWithCGImage:cg_image];
  NSImage *ns_image = [[NSImage alloc] init];
  [ns_image addRepresentation:bitmap_rep];
  NSData *image_data = [ns_image TIFFRepresentation];
  *bitmap = (uint8_t *) image_data.bytes;
  return true;
}

namespace cps {
bool GetScreenshotBitmap(unsigned char **screenshot, size_t *width, size_t *height) {
  CGImageRef p_cg_image = GetMouseScreenImage();
  return GetCGImageByteData(p_cg_image, width, height, screenshot);
}
}