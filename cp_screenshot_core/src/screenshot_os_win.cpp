#include <windows.h>
#include "cp_screenshot/cp_screenshot.h"

//int CaptureAnImage(HWND hWnd) {
//  HDC hdcScreen;
//  HDC hdcWindow;
//  HDC hdcMemDC = NULL;
//  HBITMAP hbmScreen = NULL;
//  BITMAP bmpScreen;
//  DWORD dwBytesWritten = 0;
//  DWORD dwSizeofDIB = 0;
//  HANDLE hFile = NULL;
//  char *lpbitmap = NULL;
//  HANDLE hDIB = NULL;
//  DWORD dwBmpSize = 0;
//
//  // Retrieve the handle to a display device context for the client
//  // area of the window.
//  hdcScreen = GetDC(NULL);
//  hdcWindow = GetDC(hWnd);
//
//  // Create a compatible DC, which is used in a BitBlt from the window DC.
//  hdcMemDC = CreateCompatibleDC(hdcWindow);
//
//  if (!hdcMemDC) {
//    goto done;
//  }
//
//  // Get the client area for size calculation.
//  RECT rcClient;
//  GetClientRect(hWnd, &rcClient);
//
//  // This is the best stretch mode.
//  SetStretchBltMode(hdcWindow, HALFTONE);
//
//  // The source DC is the entire screen, and the destination DC is the current window (HWND).
//  if (!StretchBlt(hdcWindow,
//                  0, 0,
//                  rcClient.right, rcClient.bottom,
//                  hdcScreen,
//                  0, 0,
//                  GetSystemMetrics(SM_CXSCREEN),
//                  GetSystemMetrics(SM_CYSCREEN),
//                  SRCCOPY)) {
//    goto done;
//  }
//
//  // Create a compatible bitmap from the Window DC.
//  hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
//
//  if (!hbmScreen) {
//    goto done;
//  }
//
//  // Select the compatible bitmap into the compatible memory DC.
//  SelectObject(hdcMemDC, hbmScreen);
//
//  // Bit block transfer into our compatible memory DC.
//  if (!BitBlt(hdcMemDC,
//              0, 0,
//              rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
//              hdcWindow,
//              0, 0,
//              SRCCOPY)) {
//    goto done;
//  }
//
//  // Get the BITMAP from the HBITMAP.
//  GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);
//
//  BITMAPFILEHEADER bmfHeader;
//  BITMAPINFOHEADER bi;
//
//  bi.biSize = sizeof(BITMAPINFOHEADER);
//  bi.biWidth = bmpScreen.bmWidth;
//  bi.biHeight = bmpScreen.bmHeight;
//  bi.biPlanes = 1;
//  bi.biBitCount = 32;
//  bi.biCompression = BI_RGB;
//  bi.biSizeImage = 0;
//  bi.biXPelsPerMeter = 0;
//  bi.biYPelsPerMeter = 0;
//  bi.biClrUsed = 0;
//  bi.biClrImportant = 0;
//
//  dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
//
//  // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
//  // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
//  // have greater overhead than HeapAlloc.
//  hDIB = GlobalAlloc(GHND, dwBmpSize);
//  lpbitmap = (char *) GlobalLock(hDIB);
//
//  // Gets the "bits" from the bitmap, and copies them into a buffer
//  // that's pointed to by lpbitmap.
//  GetDIBits(hdcWindow, hbmScreen, 0,
//            (UINT) bmpScreen.bmHeight,
//            lpbitmap,
//            (BITMAPINFO *) &bi, DIB_RGB_COLORS);
//
//  // A file is created, this is where we will save the screen capture.
//  hFile = CreateFile(reinterpret_cast<LPCSTR>(L"captureqwsx.bmp"),
//                     GENERIC_WRITE,
//                     0,
//                     NULL,
//                     CREATE_ALWAYS,
//                     FILE_ATTRIBUTE_NORMAL, NULL);
//
//  // Add the size of the headers to the size of the bitmap to get the total file size.
//  dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
//
//  // Offset to where the actual bitmap bits start.
//  bmfHeader.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + (DWORD) sizeof(BITMAPINFOHEADER);
//
//  // Size of the file.
//  bmfHeader.bfSize = dwSizeofDIB;
//
//  // bfType must always be BM for Bitmaps.
//  bmfHeader.bfType = 0x4D42; // BM.
//
//  WriteFile(hFile, (LPSTR) &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
//  WriteFile(hFile, (LPSTR) &bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
//  WriteFile(hFile, (LPSTR) lpbitmap, dwBmpSize, &dwBytesWritten, NULL);
//
//  // Unlock and Free the DIB from the heap.
//  GlobalUnlock(hDIB);
//  GlobalFree(hDIB);
//
//  // Close the handle for the file that was created.
//  CloseHandle(hFile);
//
//  // Clean up.
//  done:
//  DeleteObject(hbmScreen);
//  DeleteObject(hdcMemDC);
//  ReleaseDC(NULL, hdcScreen);
//  ReleaseDC(hWnd, hdcWindow);
//
//  return 0;
//}

