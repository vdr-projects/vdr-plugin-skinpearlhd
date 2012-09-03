#include <vdr/menuitems.h>
#include "config.h"
#include "setup.h"

extern cSkinPearlHDConfig PearlHDConfig;

// --- cSkinPearlHDSetupMenu ------------------------------------------------------
class cSkinPearlHDSetupMenu : public cOsdMenu
{
private:
   cSkinPearlHDConfig *tmpconfig;
   void Setup(void);
   const char *EpgImagesFormatItems[2];
   const char *RecTitleInfoItems[3];

protected:
   virtual eOSState ProcessKey(eKeys Key);

public:
   cSkinPearlHDSetupMenu(cSkinPearlHDConfig *tmpPearlHDConfig);
   virtual ~cSkinPearlHDSetupMenu();
};

cSkinPearlHDSetupMenu::cSkinPearlHDSetupMenu(cSkinPearlHDConfig *tmpConfig)
:cOsdMenu("", 33)
{
   SetTitle(cString::sprintf("%s - '%s' %s", trVDR("Setup"), "PearlHD", tr("General")));

   tmpconfig = tmpConfig;
   EpgImagesFormatItems[0] = "png";
   EpgImagesFormatItems[1] = "jpg";
   
   RecTitleInfoItems[0] = tr("Don't show");
   RecTitleInfoItems[1] = tr("Top");
   RecTitleInfoItems[2] = tr("Bottom");

   Setup();
}

cSkinPearlHDSetupMenu::~cSkinPearlHDSetupMenu()
{
}


void cSkinPearlHDSetupMenu::Setup(void)
{
  Add(new cMenuEditBoolItem(tr("Display oClock text"), &tmpconfig->oClockText));
  Add(new cMenuEditBoolItem(tr("Display clock in Recordings/Timer menu"), &tmpconfig->RecShowClock));
  Add(new cMenuEditBoolItem(tr("Display epg images"), &tmpconfig->DisplayEpgImages));
  Add(new cMenuEditBoolItem(tr("Use new epg images filename format"), &tmpconfig->EpgNewFilename));
  Add(new cMenuEditStraItem(tr("Epg images format"), &tmpconfig->EpgImagesFormat, 2, EpgImagesFormatItems));
  Add(new cMenuEditIntItem(tr("Padding sides"), &tmpconfig->MenuPaddingWidth));
  Add(new cMenuEditIntItem(tr("Padding top/bottom"), &tmpconfig->MenuPaddingHeight));
  Add(new cMenuEditIntItem(tr("Item line height"), &tmpconfig->LineHeight));
  Add(new cMenuEditBoolItem(tr("Show running recordings in the header"), &tmpconfig->RecTitleInfoHead));
  Add(new cMenuEditStraItem(tr("Show timers in the right bar at"), &tmpconfig->RecTitleInfo, 3, RecTitleInfoItems));
}

eOSState cSkinPearlHDSetupMenu::ProcessKey(eKeys Key)
{
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (state == osUnknown) {
      switch (Key) {
         case kOk:
         case kBack:
               state = osBack;
               break;
         default: break;
      }
   }

   return state;
}

// --- cSkinPearlHDSetupChannelInfo ------------------------------------------------------
class cSkinPearlHDSetupChannelInfo : public cOsdMenu
{
private:
   cSkinPearlHDConfig *tmpconfig;
   void Setup(void);
   const char * ChannelLogoPosItems[3];
   const char * ChannelLogoFormatItems[2];

protected:
   virtual eOSState ProcessKey(eKeys Key);

public:
   cSkinPearlHDSetupChannelInfo(cSkinPearlHDConfig *tmpConfig);
   virtual ~cSkinPearlHDSetupChannelInfo();
};

cSkinPearlHDSetupChannelInfo::cSkinPearlHDSetupChannelInfo(cSkinPearlHDConfig *tmpConfig)
:cOsdMenu("", 33)
{
   SetTitle(cString::sprintf("%s - '%s' %s", trVDR("Setup"), "PearlHD", tr("Channel Info")));

   tmpconfig = tmpConfig;
   
   ChannelLogoPosItems[0] = tr("Don't show");
   ChannelLogoPosItems[1] = tr("Left");
   ChannelLogoPosItems[2] = tr("Right");
   
   ChannelLogoFormatItems[0] = "png";
   ChannelLogoFormatItems[1] = "jpg";

   Setup();
}

