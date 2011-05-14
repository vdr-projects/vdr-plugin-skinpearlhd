#include "bitmap.h"
#include <math.h>

using namespace Magick;

cOSDImageBitmap::cOSDImageBitmap() {
}

cOSDImageBitmap::~cOSDImageBitmap() {
}

bool cOSDImageBitmap::Load(cBitmap &bmp, const char *Filename, int width, int height, int bpp)
{
  try
  {
    #ifdef DEBUG_SKINPEARLHD
    int start = cTimeMs::Now();
	#endif

    int w, h;
    if (bpp > 8)
	  bpp = 8;
	int colors = pow(2,bpp);
	Image osdImage;
	osdImage.read(Filename);

	if (bpp != 0)
	{
	  osdImage.opacity(OpaqueOpacity);
	  osdImage.backgroundColor( Color(0, 0, 0, 0));
	  osdImage.quantizeColorSpace(RGBColorspace);
	  osdImage.quantizeColors(colors);
	  osdImage.quantize();
	}

	if (height != 0 || width != 0)
	  osdImage.sample( Geometry(width, height));
	w = osdImage.columns();
	h = osdImage.rows();
	bmp.SetSize(w, h);
	
	const PixelPacket *pixels = osdImage.getConstPixels(0, 0, w, h);
	
	for (int iy = 0; iy < h; ++iy) {
      for (int ix = 0; ix < w; ++ix) {
        tColor col = (~int(pixels->opacity * 255 / MaxRGB) << 24) 
	      | (int(pixels->green * 255 / MaxRGB) << 8) 
          | (int(pixels->red * 255 / MaxRGB) << 16) 
          | (int(pixels->blue * 255 / MaxRGB) );
        bmp.DrawPixel(ix, iy, col);
        ++pixels;
      }
    }
	#ifdef DEBUG_SKINPEARLHD
    printf ("skinpearlhd: bitmap render took %d ms\n", int(cTimeMs::Now()-start));
	#endif
	return true;
  }
  catch (...)
  {
    return false;
  }
}

#if VDRVERSNUM > 10716
bool cOSDImageBitmap::Load(cImage &bmp, const char *Filename, int width, int height)
{
  try
  {
    #ifdef DEBUG_SKINPEARLHD
    int start = cTimeMs::Now();
	#endif
	
    int w, h;
	Image osdImage;
	osdImage.read(Filename);

	if (height != 0 || width != 0)
	  osdImage.sample( Geometry(width, height));
	w = osdImage.columns();
	h = osdImage.rows();
	
	const PixelPacket *pixels = osdImage.getConstPixels(0, 0, w, h);
	
	for (int iy = 0; iy < h; ++iy) {
      for (int ix = 0; ix < w; ++ix) {
        tColor col = (~int(pixels->opacity * 255 / MaxRGB) << 24) 
	      | (int(pixels->green * 255 / MaxRGB) << 8) 
          | (int(pixels->red * 255 / MaxRGB) << 16) 
          | (int(pixels->blue * 255 / MaxRGB) );
        bmp.SetPixel(cPoint(ix, iy), col);
        ++pixels;
      }
    }
	#ifdef DEBUG_SKINPEARLHD
    printf ("skinpearlhd: bitmap render took %d ms\n", int(cTimeMs::Now()-start));
	#endif
	return true;
  }
  catch (...)
  {
    return false;
  }
}
#endif