namespace cps {

/**
* OS Windows
* get the monitor info where mouse point located.
* @return
*/
HMONITOR GetPointedMonitor() {
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

bool GetScreenshotImageByteData(unsigned char **image_bytes,
                                unsigned int *width,
                                unsigned int *height,
                                unsigned int *bytes_len) {
  // 1. prepare and get monitor info struct.
  MONITORINFO info;
  info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(GetPointedMonitor(), &info);

  // 2. get target screen(which the mouse is located) rect.
  auto monitor_left = info.rcMonitor.left;
  auto monitor_top = info.rcMonitor.top;
  auto monitor_width = info.rcMonitor.right - monitor_left;
  auto monitor_height = info.rcMonitor.bottom - monitor_top;

  // prepare monitor HDC (read only)
  HDC hdc_monitor = GetDC(nullptr);
  // prepare memory monitor HDC (writable)
  HDC hdc_mem_monitor = CreateCompatibleDC(hdc_monitor);
  // create monitor HBITMAP and "connected" by hdc_mem_monitor
  HBITMAP hbitmap_monitor = CreateCompatibleBitmap(hdc_monitor, monitor_width, monitor_height);
  auto null_bitmap = SelectObject(hdc_mem_monitor, hbitmap_monitor);

  // copy data
  // entire desktop HWND(multi monitor)
  HWND hwnd_entire_desktop = GetDesktopWindow();
  // entire desktop HDC from HWND
  HDC hdc_entire_desktop = GetDC(hwnd_entire_desktop);
  // copy specified area from entire desktop to hdc_mem_monitor
  BitBlt(hdc_mem_monitor, 0, 0, monitor_width, monitor_height, hdc_entire_desktop, monitor_left, monitor_top,
         SRCCOPY | CAPTUREBLT);

  // clean up all but hbitmap_monitor
  ReleaseDC(hwnd_entire_desktop, hdc_entire_desktop);
  SelectObject(hdc_mem_monitor, null_bitmap);
  DeleteDC(hdc_mem_monitor);


  // Get the BITMAP from the HBITMAP.
  BITMAP bmp_monitor;
  GetObject(hbitmap_monitor, sizeof(BITMAP), &bmp_monitor);

  BITMAPINFOHEADER bmp_info_header;
  bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
  bmp_info_header.biWidth = bmp_monitor.bmWidth;
  bmp_info_header.biHeight = bmp_monitor.bmHeight;
  bmp_info_header.biPlanes = 1;
  bmp_info_header.biBitCount = 32;
  bmp_info_header.biCompression = BI_RGB;
  bmp_info_header.biSizeImage = 0;
  bmp_info_header.biXPelsPerMeter = 0;
  bmp_info_header.biYPelsPerMeter = 0;
  bmp_info_header.biClrUsed = 0;
  bmp_info_header.biClrImportant = 0;

  DWORD bmp_size = ((bmp_monitor.bmWidth * bmp_info_header.biBitCount + 31) / 32) * 4 * bmp_monitor.bmHeight;

  // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
  // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
  // have greater overhead than HeapAlloc.
  HGLOBAL hDIB = GlobalAlloc(GHND, bmp_size);
  auto *screenshot_bmp_bytes = (unsigned char *) GlobalLock(hDIB);
  // Gets the "bits" from the bitmap, and copies them into a buffer
  // that's pointed to by screenshot_bmp_bytes.
  GetDIBits(hdc_mem_monitor, hbitmap_monitor, 0,
            (UINT) bmp_monitor.bmHeight,
            screenshot_bmp_bytes,
            (BITMAPINFO *) &bmp_info_header, DIB_RGB_COLORS);

  HANDLE h_file = CreateFile(reinterpret_cast<LPCSTR>(L"captureqwsx.bmp"),
                             GENERIC_WRITE,
                             0,
                             nullptr,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, nullptr);

  BITMAPFILEHEADER bmfHeader;
  // Add the size of the headers to the size of the bitmap to get the total file size.
  DWORD dwSizeofDIB = bmp_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  // Offset to where the actual bitmap bits start.
  bmfHeader.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + (DWORD) sizeof(BITMAPINFOHEADER);

  // Size of the file.
  bmfHeader.bfSize = dwSizeofDIB;

  // bfType must always be BM for Bitmaps.
  bmfHeader.bfType = 0x4D42; // BM.

  DWORD dwBytesWritten = 0;
  WriteFile(h_file, (LPSTR) &bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, nullptr);
  WriteFile(h_file, (LPSTR) &bmp_info_header, sizeof(BITMAPINFOHEADER), &dwBytesWritten, nullptr);
  WriteFile(h_file, (LPSTR) screenshot_bmp_bytes, bmp_size, &dwBytesWritten, nullptr);

  // Unlock and Free the DIB from the heap.
  GlobalUnlock(hDIB);
  GlobalFree(hDIB);

  // Close the handle for the file that was created.
  CloseHandle(h_file);

//  *image_bytes = screenshot_bmp_bytes;
//  *width = bmp_monitor.bmWidth;
//  *height = bmp_monitor.bmHeight;
//  *bytes_len = bmp_size;

  return false;
}
}