cSkinPearlHDSetupChannelInfo::~cSkinPearlHDSetupChannelInfo()
{
}

void cSkinPearlHDSetupChannelInfo::Setup(void)
{
  Add(new cMenuEditStraItem(tr("Channel logo position"), &tmpconfig->ChannelLogoPos, 3,  ChannelLogoPosItems));
  Add(new cMenuEditStraItem(tr("Channel logo format"), &tmpconfig->ChannelLogoFormat, 2,  ChannelLogoFormatItems));
  Add(new cMenuEditStrItem(tr("Button red text"), &tmpconfig->ButtonRedText[0], 20));
  Add(new cMenuEditStrItem(tr("Button green text"), &tmpconfig->ButtonGreenText[0], 20));
  Add(new cMenuEditStrItem(tr("Button yellow text"), &tmpconfig->ButtonYellowText[0], 20));
  Add(new cMenuEditStrItem(tr("Button blue text"), &tmpconfig->ButtonBlueText[0], 20));
  Add(new cMenuEditBoolItem(tr("Display crypt symbols"), &tmpconfig->CryptSymbols));
  Add(new cMenuEditIntItem(tr("Padding sides"), &tmpconfig->ChannelInfoPadding));
}

eOSState cSkinPearlHDSetupChannelInfo::ProcessKey(eKeys Key)
{
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (state == osUnknown) {
      switch (Key) {
         case kOk:
         case kBack:
               state = osBack;
               break;
         default: break;
      }
   }
   return state;
}


// --- cSkinPearlHDSetup ------------------------------------------------------
cSkinPearlHDSetup::cSkinPearlHDSetup(void)
{
   tmpPearlHDConfig = PearlHDConfig;

   Setup();
}

cSkinPearlHDSetup::~cSkinPearlHDSetup()
{
}

void cSkinPearlHDSetup::Setup(void)
{
   Add(new cOsdItem(tr("Channel Info"), osUser1));
   Add(new cOsdItem(tr("Menu"), osUser2));
}

eOSState cSkinPearlHDSetup::ProcessKey(eKeys Key)
{
   bool hadSubMenu = HasSubMenu();
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (hadSubMenu && !HasSubMenu() && Key == kOk)
      Store();

   switch (state) {
      case osUser1:
         AddSubMenu(new cSkinPearlHDSetupChannelInfo(&tmpPearlHDConfig));
         state=osContinue;
         break;
      case osUser2:
	     AddSubMenu(new cSkinPearlHDSetupMenu(&tmpPearlHDConfig));
         state=osContinue;
         break;
      default:
         break;
   }
   return state;
}

void cSkinPearlHDSetup::Store(void)
{
   PearlHDConfig = tmpPearlHDConfig;

   SetupStore("ChannelLogoPos", PearlHDConfig.ChannelLogoPos);
   SetupStore("ChannelLogoFormat", PearlHDConfig.ChannelLogoFormat);
   SetupStore("ButtonRedText", PearlHDConfig.ButtonRedText);
   SetupStore("ButtonGreenText", PearlHDConfig.ButtonGreenText);
   SetupStore("ButtonYellowText", PearlHDConfig.ButtonYellowText);
   SetupStore("ButtonBlueText", PearlHDConfig.ButtonBlueText);
   SetupStore("CryptSymbols", PearlHDConfig.CryptSymbols);
   SetupStore("oClockText", PearlHDConfig.oClockText);
   SetupStore("RecShowClock", PearlHDConfig.RecShowClock);
   SetupStore("DisplayEpgImages", PearlHDConfig.DisplayEpgImages);
   SetupStore("EpgImagesFormat", PearlHDConfig.EpgImagesFormat);
   SetupStore("MenuPaddingWidth", PearlHDConfig.MenuPaddingWidth);
   SetupStore("MenuPaddingHeight", PearlHDConfig.MenuPaddingHeight);
   SetupStore("LineHeight", PearlHDConfig.LineHeight);
   SetupStore("ChannelInfoPadding", PearlHDConfig.ChannelInfoPadding);
   SetupStore("RecTitleInfoHead", PearlHDConfig.RecTitleInfoHead);
   SetupStore("RecTitleInfo", PearlHDConfig.RecTitleInfo);
}
