#ifndef __SKINPEARLHD_CONFIG_H
#define __SKINPEARLHD_CONFIG_H

#include <vdr/themes.h>
#include <vdr/config.h>

class cSkinPearlHDConfig
{
private:

public:
   cSkinPearlHDConfig(void);
   ~cSkinPearlHDConfig();
   bool SetupParse(const char *Name, const char *Value);
   void SetEpgImagesDir(const char *Dir);

   int ChannelLogoPos;
   int ChannelLogoFormat;
   char ButtonRedText[20];
   char ButtonGreenText[20];
   char ButtonYellowText[20];
   char ButtonBlueText[20];
   int CryptSymbols;
   int oClockText;
   int RecShowClock;
   int DisplayEpgImages;
   int EpgImagesFormat;
   int MenuPaddingWidth;
   int MenuPaddingHeight;
   int LineHeight;
   int ChannelInfoPadding;
   int RecTitleInfoHead;
   int RecTitleInfo;
   char EpgImagesDir[100];
   bool EpgDirSet;
};

extern cSkinPearlHDConfig PearlHDConfig;

#endif //__SKINPEARLHD_CONFIG_H
