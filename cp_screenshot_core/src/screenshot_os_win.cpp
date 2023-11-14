#include <windows.h>
#include <cstdio>
#include <cstdint>
#include "cp_screenshot/cp_screenshot.h"

namespace cps {
/**
 * OS Windows
 * get the monitor info where mouse pointer located.
 * \related MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
 */
HMONITOR GetPointerLocatedMonitor() {
  POINT point;
  BOOL result = GetCursorPos(&point);
  if (result) {
    HMONITOR h_monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONULL);
    if (h_monitor != nullptr && h_monitor != INVALID_HANDLE_VALUE) {
      return h_monitor;
    }
  }
  return nullptr;
}

/**
 * Get the monitor resolution size
 * @param hMonitor
 * @return
 */
SIZE GetMonitorResolution(HMONITOR hMonitor) {
  SIZE size = {};
  MONITORINFOEX miex;
  miex.cbSize = sizeof(miex);
  if (!GetMonitorInfo(hMonitor, &miex)) {
    return size;
  }
  DEVMODE dm;
  dm.dmSize = sizeof(dm);
  dm.dmDriverExtra = 0;

  //ENUM_CURRENT_SETTINGS: query current monitor resolution
  //ENUM_REGISTRY_SETTINGS: query current monitor resolution in REGISTRY
  if (!EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
    return size;
  }
  size.cx = static_cast<LONG>(dm.dmPelsWidth);
  size.cy = static_cast<LONG>(dm.dmPelsHeight);
  return size;
}

bool GetScreenshotImageByteData(unsigned char** image_bytes,
                                unsigned int* width,
                                unsigned int* height,
                                unsigned int* bytes_len) {
  // 1. get mouse located monitor
  HMONITOR h_monitor = GetPointerLocatedMonitor();
  // 1.1 get monitor resolution
  SIZE resolution = GetMonitorResolution(h_monitor);
  auto monitor_width = resolution.cx;
  auto monitor_height = resolution.cy;
  // 1.2 get monitor location in entire desktop
  MONITORINFO info;
  info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(GetPointerLocatedMonitor(), &info);
  auto monitor_left = info.rcMonitor.left;
  auto monitor_top = info.rcMonitor.top;
  // todo handle "left" and "top" value to real location.

  // 2. prepare monitor HDC and create memory HDC for storing monitor data
  // prepare monitor HDC (read only)
  HDC hdc_monitor = GetDC(nullptr);
  // prepare memory monitor HDC (writable)
  HDC hdc_mem_monitor = CreateCompatibleDC(hdc_monitor);
  // create monitor HBITMAP, and "connected" by hdc_mem_monitor
  HBITMAP hbitmap_monitor = CreateCompatibleBitmap(hdc_monitor, monitor_width, monitor_height);
  auto null_bitmap = SelectObject(hdc_mem_monitor, hbitmap_monitor);

  // 3. get img data from desktop specified area.
  // entire desktop HWND(multi monitor)
  HWND hwnd_entire_desktop = GetDesktopWindow();
  // entire desktop HDC from HWND
  HDC hdc_entire_desktop = GetDC(hwnd_entire_desktop);
  // copy specified area from entire desktop to hdc_mem_monitor
  BitBlt(hdc_mem_monitor,
         0,
         0,
         monitor_width,
         monitor_height,
         hdc_entire_desktop,
         monitor_left,
         monitor_top,
         SRCCOPY | CAPTUREBLT);

  // generate the BITMAP from the HBITMAP.
  BITMAP bmp_monitor;
  GetObject(hbitmap_monitor, sizeof(BITMAP), &bmp_monitor);

  // prepare bitmap info header
  // record info of the bitmap
  BITMAPINFOHEADER bih;
  bih.biSize = sizeof(BITMAPINFOHEADER);
  bih.biWidth = bmp_monitor.bmWidth;
  bih.biHeight = bmp_monitor.bmHeight;
  bih.biPlanes = bmp_monitor.bmPlanes;
  bih.biBitCount = bmp_monitor.bmBitsPixel;
  bih.biCompression = BI_RGB;
  bih.biSizeImage = bmp_monitor.bmHeight * bmp_monitor.bmWidthBytes;

  // get bmp raw byte data
  auto* bmp_rgb_buff = new unsigned char[bih.biSizeImage];
  GetDIBits(hdc_mem_monitor,
            hbitmap_monitor,
            0L,
            bmp_monitor.bmHeight,
            bmp_rgb_buff,
            reinterpret_cast<LPBITMAPINFO>(&bih),
            (DWORD)DIB_RGB_COLORS);

  // prepare bitmap file header obj
  BITMAPFILEHEADER bfh = {0};
  bfh.bfType = (static_cast<WORD>('M' << 8) | 'B');
  bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bih.biSizeImage;
  bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  // BMP completely data (file header + info header + image data)
  auto* bmp_bytes = new unsigned char[bfh.bfSize];
  auto pos = bmp_bytes;
  memcpy(pos, &bfh, sizeof(BITMAPFILEHEADER));
  pos += sizeof(BITMAPFILEHEADER);
  memcpy(pos, &bih, sizeof(BITMAPINFOHEADER));
  pos += sizeof(BITMAPINFOHEADER);
  memcpy(pos, bmp_rgb_buff, bih.biSizeImage);

  // === test ===
  // HANDLE hFile1 = CreateFile("test1.bmp", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  // DWORD dwWrite1;
  // WriteFile(hFile1, bmp_bytes, bfh.bfSize, &dwWrite1, nullptr);
  // CloseHandle(hFile1);
  // === test ===

  // clean up all
  ReleaseDC(hwnd_entire_desktop, hdc_entire_desktop);
  SelectObject(hdc_mem_monitor, null_bitmap);
  DeleteDC(hdc_mem_monitor);
  //  *image_bytes = monitor_bmp_bytes;
  //  *width = bmp_monitor.bmWidth;
  //  *height = bmp_monitor.bmHeight;
  //  *bytes_len = bmp_data_size;
  return false;
}
}
