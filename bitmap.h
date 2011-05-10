#ifndef _OSDIMAGE_BITMAP_H_
#define _OSDIMAGE_BITMAP_H_

#define X_DISPLAY_MISSING

#include <vdr/osd.h>
#include <vdr/skins.h>
#include <Magick++.h>
 
using namespace Magick;
 
class cOSDImageBitmap {
public:
  cOSDImageBitmap();
  ~cOSDImageBitmap();
  bool LoadZoomed(const char *file, int zoomWidth, int zoomHeight, int zoomLeft, int zoomTop);
  bool Load(const char *file);
  void Save(const char *file);
  void Render(cBitmap &bmp, int wWindow, int hWindow, int colors, bool dither);    
  void Render(cBitmap &bmp, int colors, int alpha=255);
  inline int Width() { return width; }
  inline int Height() { return height; }

private:	
  bool LoadImageMagick(Image &imgkLoad, const char *file);
  void QuantizeImageMagick(Image &imgkQuant, int colors, bool dither);
  void ConvertImgk2Bmp(cBitmap &bmp, Image &imgkConv, int colors);
  Image imgkZoom, imgkImage;
  int ZoomWidth, ZoomHeight, ZoomLeft, ZoomTop;
  int origWidth, origHeight;
  bool loadingFailed;
  int width, height;
};

#endif
