#include <windows.h>
#include <vector>
#include <sstream>
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
 * \return 0 ok; -1 failure.
 */
int GetEncoderClsid(const WCHAR *format, CLSID *pClsid) {
  UINT num = 0; // number of image encoders
  UINT size = 0; // size of the image encoder array in bytes

  Gdiplus::ImageCodecInfo * pImageCodecInfo = nullptr;

  //2.获取GDI+支持的图像格式编码器种类数以及ImageCodecInfo数组的存放大小
  Gdiplus::GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1; // Failure

  //3.为ImageCodecInfo数组分配足额空间
  pImageCodecInfo = static_cast<Gdiplus::ImageCodecInfo *>(malloc(size));
  if (pImageCodecInfo == nullptr)
    return -1; // Failure

  //4.获取所有的图像编码器信息
  GetImageEncoders(num, size, pImageCodecInfo);

  //5.查找符合的图像编码器的Clsid
  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j; // Success
    }
  }

  //6.释放步骤3分配的内存
  free(pImageCodecInfo);
  return -1; // Failure
}

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

/**
 * \brief Get screenshot BMP Image data and info.
 * \param image_bytes
 * \param width
 * \param height
 * \param bytes_len
 * \return
 */
bool GetScreenshotBmpImageData(unsigned char **image_bytes,
                               unsigned int *width,
                               unsigned int *height,
                               unsigned int *bytes_len) {
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
  auto *bmp_rgb_buff = new unsigned char[bih.biSizeImage];
  GetDIBits(hdc_mem_monitor,
            hbitmap_monitor,
            0L,
            bmp_monitor.bmHeight,
            bmp_rgb_buff,
            reinterpret_cast<LPBITMAPINFO>(&bih),
            (DWORD) DIB_RGB_COLORS);

  // prepare bitmap file header obj
  BITMAPFILEHEADER bfh = {0};
  bfh.bfType = (static_cast<WORD>('M' << 8) | 'B');
  bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bih.biSizeImage;
  bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

  // BMP completely data (file header + info header + image data)
  auto *bmp_bytes = new unsigned char[bfh.bfSize];
  auto pos = bmp_bytes;
  memcpy(pos, &bfh, sizeof(BITMAPFILEHEADER));
  pos += sizeof(BITMAPFILEHEADER);
  memcpy(pos, &bih, sizeof(BITMAPINFOHEADER));
  pos += sizeof(BITMAPINFOHEADER);
  memcpy(pos, bmp_rgb_buff, bih.biSizeImage);

  // clean up all
  ReleaseDC(hwnd_entire_desktop, hdc_entire_desktop);
  SelectObject(hdc_mem_monitor, null_bitmap);
  DeleteDC(hdc_mem_monitor);

  // set [out] data.
  *image_bytes = bmp_bytes;
  *width = bih.biWidth;
  *height = bih.biHeight;
  *bytes_len = bfh.bfSize;

  return true;
}
}

namespace cps {
bool GetScreenshotImageByteData(unsigned char **image_bytes,
                                unsigned int *width,
                                unsigned int *height,
                                unsigned int *bytes_len) {
  unsigned char *bmp_file_bytes;
  unsigned int bmp_bytes_len;
  unsigned int img_w;
  unsigned int img_h;
  if (!cps_win_inner_utils::GetScreenshotBmpImageData(&bmp_file_bytes, &img_w, &img_h, &bmp_bytes_len)) {
    return false;
  }

  // set width and height.
  *width = img_w;
  *height = img_h;
  // bytes and bytes_len should be set after convert to specified format.

  // === test ===
  // HANDLE hFile1 = CreateFile("test1.bmp", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  // DWORD dwWrite1;
  // WriteFile(hFile1, bmp_file_bytes, bmp_bytes_len, &dwWrite1, nullptr);
  // CloseHandle(hFile1);
  // === test ===

  // now convert BMP data to JPEG.
  // prepare BMP image data stream
  IStream *bmp_data_istream = nullptr;
  auto res = CreateStreamOnHGlobal(nullptr, true, &bmp_data_istream);
  if (res != S_OK) {
    delete bmp_data_istream;
    return false;
  }

  // create OK.
  // write data to stream
  ULONG written;
  res = bmp_data_istream->Write(bmp_file_bytes, bmp_bytes_len, &written);
  if (res != S_OK) {
    delete bmp_data_istream;
    return false;
  }

  // reset stream
  LARGE_INTEGER offset = {0};
  res = bmp_data_istream->Seek(offset, STREAM_SEEK_SET, nullptr);
  if (res != S_OK) {
    delete bmp_data_istream;
    return false;
  }

  // The BMP image data stream is ready,
  // prepare GDI context and create Image instance.

  // init GDI+
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

  // create BMP GDI image
  auto gdi_img = Gdiplus::Image::FromStream(bmp_data_istream);

  // prepare target output image data stream.
  IStream *output_img_data_stream;
  res = CreateStreamOnHGlobal(nullptr, true, &output_img_data_stream);
  if (res != S_OK) {
    bmp_data_istream->Release();
    delete gdi_img;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return false;
  }

  // get the CLSID of the JPEG encoder.
  CLSID encoderClsid;
  cps_win_inner_utils::GetEncoderClsid(L"image/jpeg", &encoderClsid);

  // now save BMP data to this stream by specified encoder.
  if (gdi_img->Save(output_img_data_stream, &encoderClsid) == Gdiplus::Status::Ok) {
    // you should reset stream
    output_img_data_stream->Seek(offset, STREAM_SEEK_SET, nullptr);
  } else {
    bmp_data_istream->Release();
    delete gdi_img;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return false;
  }
  // ===== test: save to file. ====
  // gdi_img->Save(L"test1.jpeg", &encoderClsid);
  // ===== test =====

  // now you can release resources other than the output stream.
  bmp_data_istream->Release();
  delete gdi_img;
  Gdiplus::GdiplusShutdown(gdiplusToken);

  // now read data from output stream.
  constexpr DWORD buff_size = 1024;
  BYTE buff[buff_size];
  DWORD bytes_read_num;
  std::vector<unsigned char> bytes;
  output_img_data_stream->Read(buff, buff_size, &bytes_read_num);
  while (bytes_read_num > 0) {
    for (int i = 0; i < bytes_read_num; i++) {
      bytes.push_back(buff[i]);
    }
    output_img_data_stream->Read(buff, buff_size, &bytes_read_num);
  }
  *image_bytes = bytes.data();
  *bytes_len = bytes.size();

  output_img_data_stream->Release();
  return true;
}
}
