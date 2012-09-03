#include "config.h"

cSkinPearlHDConfig PearlHDConfig;

cTheme Theme;

// --- cSkinPearlHDConfig ----------------------------------------------------------
cSkinPearlHDConfig::cSkinPearlHDConfig(void)
{
   ChannelLogoPos = 0;
   ChannelLogoFormat = 0;
   CryptSymbols = 0;
   oClockText = 1;
   RecShowClock = 1;
   DisplayEpgImages = 0;
   EpgImagesFormat = 1;
   MenuPaddingWidth = 240;
   MenuPaddingHeight = 135;
   LineHeight = 50;
   ChannelInfoPadding = 240;
   RecTitleInfoHead = 0;
   RecTitleInfo = 0;
   *EpgImagesDir = NULL;
   EpgDirSet = false;
   EpgNewFilename = 1;
}

cSkinPearlHDConfig::~cSkinPearlHDConfig()
{
}

bool cSkinPearlHDConfig::SetupParse(const char *Name, const char *Value)
{
   if      (strcmp(Name, "ChannelLogoPos") == 0)               ChannelLogoPos = atoi(Value);
   else if (strcmp(Name, "ChannelLogoFormat") == 0)            ChannelLogoFormat = atoi(Value);
   else if (strcmp(Name, "ButtonRedText") == 0)                strncpy(ButtonRedText, Value, 20);
   else if (strcmp(Name, "ButtonGreenText") == 0)              strncpy(ButtonGreenText, Value, 20);
   else if (strcmp(Name, "ButtonYellowText") == 0)             strncpy(ButtonYellowText, Value, 20);
   else if (strcmp(Name, "ButtonBlueText") == 0)               strncpy(ButtonBlueText, Value, 20);
   else if (strcmp(Name, "CryptSymbols") == 0)                 CryptSymbols = atoi(Value);
   else if (strcmp(Name, "oClockText") == 0)                   oClockText = atoi(Value);
   else if (strcmp(Name, "RecShowClock") == 0)                 RecShowClock = atoi(Value);
   else if (strcmp(Name, "DisplayEpgImages") == 0)             DisplayEpgImages = atoi(Value);
   else if (strcmp(Name, "EpgImagesFormat") == 0)              EpgImagesFormat = atoi(Value);
   else if (strcmp(Name, "MenuPaddingWidth") == 0)             MenuPaddingWidth = atoi(Value);
   else if (strcmp(Name, "MenuPaddingHeight") == 0)            MenuPaddingHeight = atoi(Value);
   else if (strcmp(Name, "LineHeight") == 0)                   LineHeight = atoi(Value);
   else if (strcmp(Name, "ChannelInfoPadding") == 0)           ChannelInfoPadding = atoi(Value);
   else if (strcmp(Name, "RecTitleInfoHead") == 0)             RecTitleInfoHead = atoi(Value);
   else if (strcmp(Name, "RecTitleInfo") == 0)                 RecTitleInfo = atoi(Value);
   else if (strcmp(Name, "EpgNewFilename") == 0)               EpgNewFilename = atoi(Value);
   else return false;
   return true;
}

 void cSkinPearlHDConfig::SetEpgImagesDir(const char *Dir)
 {
   strncpy(EpgImagesDir, Dir, 100);
   EpgDirSet = true;
 }
