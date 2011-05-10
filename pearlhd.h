#ifndef __PEARLHD_H
#define __PEARLHD_H

#include <vdr/skins.h>
#include <vdr/skinclassic.h>

class cSkinPearlHD : public cSkin {
  private:
    cSkin *skinFallback;
  
  public:
    cSkinPearlHD();
    virtual const char *Description(void);
    virtual cSkinDisplayChannel *DisplayChannel(bool WithInfo);
    virtual cSkinDisplayMenu *DisplayMenu(void);
    virtual cSkinDisplayReplay *DisplayReplay(bool ModeOnly);
    virtual cSkinDisplayVolume *DisplayVolume(void);
    virtual cSkinDisplayTracks *DisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
    virtual cSkinDisplayMessage *DisplayMessage(void);
  };
#endif //__PEARLHD
