#ifdef HAVE_C295
#include <stl.h>
#endif
#include "bitmap.h"

using namespace std; //need ???
using namespace Magick;

cOSDImageBitmap::cOSDImageBitmap() {
}

cOSDImageBitmap::~cOSDImageBitmap() {
}
 
bool cOSDImageBitmap::LoadZoomed(const char *file, int zoomWidth, int zoomHeight, int zoomLeft, int zoomTop) {
  bool status;
  status = LoadImageMagick(imgkZoom, file);
  if (zoomWidth != 0)
  imgkZoom.crop(Geometry(zoomWidth, zoomHeight, zoomLeft, zoomTop));
  height = imgkZoom.rows();
  width = imgkZoom.columns();
  return status;
}

bool cOSDImageBitmap::Load(const char *file)
{
  return LoadImageMagick(imgkImage, file);
}
 
void cOSDImageBitmap::Render(cBitmap & bmp, int colors, int alpha)
{
  dsyslog("start to rande image");
  if (!loadingFailed) {
    // quantize the picture
    QuantizeImageMagick(imgkImage, colors, false);
    // generate cBitmap
    ConvertImgk2Bmp(bmp, imgkImage, colors);
  } else {
    dsyslog("can't rander image, loading failed!!!!!!!!!!!!!!!!!");
  }
}

void cOSDImageBitmap::Render(cBitmap &bmp, int wWindow, int hWindow, int colors, bool dither) {
  int w = wWindow;
  int h = hWindow;
  int wNew, hNew;
  wNew = wWindow;
  hNew = hWindow;
  if (!loadingFailed)	{
    Image imgkRender = imgkImage;
    width = imgkRender.columns();
    height = imgkRender.rows();
    if (height != h || width != w) {
      imgkRender.scale(Geometry(wNew, hNew, 0, 0) );
      width = imgkRender.columns();
      height = imgkRender.rows();
    }
 	QuantizeImageMagick(imgkRender, colors, dither);
 	ConvertImgk2Bmp(bmp, imgkRender, colors);
  }
}
 
bool cOSDImageBitmap::LoadImageMagick(Image &imgkLoad, const char *file) {
  try {
    imgkLoad.read(file);
    if (imgkLoad.fileSize() == 0) {
      loadingFailed = true;
      return false;
    }
    else {
      height = imgkLoad.baseRows();
      width = imgkLoad.baseColumns();
      origWidth = width;
      origHeight = height;
      loadingFailed = false;
      return true;
    }
  }
  catch(exception &error)
  {
    loadingFailed = true;
    return false;
  }
}

void cOSDImageBitmap::QuantizeImageMagick(Image &imgkQuant, int colors, bool dither) {
  if (colors < 24)
  {
    imgkQuant.quantizeColors(colors);
    imgkQuant.quantizeDither(dither);
  }
  imgkQuant.quantize();
}

void cOSDImageBitmap::ConvertImgk2Bmp(cBitmap &bmp, Image &imgkConv, int colors) {
  int w = Width();
  int h = Height();
  tColor col;
  bmp.SetSize(w, h);
  bmp.SetBpp(colors);
  const PixelPacket *pixels = imgkConv.getConstPixels(0, 0, w, h);
  for (int iy = 0; iy < h; iy++) {
    for (int ix = 0; ix < w; ix++) {
      col = (0xFF << 24) 
	    | ( (pixels->green * 255 / MaxRGB) << 8) 
        | ( (pixels->red * 255 / MaxRGB) << 16) 
        | ( (pixels->blue * 255 / MaxRGB) );
      bmp.DrawPixel(ix, iy, col);
      pixels++;
    }
  }
}
