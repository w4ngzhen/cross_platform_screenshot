#include <windows.h>
#include <vector>
#include "cp_screenshot/cp_screenshot.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")

/**
 * \brief cps inner utils on Windows.
 */
namespace cps_win_inner_utils {
/**
 * \brief [GDI Plus] Get CLSID info that specifies the format
 * \param format like
 * \param pClsid CLSID obj
 * \return true ok; false failure.
 */
static bool GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
  UINT num = 0; // number of image encoders
  UINT size = 0; // size of the image encoder array in bytes

  Gdiplus::ImageCodecInfo *pImageCodecInfo;

  // 2. Obtain the number of encoder types of image formats supported by GDI+
  // and the storage size of the ImageCodecInfo array
  Gdiplus::GetImageEncodersSize(&num, &size);
  if (size == 0) {
    return false; // Failure
  }

  // 3. Allocate enough space for the ImageCodecInfo array
  pImageCodecInfo = static_cast<Gdiplus::ImageCodecInfo *>(malloc(size));
  if (pImageCodecInfo == nullptr) {
    return false; // Failure
  }

  // 4. Get all the image encoder information
  GetImageEncoders(num, size, pImageCodecInfo);

  // 5. Find the Clsid of the image encoder that matches
  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return true; // Success
    }
  }

  // release memory
  free(pImageCodecInfo);
  return false;
}

/**
 * OS Windows
 * get the monitor info where mouse pointer located.
 * \related MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
 */
static HMONITOR GetPointerLocatedMonitor() {
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
static SIZE GetMonitorResolution(HMONITOR hMonitor) {
  SIZE size = {};
  MONITORINFOEX monitor_info_ex;
  monitor_info_ex.cbSize = sizeof(monitor_info_ex);
  if (!GetMonitorInfo(hMonitor, &monitor_info_ex)) {
    return size;
  }
  DEVMODE dm;
  dm.dmSize = sizeof(dm);
  dm.dmDriverExtra = 0;

  //ENUM_CURRENT_SETTINGS: query current monitor resolution
  //ENUM_REGISTRY_SETTINGS: query current monitor resolution in REGISTRY
  if (!EnumDisplaySettings(monitor_info_ex.szDevice, ENUM_CURRENT_SETTINGS, &dm)) {
    return size;
  }
  size.cx = static_cast<LONG>(dm.dmPelsWidth);
  size.cy = static_cast<LONG>(dm.dmPelsHeight);
  return size;
}

/**
 * get bytes data from IStream.
 * @param stream
 * @param output_bytes
 * @param size
 * @return
 */
static bool GetBytesFromStream(IStream *stream, unsigned char **output_bytes, unsigned int *size) {
  HGLOBAL h_mem = nullptr;
  if (GetHGlobalFromStream(stream, &h_mem) != S_OK) {
    return false;
  }

  void *global_data = GlobalLock(h_mem);
  auto data_size = GlobalSize(h_mem);

  // allocated heap data and copy data.
  auto heap_data = new unsigned char[data_size];
  memcpy(heap_data, global_data, data_size);

  GlobalUnlock(h_mem);

  *output_bytes = heap_data;
  *size = data_size;
  return true;
}
}

namespace cps {
bool GetScreenshotImageByteData(unsigned char **image_bytes,
                                unsigned int *width,
                                unsigned int *height,
                                unsigned int *bytes_len) {
  // 1. get mouse located monitor
  HMONITOR h_monitor = cps_win_inner_utils::GetPointerLocatedMonitor();
  // 1.1 get monitor resolution
  SIZE resolution = cps_win_inner_utils::GetMonitorResolution(h_monitor);
  auto monitor_width = resolution.cx;
  auto monitor_height = resolution.cy;
  // 1.2 get monitor location in entire desktop
  MONITORINFO info;
  info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(cps_win_inner_utils::GetPointerLocatedMonitor(), &info);
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

  // init GDI+
  Gdiplus::GdiplusStartupInput gdiplus_startup_input;
  ULONG_PTR gdiplus_token;
  GdiplusStartup(&gdiplus_token, &gdiplus_startup_input, nullptr);

  // get Gdiplus::Bitmap pointer from HBITMAP.
//  auto *bitmap = Gdiplus::Bitmap::FromHBITMAP(hbitmap_monitor, nullptr);
  auto *bitmap = Gdiplus::Bitmap::FromFile(L"sample.jpeg");

  // get the image size info.
  *width = bitmap->GetWidth();
  *height = bitmap->GetHeight();

  // now you can release resource about DC or HANDLE.
  ReleaseDC(hwnd_entire_desktop, hdc_entire_desktop);
  SelectObject(hdc_mem_monitor, null_bitmap);
  DeleteDC(hdc_mem_monitor);

  // get target encode id.
  CLSID encoderClsid;
  cps_win_inner_utils::GetEncoderClsid(L"image/jpeg", &encoderClsid);

  // set JPEG compress quality.
  Gdiplus::EncoderParameters encoderParams{};
  int quality = 100;
  encoderParams.Count = 1;
  encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
  encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
  encoderParams.Parameter[0].NumberOfValues = 1;
  encoderParams.Parameter[0].Value = &quality;

  // === test ===
  // you can test save the Bitmap to file directly.
  // bitmap->Save(L"temp-test.jpeg", &encoderClsid, &encoderParams);
  // === test ===

  // prepare a IStream
  // prepare target output image data stream.
  IStream *img_data_out_stream;
  auto res = CreateStreamOnHGlobal(nullptr, true, &img_data_out_stream);
  if (res != S_OK) {
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplus_token);
    return false;
  }

  // save bitmap data to output stream.
  bitmap->Save(img_data_out_stream, &encoderClsid, &encoderParams);

  // before read byte data from output stream,
  // you can release resource.
  delete bitmap;
  Gdiplus::GdiplusShutdown(gdiplus_token);

  unsigned char *bytes;
  unsigned int len;
  if (!cps_win_inner_utils::GetBytesFromStream(img_data_out_stream, &bytes, &len)) {
    img_data_out_stream->Release();
    return false;
  }

  *image_bytes = bytes;
  *bytes_len = len;

  img_data_out_stream->Release();
  return true;
}
}
