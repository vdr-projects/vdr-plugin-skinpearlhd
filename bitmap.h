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
  bool Load(cBitmap &bmp, const char *Filename, int width=0, int height=0, int bpp=0);
  #if VDRVERSNUM > 10716
  bool Load(cImage &bmp, const char *Filename, int width, int height);
  #endif
private:	
};

#endif
