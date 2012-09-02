#include "pearlhd.h"
#include "bitmap.h"
#include "config.h"
#include "services/epgsearchservices.h"

#include <string>
#include <sstream>
#include <sys/time.h>

#include <vdr/config.h>
#include <vdr/device.h>
#include <vdr/timers.h>
#include <vdr/menu.h>
#include <vdr/font.h>
#include <vdr/osd.h>
#include <vdr/themes.h>
#include <vdr/plugin.h>
#include <vdr/tools.h>
#if VDRVERSNUM > 10727
#include <vdr/videodir.h>
#endif
/*
#define MENU_TYPE_VDR 1
#define MENU_TYPE_SCHEDULE 2
#define MENU_TYPE_RECORDINGS 3
#define MENU_TYPE_RECORDING 4
#define MENU_TYPE_EVENT 5*/

static cTheme Theme;

THEME_CLR(Theme, clrMainLight,             0x80000000);
THEME_CLR(Theme, clrMainSolid,             0xCC000000);
THEME_CLR(Theme, clrBar,                   0x80bbe5ff);
THEME_CLR(Theme, clrBarFill,               0x803b86b6);
THEME_CLR(Theme, clrBarBlock,              0x80ff6969);
THEME_CLR(Theme, clrFontColor,             0xFFFFFFFF);

THEME_CLR(Theme, clrDivider,               0xCC8c8c8c);

THEME_CLR(Theme, clrSymbolGreen,           0x6000ff00);
THEME_CLR(Theme, clrSymbolRed,             0xAAff0000);

THEME_CLR(Theme, clrStatus,                0xAA6d8c00);
THEME_CLR(Theme, clrWarning,               0xAAff8a00);
THEME_CLR(Theme, clrError,                 0xAAff0000);

THEME_CLR(Theme, clrLoLight,               0x95000000);
THEME_CLR(Theme, clrFontColorInactive,     0x993b86b6);
THEME_CLR(Theme, clrMessageWhite,          0xffffffff);
THEME_CLR(Theme, clrMessageBlack,          0xff000000);

THEME_CLR(Theme, clrMark,                  0xFFe39b00);
THEME_CLR(Theme, clrKeep,                  0xFF6d8c00);

THEME_CLR(Theme, clrMainSolidOverlay,      0xDD000000);

// --- cSkinPearlHDDisplayChannel ----------------------------------------------

class cSkinPearlHDDisplayChannel : public cSkinDisplayChannel {
private:
  cOsd *osd;
  
  int heightChannelBar;
  int heightProgressBar;
  int heightInfoBar;
  int heightButtons;
  int widthButtons;
  
  int heightClock;
  int widthClock;
  
  int x1ChannelInfo;
  int x2ChannelInfo;
  int y1ChannelInfo;
  int y2ChannelInfo;
  int xSymbolStart;
  int ySymbol;
  int symbolWidth;
  int x1DateTime;
  int x2DateTime;
  int y1DateTime;
  int x1Message;
  int x2Message;
  int y1Message;
  int y2Message;
  int bpp;
  bool isGroupSep;
  unsigned long blinkNext;
  int blinkState;
  bool withInfo;
  int lineHeight;
  char lastdatetime[50];
  const cFont *fontSansBook15;
  const cFont *fontSansBook27;
  const cFont *fontSansBook37;
  const cFont *fontSansBold16;
  const cFont *fontCryptSymbols;
  std::string logoPath, logoFormat;
  #if VDRVERSNUM > 10716
  cPixmap *logoPixmap;
  #endif
  void DrawRedButton(int xButton, int yButton);
  void DrawGreenButton(int xButton, int yButton);
  void DrawYellowButton(int xButton, int yButton);
  void DrawBlueButton(int xButton, int yButton);
  void DrawCryptSymbol(int xSymbol, int ySymbol, const char *Text, tColor color, tColor bgcolor);
public:
  cSkinPearlHDDisplayChannel(bool WithInfo);
  virtual ~cSkinPearlHDDisplayChannel();
  virtual void SetChannel(const cChannel *Channel, int Number);
  virtual void SetEvents(const cEvent *Present, const cEvent *Following);
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };

cSkinPearlHDDisplayChannel::cSkinPearlHDDisplayChannel(bool WithInfo)
{
  bpp = 0;
  fontSansBook15 = cFont::CreateFont("VDRSymbols Sans:Book", 15);
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);
  fontSansBook37 = cFont::CreateFont("VDRSymbols Sans:Book", 37);
  fontSansBold16 = cFont::CreateFont("VDRSymbols Sans:Bold", 16);
  fontCryptSymbols = cFont::CreateFont("VDRSymbols Sans:Bold", 9);
  x1ChannelInfo = PearlHDConfig.ChannelInfoPadding;
  x2ChannelInfo = cOsd::OsdWidth() - PearlHDConfig.ChannelInfoPadding;
  y1ChannelInfo = cOsd::OsdHeight() - 340;
  y2ChannelInfo = cOsd::OsdHeight() - 55;
  x1DateTime = cOsd::OsdWidth() - 460;
  x2DateTime = cOsd::OsdWidth();
  y1DateTime = 0;
  x1Message = 480;
  x2Message = cOsd::OsdWidth() - 1;
  y1Message = cOsd::OsdHeight() - 70;
  y2Message = cOsd::OsdHeight() - 1;
  xSymbolStart = x2ChannelInfo-255;
  ySymbol = y2ChannelInfo-278;
  symbolWidth = 50;
  
  blinkNext=0;
  blinkState=0;
  
  strcpy (lastdatetime, "");

  osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop());
  tArea Areas[] = { { 0, 0, cOsd::OsdWidth() - 1, cOsd::OsdHeight() - 1, 32 } }; // TrueColor
  if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
  {
    osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
    bpp = 24;
  }
  else
  {
    tArea Areas[] = { { 0, 0, cOsd::OsdWidth() - 1, y2ChannelInfo - 1, 8 },  
	  { x2ChannelInfo - 957, y2ChannelInfo, x2ChannelInfo - 478, cOsd::OsdHeight() - 1, 8 }, 
      { x2ChannelInfo - 477, y2ChannelInfo, x2ChannelInfo - 1, cOsd::OsdHeight() - 1, 8 } }; // 256 colors
    if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
    {
      bpp = 8;
      osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
    }
  }
  
  if ((SysLogLevel == 3) && (bpp == 24))
    printf ("skinpearlhd: using truecolor mode\n");
  if ((SysLogLevel == 3) && (bpp == 8))
    printf ("skinpearlhd: using 256 color mode\n");
		
  osd->DrawRectangle(cOsd::OsdLeft(), cOsd::OsdTop(), cOsd::OsdWidth(), cOsd::OsdHeight(), clrTransparent);
	
  if (strcmp(PearlHDConfig.ButtonRedText, "") != 0)
  {
    DrawRedButton(x2ChannelInfo - 957, y2ChannelInfo);
    osd->DrawText(x2ChannelInfo - 957, y2ChannelInfo, PearlHDConfig.ButtonRedText, Theme.Color(clrMessageWhite), clrTransparent, fontSansBook27, 239, 0, taCenter);
  }
  if (strcmp(PearlHDConfig.ButtonGreenText, "") != 0)
  {
    DrawGreenButton(x2ChannelInfo - 717, y2ChannelInfo);
    osd->DrawText(x2ChannelInfo - 717, y2ChannelInfo, PearlHDConfig.ButtonGreenText, Theme.Color(clrMessageBlack), clrTransparent, fontSansBook27, 237, 0, taCenter);
  }
  if (strcmp(PearlHDConfig.ButtonYellowText, "") != 0)
  {
    DrawYellowButton(x2ChannelInfo - 477, y2ChannelInfo);
    osd->DrawText(x2ChannelInfo - 477, y2ChannelInfo, PearlHDConfig.ButtonYellowText, Theme.Color(clrMessageBlack), clrTransparent, fontSansBook27, 237, 0, taCenter);
  }
  if (strcmp(PearlHDConfig.ButtonBlueText, "") != 0)
  {
    DrawBlueButton(x2ChannelInfo - 237, y2ChannelInfo);
    osd->DrawText(x2ChannelInfo - 237, y2ChannelInfo, PearlHDConfig.ButtonBlueText, Theme.Color(clrMessageWhite), clrTransparent, fontSansBook27, 237, 0, taCenter);
  }
  
  logoPath = cPlugin::ConfigDirectory();
  logoPath += "/skinpearlhd/logos/";
  switch (PearlHDConfig.ChannelLogoFormat)
  {
    case 0:
	  logoFormat = "png";
	break;
	case 1:
	  logoFormat = "jpg";
	break;
  }
}

cSkinPearlHDDisplayChannel::~cSkinPearlHDDisplayChannel()
{
  delete osd;
}

void cSkinPearlHDDisplayChannel::SetChannel(const cChannel *Channel, int Number)
{
  osd->DrawEllipse(x1ChannelInfo, y1ChannelInfo, x1ChannelInfo + 50, y1ChannelInfo + 50, Theme.Color(clrMainLight), 2);
  osd->DrawRectangle(x1ChannelInfo + 50, y1ChannelInfo, x2ChannelInfo, y1ChannelInfo + 50, Theme.Color(clrMainLight));
  osd->DrawRectangle(x1ChannelInfo, y1ChannelInfo + 70, x2ChannelInfo, y2ChannelInfo, Theme.Color(clrMainSolid));

  std::string displayLogoPath = logoPath + Channel->Name() + "." + logoFormat;
  cOSDImageBitmap osdbitmap;

  switch (PearlHDConfig.ChannelLogoPos)
  {
    case 1 :
	  #if VDRVERSNUM > 10716
	  if (bpp > 8)
	  {
	    osd->DestroyPixmap(logoPixmap);
	    if(osdbitmap.Load(displayLogoPath.c_str(), 64, 48))
        {
          cImage logo = osdbitmap.GetImage();
	      logoPixmap = osd->CreatePixmap(0, cRect(x1ChannelInfo+120, y1ChannelInfo, logo.Width(), logo.Height()));
	      logoPixmap->DrawImage(cPoint(0, 0), logo);
        }
	  }
      else
	  #endif
	  {
	    cBitmap logo (1, 1, bpp);
	    if(osdbitmap.Load(logo, displayLogoPath.c_str(), 64, 48)){
          osd->DrawBitmap(x1ChannelInfo+120, y1ChannelInfo, logo);
        }
	  }
	break;
	
	case 2 :
	  #if VDRVERSNUM > 10716
	  if (bpp > 8)
	  {
	    osd->DestroyPixmap(logoPixmap);
	    if(osdbitmap.Load(displayLogoPath.c_str(), 120, 100))
        {
          cImage logo = osdbitmap.GetImage();
	      logoPixmap = osd->CreatePixmap(0, cRect(x2ChannelInfo-125, y2ChannelInfo-110, logo.Width(), logo.Height()));
	      logoPixmap->DrawImage(cPoint(0, 0), logo);
        }
	  }
      else
	  #endif
	  {
	    cBitmap logo (1, 1, bpp);
	    if(osdbitmap.Load(logo, displayLogoPath.c_str(), 120, 100)){
          osd->DrawBitmap(x2ChannelInfo-125, y2ChannelInfo-110, logo);
        }
	  }
	break;
  }
  
  if (Channel && !Channel->GroupSep()) {
    isGroupSep = false;
    char sNumber[50];
	switch (PearlHDConfig.ChannelLogoPos)
    {
	  case 0 :
	  case 2 :
	    snprintf(sNumber, sizeof(sNumber), "%d", Channel->Number());
        osd->DrawText(x1ChannelInfo + 120, y1ChannelInfo + 13, sNumber, Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
		osd->DrawText(x1ChannelInfo + 240, y1ChannelInfo + 5, Channel->Name(), Theme.Color(clrFontColor), clrTransparent, fontSansBook37, x2ChannelInfo - (x1ChannelInfo + 240), 0, taLeft);
	  break;
	  
	  case 1 :
        snprintf(sNumber, sizeof(sNumber), "%d - %s", Channel->Number(), Channel->Name());
        osd->DrawText(x1ChannelInfo + 240, y1ChannelInfo + 5, sNumber, Theme.Color(clrFontColor), clrTransparent, fontSansBook37, x2ChannelInfo - (x1ChannelInfo + 240), 0, taLeft);
	  break;
	}
	
	int xSymbol = xSymbolStart;

	//Teletext
	osd->DrawRectangle(xSymbol+1, ySymbol+1, xSymbol+35, ySymbol+26, Theme.Color(Channel->Tpid() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+2, ySymbol+2, xSymbol+34, ySymbol+25, Theme.Color(clrMainLight));
	osd->DrawRectangle(xSymbol+4, ySymbol+5, xSymbol+32, ySymbol+7, Theme.Color(Channel->Tpid() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+4, ySymbol+10, xSymbol+32, ySymbol+12, Theme.Color(Channel->Tpid() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+4, ySymbol+15, xSymbol+32, ySymbol+17, Theme.Color(Channel->Tpid() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+4, ySymbol+20, xSymbol+32, ySymbol+22, Theme.Color(Channel->Tpid() ? clrSymbolGreen : clrDivider));
	
	//DolbyDigital
	xSymbol += symbolWidth;
	osd->DrawRectangle(xSymbol+1, ySymbol+1, xSymbol+35, ySymbol+26, Theme.Color(Channel->Dpid(0) ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+2, ySymbol+2, xSymbol+34, ySymbol+25, Theme.Color(clrMainLight));
	osd->DrawRectangle(xSymbol+18, ySymbol+2, xSymbol+18, ySymbol+26, Theme.Color(Channel->Dpid(0) ? clrSymbolGreen : clrDivider));
	osd->DrawEllipse(xSymbol+4, ySymbol+4, xSymbol+16, ySymbol+22, Theme.Color(Channel->Dpid(0) ? clrSymbolGreen : clrDivider), 5);
	osd->DrawEllipse(xSymbol+20, ySymbol+4, xSymbol+32, ySymbol+22, Theme.Color(Channel->Dpid(0) ? clrSymbolGreen : clrDivider), 7);
	     
    //MultiLang
	xSymbol += symbolWidth;
	osd->DrawRectangle(xSymbol+1, ySymbol+1, xSymbol+35, ySymbol+26, Theme.Color(Channel->Apid(1) ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+2, ySymbol+2, xSymbol+34, ySymbol+25, Theme.Color(clrMainLight));
	osd->DrawEllipse(xSymbol+5, ySymbol+6, xSymbol+18, ySymbol+22, Theme.Color(Channel->Apid(1) ? clrSymbolGreen : clrDivider));
	osd->DrawEllipse(xSymbol+5, ySymbol+8, xSymbol+18, ySymbol+20, Theme.Color(Channel->Apid(1) ? clrSymbolGreen : clrDivider));
	osd->DrawEllipse(xSymbol+19, ySymbol+6, xSymbol+32, ySymbol+22, Theme.Color(Channel->Apid(1) ? clrSymbolGreen : clrDivider));
	osd->DrawEllipse(xSymbol+19, ySymbol+8, xSymbol+32, ySymbol+20, Theme.Color(Channel->Apid(1) ? clrSymbolGreen : clrMainLight));

	if (Channel->Apid(1))
	{
	  switch (cDevice::PrimaryDevice()->GetAudioChannel())
	  {
	    case 1:
		  osd->DrawEllipse(xSymbol+19, ySymbol+8, xSymbol+32, ySymbol+20, Theme.Color(clrMainLight));
		break;
		
		case 2 :
		  osd->DrawEllipse(xSymbol+5, ySymbol+8, xSymbol+18, ySymbol+20, Theme.Color(clrMainLight));
		break;
	  }
	}
	
	//Encrypted
	xSymbol += symbolWidth;
	osd->DrawRectangle(xSymbol+1, ySymbol+1, xSymbol+35, ySymbol+26, Theme.Color(Channel->Ca() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+2, ySymbol+2, xSymbol+34, ySymbol+25, Theme.Color(clrMainLight));
	osd->DrawEllipse(xSymbol+19, ySymbol+9, xSymbol+32, ySymbol+23, Theme.Color(Channel->Ca() ? clrSymbolGreen : clrDivider));
	osd->DrawEllipse(xSymbol+22, ySymbol+12, xSymbol+29, ySymbol+20, Theme.Color(clrMainLight));
	osd->DrawRectangle(xSymbol+4, ySymbol+9, xSymbol+22, ySymbol+11, Theme.Color(Channel->Ca() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+6, ySymbol+11, xSymbol+8, ySymbol+15, Theme.Color(Channel->Ca() ? clrSymbolGreen : clrDivider));
	osd->DrawRectangle(xSymbol+10, ySymbol+11, xSymbol+12, ySymbol+15, Theme.Color(Channel->Ca() ? clrSymbolGreen : clrDivider));

    //Recording
	xSymbol += symbolWidth;
	osd->DrawRectangle(xSymbol+1, ySymbol+1, xSymbol+35, ySymbol+26, Theme.Color(cRecordControls::Active() ? clrSymbolRed : clrDivider));
	osd->DrawRectangle(xSymbol+2, ySymbol+2, xSymbol+34, ySymbol+25, Theme.Color(clrMainLight));
    osd->DrawText(xSymbol+4, ySymbol+3, "rec", Theme.Color(cRecordControls::Active() ? clrSymbolRed : clrDivider), clrTransparent, fontSansBold16);

	//CryptSymbols
	if (PearlHDConfig.CryptSymbols)
	{
	  #define NUM_CA_IDS 9
	  bool caOn[NUM_CA_IDS];
	  for (int i=0;i<NUM_CA_IDS;i++)
	    caOn[i] = false;
	  const char *caShort[NUM_CA_IDS] = {"B", "I", "NDS", "N", "S", "V", "C", "CW", "BC" };
	  int xCa = x2ChannelInfo-605;
	  int widthCa = 40;

	  for (int i=0; Channel->Ca(i) != 0 ; i++)
	  {
	    switch (Channel->Ca(i))
		{
		  case 0x2600 : //Biss
		    caOn[0] = true;
		  break;
	      
		  case 0x0600 ... 0x06FF : //Irdeto
		    caOn[1] = true;
		  break;
		  
		  case 0x0900 ... 0x09FF : //NDS
		    caOn[2] = true;
		  break;
		  
		  case 0x1200 ... 0x12FF : //Nagra
		  case 0x1800 ... 0x18FF : //Nagra
		    caOn[3] = true;
		  break;
		  
		  case 0x0100 ... 0x01FF : //Seca
		    caOn[4] = true;
		  break;
		  
		  case 0x0500 ... 0x05FF : //Viaccess
		    caOn[5] = true;
		  break;
		  
		  case 0x0B00 ... 0x0BFF : //Conax
		    caOn[6] = true;
		  break;
		  
		  case 0x0D00 ... 0x0DFF : //Cryptoworks
		    caOn[7] = true;
		  break;
		  
		  case 0x1700 ... 0x17FF : //Betacrypt
		    caOn[8] = true;
		  break;
		}
	  }
	  
	  for (int i=0;i<NUM_CA_IDS;i++)
	  {
	    DrawCryptSymbol(xCa, ySymbol+1, caShort[i], Theme.Color(caOn[i] ? clrSymbolGreen : clrDivider), Theme.Color(clrMainLight));
		xCa += widthCa;
	  }
	}
  }
  else
  {
    isGroupSep=true;
    osd->DrawText(x1ChannelInfo + 240, y1ChannelInfo + 5, Channel->Name(), Theme.Color(clrFontColor), clrTransparent, fontSansBook37);
  }
}

void cSkinPearlHDDisplayChannel::SetEvents(const cEvent *Present, const cEvent *Following)
{
  for (int i = 0; i < 2; i++) {
      const cEvent *e = !i ? Present : Following;
      if (e) {
	     osd->DrawText(x1ChannelInfo + 120, y1ChannelInfo + 98 + (i * 95), e->GetTimeString(), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 120);

		 char title[100];
		 if (e->HasTimer())
		 {
		   snprintf (title, sizeof(title), "%s *", e->Title());
		 }
		 else
		 {
		   snprintf (title, sizeof(title), "%s", e->Title());
		 }
		 osd->DrawText(x1ChannelInfo + 240, y1ChannelInfo + 96 + (i * 95), title, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook37, (x2ChannelInfo -125) - (x1ChannelInfo + 240));
		 osd->DrawText(x1ChannelInfo + 240, y1ChannelInfo + 141 + (i * 95), e->ShortText(), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, (x2ChannelInfo -125) - (x1ChannelInfo + 240));
		 
		 if (i == 0)
		 {
		   osd->DrawRectangle(x1ChannelInfo, y1ChannelInfo + 50, x2ChannelInfo, y1ChannelInfo + 70, Theme.Color(clrBar));
		   int xBarLength = (int)(ceil((float)((time(NULL) - e->StartTime())) / (float)(e->Duration()) * (float)(x2ChannelInfo - x1ChannelInfo)));
		   osd->DrawRectangle(x1ChannelInfo, y1ChannelInfo + 50, x1ChannelInfo + xBarLength, y1ChannelInfo + 70, Theme.Color(clrBarFill));
		   int iRemaining = (e->Duration() - (time(NULL) - e->StartTime())) / 60;
		   char sRemaining[12];
		   snprintf(sRemaining, sizeof(sRemaining), "%d", iRemaining);
		   if (PearlHDConfig.ChannelLogoPos != 2)
		   {
			 osd->DrawText(x2ChannelInfo - 120, y1ChannelInfo + 95, "m", Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook15, 95, 0, taRight);
             osd->DrawText(x2ChannelInfo - 120, y1ChannelInfo + 95, sRemaining, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 75, 0, taRight);
		   }
		 }
		 else if (PearlHDConfig.ChannelLogoPos != 2)
		 {
		   char sNextDuration[12];
           snprintf(sNextDuration, sizeof(sNextDuration), "%d", e->Duration() / 60);
		   osd->DrawText(x2ChannelInfo - 120, y1ChannelInfo + 190, "m", Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook15, 95, 0, taRight);
           osd->DrawText(x2ChannelInfo - 120, y1ChannelInfo + 190, sNextDuration, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 75, 0, taRight);			
		 }
     }
  }
}

void cSkinPearlHDDisplayChannel::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
    osd->DrawRectangle(x1Message + 50, y1Message, x2Message, y1Message + 50, Theme.Color(clrMainLight));
	osd->DrawEllipse(x1Message, y1Message, x1Message + 50, y1Message + 50, Theme.Color(clrMainLight), 2);
	switch(Type) {
	  case mtInfo :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrMainSolid));
	    break;
	  case mtStatus :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrStatus));
	    break;
	  case mtWarning :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrWarning));
	    break;
	  case mtError :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrError));
	    break;
	}
	osd->DrawText(x1Message + 50, y1Message + 15, Text, Theme.Color(clrFontColor), clrTransparent, fontSansBook27, x2Message - (x1Message+50), 0, taRight);			
  }
}

void cSkinPearlHDDisplayChannel::Flush(void)
{
  char datetime[50];
  struct tm tm_r;
  time_t t;
  time(&t);
  tm *tm = localtime_r(&t, &tm_r);
  snprintf(datetime, sizeof(datetime), "%s %02d.%02d. %02d:%02d", *WeekDayNameFull(t), tm->tm_mday, tm->tm_mon + 1, tm->tm_hour, tm->tm_min);
  if (strcmp(lastdatetime, datetime) != 0)
  {
    osd->DrawRectangle(x1DateTime, y1DateTime, x2DateTime, y1DateTime + 20, Theme.Color(clrMainLight));
    osd->DrawEllipse(x1DateTime, y1DateTime + 20, x1DateTime + 50, y1DateTime + 70, Theme.Color(clrMainLight), 3);
    osd->DrawRectangle(x1DateTime + 50, y1DateTime + 20, x2DateTime, y1DateTime + 70, Theme.Color(clrMainLight));
    osd->DrawText(x1DateTime + 30, y1DateTime + 20, datetime, Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
	strcpy(lastdatetime, datetime);
  }
  
  cPlugin *p = cPluginManager::GetPlugin("epgsearch");
  if (!isGroupSep && p)
  {
    Epgsearch_lastconflictinfo_v1_0 *serviceData = new Epgsearch_lastconflictinfo_v1_0;
    if (serviceData)
	{
      serviceData->nextConflict = 0;
      serviceData->relevantConflicts = 0;
      serviceData->totalConflicts = 0;

      p->Service("Epgsearch-lastconflictinfo-v1.0", serviceData);
      if (serviceData->totalConflicts > 0)
	  {
	    int delay = cRecordControls::Active() ? 800 : 400;
        if (cTimeMs::Now() > blinkNext)
        {
          osd->DrawText(xSymbolStart+200+4, ySymbol+3, "rec", Theme.Color(blinkState ? clrSymbolRed : clrDivider), Theme.Color(clrMainLight), fontSansBold16);
	      blinkNext = cTimeMs::Now() + delay;
          blinkState = blinkState ^ 1;
        }
	  }
	}
  }

  osd->Flush();
}

void cSkinPearlHDDisplayChannel::DrawRedButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+4, 0xb4690000);
  osd->DrawRectangle(xButton, yButton+4, xButton+237, yButton+6, 0xb4740000);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb47c0000);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+11, 0xb4860000);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+14, 0xb4960000);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+16, 0xb4a60000);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb4ac0000);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+19, 0xb4b70000);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb4bd0000);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb4c20000);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+23, 0xb4cd0000);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb4d80000);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb4d80000);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb4d80000);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb4e20000);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb4e20000);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb4ef0000);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb4ef0000);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb4ef0000);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb4f80000);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb4f80000);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb4f80000);
}

void cSkinPearlHDDisplayChannel::DrawGreenButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+2, 0xb4006400);
  osd->DrawRectangle(xButton, yButton+2, xButton+237, yButton+5, 0xb4006d00);
  osd->DrawRectangle(xButton, yButton+5, xButton+237, yButton+6, 0xb4007400);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb4007c00);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+9, 0xb4008100);
  osd->DrawRectangle(xButton, yButton+9, xButton+237, yButton+11, 0xb4008600);
  osd->DrawRectangle(xButton, yButton+10, xButton+237, yButton+11, 0xb4008b00);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+12, 0xb4009000);
  osd->DrawRectangle(xButton, yButton+12, xButton+237, yButton+13, 0xb4009600);
  osd->DrawRectangle(xButton, yButton+13, xButton+237, yButton+14, 0xb4009b00);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+15, 0xb400a100);
  osd->DrawRectangle(xButton, yButton+15, xButton+237, yButton+16, 0xb400a600);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb400ac00);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+18, 0xb400b200);
  osd->DrawRectangle(xButton, yButton+18, xButton+237, yButton+19, 0xb400b700);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb400bd00);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb400c200);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+22, 0xb400c800);
  osd->DrawRectangle(xButton, yButton+22, xButton+237, yButton+23, 0xb400cd00);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb400d300);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb400d800);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb400dd00);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb400e200);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb400e200);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb400ef00);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb400ef00);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb400ef00);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb400f800);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb400f800);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb400f800);
}

void cSkinPearlHDDisplayChannel::DrawYellowButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+3, 0xb4666600);
  osd->DrawRectangle(xButton, yButton+3, xButton+237, yButton+4, 0xb46d6d00);
  osd->DrawRectangle(xButton, yButton+4, xButton+237, yButton+6, 0xb4707000);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb47c7c00);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+10, 0xb4868600);
  osd->DrawRectangle(xButton, yButton+10, xButton+237, yButton+11, 0xb48b8b00);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+12, 0xb4909000);
  osd->DrawRectangle(xButton, yButton+12, xButton+237, yButton+14, 0xb49b9b00);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+16, 0xb4a6a600);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb4acac00);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+19, 0xb4b7b700);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb4bdbd00);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb4c2c200);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+23, 0xb4cdcd00);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb4d3d300);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb4d8d800);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb4dddd00);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb4e2e200);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb4e2e200);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb4efef00);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb4efef00);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb4efef00);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb4f8f800);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb4f8f800);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb4f8f800);
}

void cSkinPearlHDDisplayChannel::DrawBlueButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+4, 0xb4000068);
  osd->DrawRectangle(xButton, yButton+4, xButton+237, yButton+6, 0xb4000072);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb400007a);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+11, 0xb4000086);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+14, 0xb4000096);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+16, 0xb40000a4);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb40000ac);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+19, 0xb40000b5);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb40000bd);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb40000c2);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+23, 0xb40000cb);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb40000d8);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb40000d8);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb40000d8);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb40000e4);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb40000e4);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb40000ee);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb40000ee);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb40000ee);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb40000f8);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb40000f8);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb40000f8);
}

void cSkinPearlHDDisplayChannel::DrawCryptSymbol(int xSymbol, int ySymbol, const char *Text, tColor color, tColor bgcolor)
{
  osd->DrawRectangle (xSymbol, ySymbol, xSymbol+25, ySymbol+20, color);
  osd->DrawRectangle (xSymbol+1, ySymbol+1, xSymbol+24, ySymbol+19, bgcolor);
  osd->DrawText (xSymbol+2, ySymbol+5, Text, color, clrTransparent, fontCryptSymbols, 22, 0, taCenter);
}

// --- cSkinPearlHDDisplayMenu -------------------------------------------------

class cSkinPearlHDDisplayMenu : public cSkinDisplayMenu {
private:
  cOsd *osd;
  int x1Menu, x2Menu, y1Menu, y2Menu;
  int x1Scrollbar, x2Scrollbar, y1Scrollbar, y2Scrollbar;
  int rightSmall, rightBig;
  int lineHeight;
  tColor frameColor;
  int currentIndex;
  bool message;
  cString lastDate;
  void DrawScrollbar(int Total, int Offset, int Shown, int Top, int Height, bool CanScrollUp, bool CanScrollDown);
  void SetTextScrollbar(void);
  const cFont *fontSansBook15;
  const cFont *fontSansBook20;
  const cFont *fontSansBook22;
  const cFont *fontSansBook27;
  const cFont *fontSansBook37;
  const cFont *fontSansBook57;
  const cFont *fontSansBold27;
  const cFont *fontSansBold37;
  int menuType;
  int bpp;
  unsigned long blinkNext;
  int blinkState;
  #if VDRVERSNUM > 10716
  cPixmap *epgPixmap;
  #endif
  const cBitmap *bRecTitleHead;
  const char *MenuTitleClean(std::string Title);
  const char *MenuTitleRest(std::string Title);
  void DrawHeader();
  void DrawRedButton(int xButton, int yButton);
  void DrawGreenButton(int xButton, int yButton);
  void DrawYellowButton(int xButton, int yButton);
  void DrawBlueButton(int xButton, int yButton);
  int GetRecordingLength(const char *FileName, double FramesPerSecond, bool IsPesRecording);
  int GetRecordingCuttedLength(const char *FileName, double FramesPerSecond, bool IsPesRecording);
  bool IsTextProgressbar(const char *Text);
public:
  cSkinPearlHDDisplayMenu(void);
  virtual ~cSkinPearlHDDisplayMenu();
  virtual void Scroll(bool Up, bool Page);
  virtual int MaxItems(void);
  virtual void Clear(void);
  virtual void SetTitle(const char *Title);
  virtual void SetButtons(const char *Red, const char *Green = NULL, const char *Yellow = NULL, const char *Blue = NULL);
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);
  virtual void SetEvent(const cEvent *Event);
  virtual void SetRecording(const cRecording *Recording);
  virtual void SetText(const char *Text, bool FixedFont);
  virtual int GetTextAreaWidth(void) const;
  virtual const cFont *GetTextAreaFont(bool FixedFont) const;
  virtual void Flush(void);
  };

cSkinPearlHDDisplayMenu::cSkinPearlHDDisplayMenu(void)
{
  bpp=0;
  lineHeight = PearlHDConfig.LineHeight;
  currentIndex = -1;
  message = false;
  x1Menu = PearlHDConfig.MenuPaddingWidth;
  x2Menu = cOsd::OsdWidth() - PearlHDConfig.MenuPaddingWidth;
  y1Menu = PearlHDConfig.MenuPaddingHeight;
  y2Menu = cOsd::OsdHeight() - PearlHDConfig.MenuPaddingHeight - 34;
  fontSansBook15 = cFont::CreateFont("VDRSymbols Sans:Book", 15);
  fontSansBook20 = cFont::CreateFont("VDRSymbols Sans:Book", 20);
  fontSansBook22 = cFont::CreateFont("VDRSymbols Sans:Book", 22);
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);
  fontSansBook37 = cFont::CreateFont("VDRSymbols Sans:Book", 37);
  fontSansBook57 = cFont::CreateFont("VDRSymbols Sans:Book", 57);
  fontSansBold27 = cFont::CreateFont("VDRSymbols Sans:Bold", 27);
  fontSansBold37 = cFont::CreateFont("VDRSymbols Sans:Bold", 37);
  menuType = 0;
  blinkNext=0;
  blinkState=0;

  osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop());
  tArea Areas[] = { { 0, 0, cOsd::OsdWidth() - 1, cOsd::OsdHeight() - 1, 32 } };
  if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
  {
    bpp=24;
    osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
  }
  else
  {
    tArea Areas[] = { { 0, 0, cOsd::OsdWidth() - 1, y2Menu, 8 },  
	  { x2Menu - 957, y2Menu + 1, x2Menu - 478, cOsd::OsdHeight() - 1, 8 }, 
      { x2Menu - 477, y2Menu + 1, x2Menu - 1, cOsd::OsdHeight() - 1, 8 } }; // 256 colors
    if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
	{
	  bpp=8;
      osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
	}
  }
  
  if ((SysLogLevel == 3) && (bpp == 24))
    printf ("skinpearlhd: using truecolor mode\n");
  if ((SysLogLevel == 3) && (bpp == 8))
    printf ("skinpearlhd: using 256 color mode\n");
	
  osd->DrawRectangle(1,1,1,1, clrTransparent);
  osd->DrawRectangle(x2Menu-957, y2Menu+1, x2Menu-957, y2Menu+1, clrTransparent);
  osd->DrawRectangle(x2Menu-477, y2Menu+1, x2Menu-477, y2Menu+1, clrTransparent);
}

cSkinPearlHDDisplayMenu::~cSkinPearlHDDisplayMenu()
{
  delete osd;
}

void cSkinPearlHDDisplayMenu::DrawScrollbar(int Total, int Offset, int Shown, int Top, int Height, bool CanScrollUp, bool CanScrollDown)
{
  if (Total > 0 && Total > Shown) {
	 int th = int((y2Scrollbar - y1Scrollbar) * double(Shown) / Total + 0.5);
     int tt = int(y1Scrollbar + (y2Scrollbar - y1Scrollbar) * double(Offset) / Total + 0.5);
     int tb = tt + th;
	 osd->DrawRectangle(x1Scrollbar, y1Scrollbar, x2Scrollbar, y2Scrollbar, Theme.Color(clrBar));
	 osd->DrawRectangle(x1Scrollbar, tt, x2Scrollbar, tb - 1, Theme.Color(clrBarFill));
  }
}

void cSkinPearlHDDisplayMenu::SetTextScrollbar(void)
{
  if (textScroller.CanScroll())
  {
    DrawScrollbar(textScroller.Total(), textScroller.Offset(), textScroller.Shown(), textScroller.Top(), textScroller.Height(), textScroller.CanScrollUp(), textScroller.CanScrollDown());
  }
}

void cSkinPearlHDDisplayMenu::Scroll(bool Up, bool Page)
{
  cSkinDisplayMenu::Scroll(Up, Page);
  SetTextScrollbar();
}

int cSkinPearlHDDisplayMenu::MaxItems(void)
{
  return ( ((y2Menu-84) - (y1Menu+175)) / lineHeight);
}

void cSkinPearlHDDisplayMenu::Clear(void)
{
  #if VDRVERSNUM > 10716
  osd->DestroyPixmap(epgPixmap);
  #endif
  textScroller.Reset();
}

void cSkinPearlHDDisplayMenu::SetTitle(const char *Title)
{
/*  if (strcmp(trVDR("VDR"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_VDR;
  else if (strcmp(trVDR("Commands"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_VDR;
  else if (strcmp(trVDR("Schedule"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_SCHEDULE;
  else if (strcmp(trVDR("Channels"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_SCHEDULE;
  else if (strcmp(trVDR("Setup"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_SCHEDULE;
  else if (strcmp(trVDR("Edit timer"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_SCHEDULE;
  else if (strcmp(trVDR("Recordings"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_RECORDINGS;
  else if (strcmp(trVDR("Timers"), MenuTitleClean(Title)) == 0)
    menuType = MENU_TYPE_RECORDINGS;
  else
    menuType = -1;
*/
  DrawHeader();
  
//#if VDRVERSNUM > 10727
  switch (MenuCategory())
  {
  case mcMain :
  case mcCommand :
    //Sub Menu VDR, Commands
    osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu-480, y2Menu, Theme.Color(clrMainLight));
  
    // Background right
    osd->DrawRectangle(x2Menu-479, y1Menu+117, x2Menu, y2Menu, Theme.Color(clrMainSolid));
  	osd->DrawText(x2Menu-479, y2Menu-55, *cVideoDiskUsage::String(), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 449, 0, taRight);
  break;
//mcUndefined = -1, mcUnknown = 0, mcMain, mcSchedule, mcChannel, mcTimer, mcRecording, mcPlugin, mcSetup, mcCommand, mcEvent, mcText, mcFolder, mcCam };

  case mcSchedule :
  case mcChannel :
  case mcSetup :
  case mcTimer :
  case mcEvent :
  // Sub Menu Schedule, Channels, Setup, Edit timer
    osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu-360, y2Menu, Theme.Color(clrMainLight));
	
	//Background Right
	osd->DrawRectangle(x2Menu-360, y1Menu+117, x2Menu, y2Menu, Theme.Color(clrMainSolid));
    osd->DrawText(x2Menu-360, y2Menu-55, MenuTitleRest(Title), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 330, 0, taRight);
	osd->DrawText(x1Menu+75, y1Menu+60, Title, Theme.Color(clrFontColor), clrTransparent, fontSansBook37);
  break;
  
  case mcUndefined :
  case mcUnknown :
  case mcRecording :
  case mcPlugin :
  case mcText :
  case mcFolder :
  case mcCam :
  //default :
  //Sub Menu Default
    osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu, y2Menu, Theme.Color(clrMainLight));
	osd->DrawText(x1Menu+75, y1Menu+60, MenuTitleClean(Title), Theme.Color(clrFontColor), clrTransparent, fontSansBook37);
  break;
  }
/*#else
  switch (menuType)
  {
  case MENU_TYPE_VDR :
    //Sub Menu VDR, Commands
    osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu-480, y2Menu, Theme.Color(clrMainLight));
  
    // Background right
    osd->DrawRectangle(x2Menu-479, y1Menu+117, x2Menu, y2Menu, Theme.Color(clrMainSolid));
  	osd->DrawText(x2Menu-479, y2Menu-55, MenuTitleRest(Title), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 449, 0, taRight);
  break;

  case MENU_TYPE_SCHEDULE :
  // Sub Menu Schedule, Channels, Setup, Edit timer
    osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu-360, y2Menu, Theme.Color(clrMainLight));
	
	//Background Right
	osd->DrawRectangle(x2Menu-360, y1Menu+117, x2Menu, y2Menu, Theme.Color(clrMainSolid));
    osd->DrawText(x2Menu-360, y2Menu-55, MenuTitleRest(Title), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 330, 0, taRight);
	osd->DrawText(x1Menu+75, y1Menu+60, Title, Theme.Color(clrFontColor), clrTransparent, fontSansBook37);
  break;
   
  default :
  //Sub Menu Default
    osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu, y2Menu, Theme.Color(clrMainLight));
	osd->DrawText(x1Menu+75, y1Menu+60, MenuTitleClean(Title), Theme.Color(clrFontColor), clrTransparent, fontSansBook37);
  break;
  }
#endif*/
}

void cSkinPearlHDDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue)
{
  if (Red)
  {
    DrawRedButton(x2Menu-957, y2Menu);
	osd->DrawText(x2Menu-957, y2Menu+1, Red, Theme.Color(clrMessageWhite), clrTransparent, fontSansBook27, 239, 0, taCenter);
  }
  else
    osd->DrawRectangle(x2Menu-957, y2Menu, x2Menu-718, y2Menu+35, clrTransparent);
  
  if (Green)
  {
    DrawGreenButton(x2Menu-717, y2Menu);
	osd->DrawText(x2Menu-717, y2Menu+1, Green, Theme.Color(clrMessageBlack), clrTransparent, fontSansBook27, 237, 0, taCenter);
  }
  else
    osd->DrawRectangle(x2Menu-717, y2Menu, x2Menu-478, y2Menu+35, clrTransparent);
  
  if (Yellow)
  {
    DrawYellowButton(x2Menu-477, y2Menu);
	osd->DrawText(x2Menu-477, y2Menu+1, Yellow, Theme.Color(clrMessageBlack), clrTransparent, fontSansBook27, 241, 0, taCenter);
  }
  else
    osd->DrawRectangle(x2Menu-477, y2Menu, x2Menu-238, y2Menu+35, clrTransparent);
  
  if (Blue)
  {
    DrawBlueButton(x2Menu-237, y2Menu);
	osd->DrawText(x2Menu-237, y2Menu+1, Blue, Theme.Color(clrMessageWhite), clrTransparent, fontSansBook27, 237, 0, taCenter);
  }
  else
    osd->DrawRectangle(x2Menu-237, y2Menu, x2Menu, y2Menu+35, clrTransparent);
}

void cSkinPearlHDDisplayMenu::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
    tColor color;
    int MessageX2;
    switch (Type) {
	  case mtInfo :
	    color = Theme.Color(clrMainSolid);
	    break;
	  case mtStatus :
	    color = Theme.Color(clrStatus);
	    break;
	  case mtWarning :
	    color = Theme.Color(clrWarning);
	    break;
	  case mtError :
	    color = Theme.Color(clrError);
	    break;
	}
/*	switch (menuType) {
	 case MENU_TYPE_VDR :
	   MessageX2 = x2Menu-479;
	   break;
	 case MENU_TYPE_SCHEDULE :
	   MessageX2 = x2Menu-360;
	   break;
	 default :
	   MessageX2 = x2Menu;
	}*/
	switch (MenuCategory()) {
      case mcMain :
      case mcCommand :
	     MessageX2 = x2Menu-479;
	     break;
      
	  case mcSchedule :
      case mcChannel :
      case mcSetup :
      case mcTimer :
      case mcEvent :
	    MessageX2 = x2Menu-360;
	    break;

      case mcUndefined :
      case mcUnknown :
      case mcRecording :
      case mcPlugin :
      case mcText :
      case mcFolder :
      case mcCam :
	    MessageX2 = x2Menu;
	}
	osd->SaveRegion(x1Menu, y1Menu+117, MessageX2, y1Menu+158);
    osd->DrawRectangle(x1Menu, y1Menu+117, MessageX2, y1Menu+158, color);
	osd->DrawText(x1Menu+75, y1Menu+122, Text, Theme.Color(clrMessageWhite), clrTransparent, fontSansBook27);
  }
  else {
     osd->RestoreRegion();
     message = false;
  }
}

void cSkinPearlHDDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable)
{
  int x1Item;
  int y1Item;
  int x2Item;
  int y2Item;
  switch (MenuCategory())
  {
  case mcMain :
  case mcCommand :
    x1Item = x1Menu;
    y1Item = y1Menu+175 + (lineHeight * Index);
    x2Item = x2Menu-480;
    y2Item = y1Item + lineHeight;
	break;

  case mcSchedule :
  case mcChannel :
  case mcSetup :
  case mcTimer :
  case mcEvent :
    x1Item = x1Menu;
    y1Item = y1Menu+175 + (lineHeight * Index);
    x2Item = x2Menu-360;
    y2Item = y1Item + lineHeight;
	break;

  case mcUndefined :
  case mcUnknown :
  case mcRecording :
  case mcPlugin :
  case mcText :
  case mcFolder :
  case mcCam :
    x1Item = x1Menu;
    y1Item = y1Menu+175 + (lineHeight * Index);
    x2Item = x2Menu;
    y2Item = y1Item + lineHeight;
  }
  /*switch (menuType)
  {
  case MENU_TYPE_VDR :
    x1Item = x1Menu;
    y1Item = y1Menu+175 + (lineHeight * Index);
    x2Item = x2Menu-480;
    y2Item = y1Item + lineHeight;
	break;
  case MENU_TYPE_SCHEDULE :
    x1Item = x1Menu;
    y1Item = y1Menu+175 + (lineHeight * Index);
    x2Item = x2Menu-360;
    y2Item = y1Item + lineHeight;
	break;
  default : 
    x1Item = x1Menu;
    y1Item = y1Menu+175 + (lineHeight * Index);
    x2Item = x2Menu;
    y2Item = y1Item + lineHeight;
  }*/
  if (Current) {
    osd->DrawRectangle(x1Item+74, y1Item, x2Item-1, y2Item-6, Theme.Color(clrMainSolid));
    osd->DrawEllipse(x1Item+60, y1Item, x1Item+74, y1Item+15, Theme.Color(clrMainSolid), 2);
	osd->DrawRectangle(x1Item+60, y1Item+14, x1Item+74, y2Item-21, Theme.Color(clrMainSolid));
	osd->DrawEllipse(x1Item+60, y2Item-21, x1Item+74, y2Item-6, Theme.Color(clrMainSolid), 3);
  }
  else {
    osd->DrawRectangle(x1Item+74, y1Item, x2Item-1, y2Item-6, Theme.Color(clrLoLight));
    osd->DrawEllipse(x1Item+60, y1Item, x1Item+74, y1Item+15, Theme.Color(clrLoLight), 2);
	osd->DrawRectangle(x1Item+60, y1Item+14, x1Item+74, y2Item-21, Theme.Color(clrLoLight));
	osd->DrawEllipse(x1Item+60, y2Item-21, x1Item+74, y2Item-6, Theme.Color(clrLoLight), 3);
  }
  
  for (int i = 0; i < MaxTabs; i++)
  {
    const char *s = GetTabbedText(Text, i);
    if (s)
    {
      int xt = x1Item+75 + Tab(i);
	  if (!IsTextProgressbar(s))
        osd->DrawText(xt, y1Item+4, s, Theme.Color(Current ? clrFontColor : clrFontColorInactive), Theme.Color(Current ? clrMainSolid : clrLoLight), fontSansBook27, x2Item-xt);
      else
      {
	    std::string text = s;
		int pgwidth = fontSansBook27->Width(s);
	    int total = text.length() - 2;
        int current = 0;
        const char *p = text.c_str() + 1;
        while (*p == '|')
          (++current, ++p);
		osd->DrawRectangle(xt, y1Item+4, xt+pgwidth, y1Item+27, Theme.Color(Current ? clrFontColor : clrFontColorInactive));
		osd->DrawRectangle(xt+2, y1Item+6, xt+pgwidth-2, y1Item+27-2, Theme.Color(Current ? clrMainSolid : clrLoLight));
		osd->DrawRectangle(xt+4, y1Item+8, xt+int(double(pgwidth)*(double(current)/double(total))), y1Item+27-4, Theme.Color(Current ? clrFontColor : clrFontColorInactive));
      }
    }
    if (!Tab(i + 1))
      break;
  }
  SetEditableWidth(Tab(1));
}

void cSkinPearlHDDisplayMenu::SetEvent(const cEvent *Event)
{
  if (!Event)
     return;
	
  //menuType = MENU_TYPE_EVENT;
  
  DrawHeader();
  
  osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu-360, y2Menu, Theme.Color(clrMainLight));
  osd->DrawRectangle(x2Menu-360, y1Menu+117, x2Menu-1, y2Menu, Theme.Color(clrMainSolid));
  
  if(PearlHDConfig.DisplayEpgImages)
  {
    char logoFormat[4];
    switch (PearlHDConfig.EpgImagesFormat)
    {
      case 0:
	    snprintf(logoFormat, sizeof(logoFormat), "png");
	  break;
	  case 1:
	    snprintf(logoFormat, sizeof(logoFormat), "jpg");
	  break;
    }
	
    cOSDImageBitmap osdbitmap;
	  
    std::stringstream epgPath;
    if (PearlHDConfig.EpgDirSet)
      epgPath << PearlHDConfig.EpgImagesDir;
	else
	{
	  epgPath << cPlugin::ConfigDirectory() << "/tvm2vdr/epgimages";
	}
	epgPath << "/" << Event->EventID() << "." << logoFormat;

	#if VDRVERSNUM > 10716
	if (bpp > 8)
	{
	  if(osdbitmap.Load(epgPath.str().c_str(), 300, 225)){
        cImage epgImg = osdbitmap.GetImage();
	    epgPixmap = osd->CreatePixmap(0, cRect(x2Menu-330, y2Menu-285, epgImg.Width(), epgImg.Height()));
	    epgPixmap->DrawImage(cPoint(0, 0), epgImg);
      }
	}
	else
	#endif
	{
	  cBitmap epgImg (300, 225, bpp);
	  if(osdbitmap.Load(epgImg, epgPath.str().c_str(), 300, 225, bpp)){
	    osd->DrawBitmap(x2Menu-330, y2Menu-285, epgImg);
      }
	}
  }
  
  osd->DrawText(x1Menu+75, y1Menu+60, Event->Title(), Theme.Color(clrFontColor), clrTransparent, fontSansBook37);
  
  char text[32];
  snprintf(text, sizeof(text), "%s  %s - %s", *Event->GetDateString(), *Event->GetTimeString(), *Event->GetEndTimeString());
  osd->DrawText(x1Menu+75, y1Menu+310, text, Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
  
  if (Event->Vps())
  {
    snprintf(text, sizeof(text), "%d min | VPS: %s", Event->Duration() / 60, *Event->GetVpsString());
	osd->DrawText(x1Menu+75, y1Menu+215, text, Theme.Color(clrFontColor), clrTransparent, fontSansBook20);
  }
  else
  {
    snprintf(text, sizeof(text), "%d min", Event->Duration() / 60);
	osd->DrawText(x1Menu+75, y1Menu+235, text, Theme.Color(clrFontColor), clrTransparent, fontSansBook20);
  }

  //Scrollbar position
  x1Scrollbar = x2Menu-380;
  x2Scrollbar = x2Menu - 360;
  y1Scrollbar = y1Menu + 117;
  y2Scrollbar = y2Menu - 1;

  if (isempty(Event->Description()))
  {
	textScroller.Set (osd, x1Menu+75, y1Menu+170, x2Menu-510, y2Menu-1, Event->ShortText(), fontSansBold27, Theme.Color(clrFontColor), Theme.Color(clrMainLight));
  }
  else
  {
    osd->DrawText(x1Menu+75, y1Menu+270, Event->ShortText(), Theme.Color(clrFontColor), clrTransparent, fontSansBold27);
	textScroller.Set (osd, x1Menu+75, y1Menu+347, (x2Menu-360-150) - (x1Menu+75), (y2Menu - 41) - (y1Menu + 347), Event->Description(), fontSansBook27, Theme.Color(clrFontColor), Theme.Color(clrMainLight));
  }
  SetTextScrollbar();
}

void cSkinPearlHDDisplayMenu::SetRecording(const cRecording *Recording)
{
  if (!Recording)
     return;

  //menuType = MENU_TYPE_RECORDING;
  DrawHeader();

  const cRecordingInfo *Info = Recording->Info();
  osd->DrawRectangle(x1Menu, y1Menu+117, x2Menu-360, y2Menu, Theme.Color(clrMainLight));
  
  // Removed for now, because Recording list and RecordingInfo have the same MenuID
  //osd->DrawRectangle(x2Menu-360, y1Menu+117, x2Menu-1, y2Menu, Theme.Color(clrMainSolid));
  
  osd->DrawText(x1Menu+75, y1Menu+60, Info->Title(), Theme.Color(clrFontColor), clrTransparent, fontSansBold37);
  
  char text[32];
  snprintf(text, sizeof(text), "%s  %s", *DateString(Recording->Start()), *TimeString(Recording->Start()));
  osd->DrawText(x1Menu+75, y1Menu+175, text, Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
  
  int RecordingLength, RecordingCuttedLength;
  #if VDRVERSNUM >= 10703
    RecordingLength = GetRecordingLength(Recording->FileName(), Recording->FramesPerSecond(), Recording->IsPesRecording());
	RecordingCuttedLength = GetRecordingCuttedLength(Recording->FileName(), Recording->FramesPerSecond(), Recording->IsPesRecording());
  #else
    RecordingLength = GetRecordingLength(Recording->FileName(), FRAMESPERSEC, true);
	RecordingCuttedLength = GetRecordingCuttedLength(Recording->FileName(), FRAMESPERSEC, true);
  #endif
  char length[50];
  snprintf(length, sizeof(length), "%d min | %s: %d min", RecordingLength, tr("cutted"), RecordingCuttedLength);
  osd->DrawText(x1Menu+75, y1Menu+215, length, Theme.Color(clrFontColor), clrTransparent, fontSansBook20);
  
 //Scrollbar position
  x1Scrollbar = x2Menu-380;
  x2Scrollbar = x2Menu-360;
  y1Scrollbar = y1Menu+117;
  y2Scrollbar = y2Menu-1;
 
  if (isempty(Info->Description()))
  {
    textScroller.Set (osd, x1Menu+75, y1Menu+270, (x2Menu-510) - (x1Menu+75), (y2Menu-1) - (y1Menu+270), Info->ShortText(), fontSansBook27, Theme.Color(clrFontColor), Theme.Color(clrMainLight));
  }
  
  if (isempty(Info->ShortText()))
    osd->DrawText(x1Menu+75, y1Menu+290, Info->Title(), Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
  
  if (!isempty(Info->Description()))
  {
    osd->DrawText(x1Menu+75, y1Menu+290, Info->ShortText(), Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
	textScroller.Set (osd, x1Menu+75, y1Menu+347, (x2Menu-510) - (x1Menu+75), (y2Menu-41) - (y1Menu+347), Info->Description(), fontSansBook27, Theme.Color(clrFontColor), Theme.Color(clrMainLight));
  }
  
  SetTextScrollbar();
}

void cSkinPearlHDDisplayMenu::SetText(const char *Text, bool FixedFont)
{
  //Scrollbar position
  x1Scrollbar = x2Menu - 20;
  x2Scrollbar = x2Menu;
  y1Scrollbar = y1Menu + 117;
  y2Scrollbar = y2Menu - 1;
  
  textScroller.Set (osd, x1Menu+75, y1Menu+175, (x2Menu-40) - (x1Menu+75), (y2Menu-60) - (y1Menu+175), Text, fontSansBook27, Theme.Color(clrFontColor), Theme.Color(clrMainLight));
  SetTextScrollbar();
}

int cSkinPearlHDDisplayMenu::GetTextAreaWidth(void) const
{
  return (x2Menu-40) - (x1Menu+75);
}

const cFont *cSkinPearlHDDisplayMenu::GetTextAreaFont(bool FixedFont) const
{
  return fontSansBook27;
}

void cSkinPearlHDDisplayMenu::Flush(void)
{
  char datestring[50];
  struct tm tm_r;
  time_t t;
  time(&t);
  tm *tmt = localtime_r(&t, &tm_r);
  strftime(datestring, sizeof(datestring), "%d.%m.%Y", tmt);
  
  int oClockWidth = 0;
  switch (MenuCategory())
  {
    case mcMain :
    case mcCommand :
	  if (PearlHDConfig.oClockText)
	  {
	    oClockWidth = fontSansBook27->Width(tr("oClock")) + 10;
	    osd->DrawText(x2Menu-479, y1Menu+146, tr("oClock"), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 449, 0, taRight);
	  }
	  osd->DrawText(x2Menu-479, y1Menu+136, *TimeString(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook37, 449-oClockWidth, 0, taRight);
      osd->DrawText(x2Menu-479, y1Menu+187, *WeekDayNameFull(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 449, 0, taRight);
      osd->DrawText(x2Menu-479, y1Menu+225, datestring, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook20, 449, 0, taRight);
	break;
	
    case mcSchedule :
    case mcChannel :
    case mcSetup :
    case mcTimer :
    case mcEvent :
	  if (PearlHDConfig.oClockText)
	  {
	    oClockWidth = fontSansBook27->Width(tr("oClock")) + 10;
	    osd->DrawText(x2Menu-360, y1Menu+146, tr("oClock"), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 330, 0, taRight);
	  }
	  osd->DrawText(x2Menu-360, y1Menu+136, *TimeString(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook37, 330-oClockWidth, 0, taRight);
      osd->DrawText(x2Menu-360, y1Menu+187, *WeekDayNameFull(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 330, 0, taRight);
      osd->DrawText(x2Menu-360, y1Menu+225, datestring, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook20, 330, 0, taRight);
	break;

    case mcRecording :
	  if (!PearlHDConfig.RecShowClock)
	    break;
	  if (PearlHDConfig.oClockText)
	  {
	    oClockWidth = fontSansBook27->Width(tr("oClock")) + 10;
	    osd->DrawText(x2Menu-90, y2Menu-55, tr("oClock"), Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 60, 0, taRight);
	  }
	  osd->DrawText(x2Menu-300, y2Menu-55, *TimeString(t), Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 270-oClockWidth, 0, taRight);
    break;

	case mcUndefined :
    case mcUnknown :
    case mcPlugin :
    case mcText :
    case mcFolder :
    case mcCam :
    break;
  }

  /*switch (menuType)
  {
    case MENU_TYPE_VDR :
	  if (PearlHDConfig.oClockText)
	  {
	    oClockWidth = fontSansBook27->Width(tr("oClock")) + 10;
	    osd->DrawText(x2Menu-479, y1Menu+146, tr("oClock"), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 449, 0, taRight);
	  }
	  osd->DrawText(x2Menu-479, y1Menu+136, *TimeString(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook37, 449-oClockWidth, 0, taRight);
      osd->DrawText(x2Menu-479, y1Menu+187, *WeekDayNameFull(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 449, 0, taRight);
      osd->DrawText(x2Menu-479, y1Menu+225, datestring, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook20, 449, 0, taRight);
	break;
	
	case MENU_TYPE_SCHEDULE :
	case MENU_TYPE_RECORDING :
	case MENU_TYPE_EVENT :
	  if (PearlHDConfig.oClockText)
	  {
	    oClockWidth = fontSansBook27->Width(tr("oClock")) + 10;
	    osd->DrawText(x2Menu-360, y1Menu+146, tr("oClock"), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 330, 0, taRight);
	  }
	  osd->DrawText(x2Menu-360, y1Menu+136, *TimeString(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook37, 330-oClockWidth, 0, taRight);
      osd->DrawText(x2Menu-360, y1Menu+187, *WeekDayNameFull(t), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, 330, 0, taRight);
      osd->DrawText(x2Menu-360, y1Menu+225, datestring, Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook20, 330, 0, taRight);
	break;

	case MENU_TYPE_RECORDINGS :
	  if (!PearlHDConfig.RecShowClock)
	    break;
	  if (PearlHDConfig.oClockText)
	  {
	    oClockWidth = fontSansBook27->Width(tr("oClock")) + 10;
	    osd->DrawText(x2Menu-90, y2Menu-55, tr("oClock"), Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 60, 0, taRight);
	  }
	  osd->DrawText(x2Menu-300, y2Menu-55, *TimeString(t), Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 270-oClockWidth, 0, taRight);
    break;
  }*/  
 
  //if (PearlHDConfig.RecTitleInfoHead && menuType == MENU_TYPE_VDR && cRecordControls::Active())
  if (PearlHDConfig.RecTitleInfoHead && MenuCategory() == mcMain && cRecordControls::Active())
  {
  cTimer *t = Timers.First();
  std::string RecTitleHeadString = "";
  Timers.SetEvents();
  bool bFirst = true;
  for (; t != NULL; t = Timers.Next(t))
  {
    if (t->Recording())
	{
	  const cEvent *tevent = t->Event();
	  if (tevent)
	  {
	    if (!bFirst)
		{
		  RecTitleHeadString += " - ";
		}
		else
		  bFirst = false;
	    RecTitleHeadString += tevent->Title();
	  }
	}
  }
      osd->DrawEllipse(x1Menu+95, y1Menu+78, x1Menu+110, y1Menu+93, Theme.Color(clrError));
	  //TODO Marquee
	  osd->DrawText(x1Menu+127, y1Menu+70, RecTitleHeadString.c_str(), Theme.Color(clrFontColor), Theme.Color(clrMainSolid), fontSansBook27, (x2Menu-479) - (x1Menu+127), 0, taLeft);
  }
  
  //if ((PearlHDConfig.RecTitleInfo != 0) && menuType == MENU_TYPE_VDR)
  if ((PearlHDConfig.RecTitleInfo != 0) && MenuCategory() == mcMain)
  {
    int iEventCounter = 0;
	int iNumTimers = 0;
	int x1InfoBottom = x2Menu-479;
	int x2InfoBottom = x2Menu;
	int y1InfoBottom;
	
	if (PearlHDConfig.RecTitleInfo == 2)
	  y1InfoBottom = y1Menu+660;
	else
	  y1InfoBottom = y1Menu+410;
	  
	int iEventHeight = 50;
    cTimer *t = Timers.First();
    Timers.SetEvents();
	
	for (; t != NULL; t = Timers.Next(t))
	  iNumTimers++;
	t = Timers.First();
	
	if ((PearlHDConfig.RecTitleInfo == 2) && iNumTimers < 3)
	  y1InfoBottom += (iEventHeight * iNumTimers);
    
    for (; t != NULL; t = Timers.Next(t))
    {
	  osd->DrawRectangle(x1InfoBottom+14, y1InfoBottom, x2InfoBottom-1, y1InfoBottom+45, Theme.Color(clrMainSolidOverlay));
	  osd->DrawEllipse(x1InfoBottom, y1InfoBottom, x1InfoBottom+14, y1InfoBottom+15, Theme.Color(clrMainSolidOverlay), 2);
	  osd->DrawRectangle(x1InfoBottom, y1InfoBottom+14, x1InfoBottom+14, y1InfoBottom+31, Theme.Color(clrMainSolidOverlay));
	  osd->DrawEllipse(x1InfoBottom, y1InfoBottom+31, x1InfoBottom+14, y1InfoBottom+45, Theme.Color(clrMainSolidOverlay), 3);

	  std::stringstream st;
	  st << (t->Recording() ? t->Stop() : t->Start());
	  std::string timestring;
	  timestring = st.str();
	  timestring.insert(timestring.length()-2, ":");
	  timestring += " / ";
      timestring += t->Channel()->Name();

	  osd->DrawText(x1InfoBottom+20, y1InfoBottom+10, timestring.c_str(), Theme.Color(clrFontColor), Theme.Color(clrMainSolidOverlay), fontSansBook22, (x2InfoBottom-51) - (x1InfoBottom+20), 0, taRight);
	  osd->DrawText(x2InfoBottom-48, y1InfoBottom+6, t->Recording() ? "R" : "T", Theme.Color(t->Recording() ? clrError : clrStatus), Theme.Color(clrMainSolidOverlay), fontSansBook15, 17, 0, taRight);

	  y1InfoBottom += iEventHeight;
	  iEventCounter++;
	  if (iEventCounter == 3)
	    break;
    }
	
	cPlugin *p = cPluginManager::GetPlugin("epgsearch");
    if ((PearlHDConfig.RecTitleInfo == 2) && p)
    {
      Epgsearch_lastconflictinfo_v1_0 *serviceData = new Epgsearch_lastconflictinfo_v1_0;
      if (serviceData)
	  {
        serviceData->nextConflict = 0;
        serviceData->relevantConflicts = 0;
        serviceData->totalConflicts = 0;

        p->Service("Epgsearch-lastconflictinfo-v1.0", serviceData);
        if (serviceData->totalConflicts > 0)
	    {
	      int delay = 2000;
          if (cTimeMs::Now() > blinkNext)
          {
		    char conflict[50];
			snprintf(conflict, sizeof(conflict), "%d %s!", serviceData->totalConflicts, tr("Conflicting Timers"));
			osd->DrawRectangle(x1InfoBottom+14, y1InfoBottom, x2InfoBottom-1, y1InfoBottom+45, Theme.Color(clrMainSolidOverlay));
	        osd->DrawEllipse(x1InfoBottom, y1InfoBottom, x1InfoBottom+14, y1InfoBottom+15, Theme.Color(clrMainSolidOverlay), 2);
            osd->DrawRectangle(x1InfoBottom, y1InfoBottom+14, x1InfoBottom+14, y1InfoBottom+31, Theme.Color(clrMainSolidOverlay));
            osd->DrawEllipse(x1InfoBottom, y1InfoBottom+31, x1InfoBottom+14, y1InfoBottom+45, Theme.Color(clrMainSolidOverlay), 3);
            osd->DrawText(x1InfoBottom+20, y1InfoBottom+6, conflict, Theme.Color(blinkState ? clrError : clrFontColor), Theme.Color(clrMainSolidOverlay), fontSansBook22, (x2InfoBottom-31) - (x1InfoBottom+20), 0, taRight);
	        blinkNext = cTimeMs::Now() + delay;
            blinkState = blinkState ^ 1;
          }
	    }
	  }
    }
  }

  osd->Flush();
}

const char *cSkinPearlHDDisplayMenu::MenuTitleClean(std::string Title)
{
  int pos = -1;
  if ((pos = Title.find(" - ")) != -1) {
    Title.erase(pos);
	while (Title[Title.length() - 1] == ' ')
	  Title.erase(Title.length() - 1);
  }
  return Title.c_str();
}

const char *cSkinPearlHDDisplayMenu::MenuTitleRest(std::string Title)
{
  int pos = -1;
  if ((pos = Title.find(" - ")) != -1) {
    Title.erase(0, pos + 3);
	while (Title[0] == ' ')
	  Title.erase(0, 1);
  }
  return Title.c_str();
}

void cSkinPearlHDDisplayMenu::DrawHeader()
{
  osd->DrawEllipse(x1Menu, y1Menu, x1Menu+50, y1Menu+50, Theme.Color(clrMainSolid), 2);
  osd->DrawRectangle(x1Menu+50, y1Menu, x2Menu, y1Menu+50, Theme.Color(clrMainSolid));
  osd->DrawRectangle(x1Menu, y1Menu+50, x2Menu, y1Menu+115, Theme.Color(clrMainSolid));
  osd->DrawText(x2Menu-300, y1Menu+43, "VDR", Theme.Color(clrFontColor), clrTransparent, fontSansBook57, 232, 0, taRight);	
  osd->DrawText(x2Menu-300, y1Menu+50, "HD", Theme.Color(clrFontColor), clrTransparent, fontSansBook27, 270, 0, taRight);	
  osd->DrawRectangle(x1Menu, y1Menu+115, x2Menu, y1Menu+117, Theme.Color(clrDivider));
}

void cSkinPearlHDDisplayMenu::DrawRedButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+4, 0xb4690000);
  osd->DrawRectangle(xButton, yButton+4, xButton+237, yButton+6, 0xb4740000);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb47c0000);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+11, 0xb4860000);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+14, 0xb4960000);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+16, 0xb4a60000);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb4ac0000);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+19, 0xb4b70000);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb4bd0000);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb4c20000);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+23, 0xb4cd0000);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb4d80000);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb4d80000);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb4d80000);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb4e20000);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb4e20000);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb4ef0000);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb4ef0000);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb4ef0000);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb4f80000);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb4f80000);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb4f80000);
}

void cSkinPearlHDDisplayMenu::DrawGreenButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+2, 0xb4006400);
  osd->DrawRectangle(xButton, yButton+2, xButton+237, yButton+5, 0xb4006d00);
  osd->DrawRectangle(xButton, yButton+5, xButton+237, yButton+6, 0xb4007400);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb4007c00);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+9, 0xb4008100);
  osd->DrawRectangle(xButton, yButton+9, xButton+237, yButton+11, 0xb4008600);
  osd->DrawRectangle(xButton, yButton+10, xButton+237, yButton+11, 0xb4008b00);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+12, 0xb4009000);
  osd->DrawRectangle(xButton, yButton+12, xButton+237, yButton+13, 0xb4009600);
  osd->DrawRectangle(xButton, yButton+13, xButton+237, yButton+14, 0xb4009b00);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+15, 0xb400a100);
  osd->DrawRectangle(xButton, yButton+15, xButton+237, yButton+16, 0xb400a600);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb400ac00);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+18, 0xb400b200);
  osd->DrawRectangle(xButton, yButton+18, xButton+237, yButton+19, 0xb400b700);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb400bd00);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb400c200);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+22, 0xb400c800);
  osd->DrawRectangle(xButton, yButton+22, xButton+237, yButton+23, 0xb400cd00);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb400d300);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb400d800);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb400dd00);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb400e200);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb400e200);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb400ef00);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb400ef00);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb400ef00);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb400f800);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb400f800);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb400f800);
}

void cSkinPearlHDDisplayMenu::DrawYellowButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+3, 0xb4666600);
  osd->DrawRectangle(xButton, yButton+3, xButton+237, yButton+4, 0xb46d6d00);
  osd->DrawRectangle(xButton, yButton+4, xButton+237, yButton+6, 0xb4707000);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb47c7c00);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+10, 0xb4868600);
  osd->DrawRectangle(xButton, yButton+10, xButton+237, yButton+11, 0xb48b8b00);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+12, 0xb4909000);
  osd->DrawRectangle(xButton, yButton+12, xButton+237, yButton+14, 0xb49b9b00);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+16, 0xb4a6a600);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb4acac00);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+19, 0xb4b7b700);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb4bdbd00);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb4c2c200);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+23, 0xb4cdcd00);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb4d3d300);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb4d8d800);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb4dddd00);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb4e2e200);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb4e2e200);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb4efef00);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb4efef00);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb4efef00);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb4f8f800);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb4f8f800);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb4f8f800);
}

void cSkinPearlHDDisplayMenu::DrawBlueButton(int xButton, int yButton)
{
  osd->DrawRectangle(xButton, yButton, xButton+237, yButton+4, 0xb4000068);
  osd->DrawRectangle(xButton, yButton+4, xButton+237, yButton+6, 0xb4000072);
  osd->DrawRectangle(xButton, yButton+6, xButton+237, yButton+8, 0xb400007a);
  osd->DrawRectangle(xButton, yButton+8, xButton+237, yButton+11, 0xb4000086);
  osd->DrawRectangle(xButton, yButton+11, xButton+237, yButton+14, 0xb4000096);
  osd->DrawRectangle(xButton, yButton+14, xButton+237, yButton+16, 0xb40000a4);
  osd->DrawRectangle(xButton, yButton+16, xButton+237, yButton+17, 0xb40000ac);
  osd->DrawRectangle(xButton, yButton+17, xButton+237, yButton+19, 0xb40000b5);
  osd->DrawRectangle(xButton, yButton+19, xButton+237, yButton+20, 0xb40000bd);
  osd->DrawRectangle(xButton, yButton+20, xButton+237, yButton+21, 0xb40000c2);
  osd->DrawRectangle(xButton, yButton+21, xButton+237, yButton+23, 0xb40000cb);
  osd->DrawRectangle(xButton+1, yButton+23, xButton+237, yButton+24, 0xb40000d8);
  osd->DrawRectangle(xButton+1, yButton+24, xButton+236, yButton+25, 0xb40000d8);
  osd->DrawRectangle(xButton+1, yButton+25, xButton+236, yButton+26, 0xb40000d8);
  osd->DrawRectangle(xButton+2, yButton+26, xButton+235, yButton+27, 0xb40000e4);
  osd->DrawRectangle(xButton+3, yButton+27, xButton+235, yButton+28, 0xb40000e4);
  osd->DrawRectangle(xButton+3, yButton+28, xButton+234, yButton+29, 0xb40000ee);
  osd->DrawRectangle(xButton+4, yButton+29, xButton+233, yButton+30, 0xb40000ee);
  osd->DrawRectangle(xButton+5, yButton+30, xButton+232, yButton+31, 0xb40000ee);
  osd->DrawRectangle(xButton+6, yButton+31, xButton+231, yButton+32, 0xb40000f8);
  osd->DrawRectangle(xButton+8, yButton+32, xButton+229, yButton+33, 0xb40000f8);
  osd->DrawRectangle(xButton+10, yButton+33, xButton+227, yButton+34, 0xb40000f8);
}

int cSkinPearlHDDisplayMenu::GetRecordingLength(const char *FileName, double FramesPerSecond, bool IsPesRecording)
{
#define INDEXFILESUFFIX     "/index"
	struct tIndex { int offset; uchar type; uchar number; short reserved; };
	struct stat buf;
	cString fullname = cString::sprintf("%s%s", FileName, IsPesRecording ? INDEXFILESUFFIX ".vdr" : INDEXFILESUFFIX);
	if (FileName && *fullname && access(fullname, R_OK) == 0 && stat(fullname, &buf) == 0)
		return buf.st_size ? ((buf.st_size - 1) / sizeof(tIndex) + 1) / (60 * FramesPerSecond) : 0;
	return 0;
}

int cSkinPearlHDDisplayMenu::GetRecordingCuttedLength(const char *FileName, double FramesPerSecond, bool IsPesRecording)
{
	cMarks marks;
	double length = 0;
	int totalLength = GetRecordingLength(FileName, FramesPerSecond, IsPesRecording);
	const double diffIFrame = FramesPerSecond / 2; // approx. 1/2 sec.

#if VDRVERSNUM >= 10703
	marks.Load(FileName, FramesPerSecond, IsPesRecording);
#else
	marks.Load(FileName);
#endif

	if (marks.Count()) {
		int start = 1; // first frame
		bool isStart = true;

		for (cMark *m = marks.First(); m; m = marks.GetNext(m->Position())) {
			if (isStart)
				start = m->Position();
			else
				length += (double)(m->Position() - start + 1 + diffIFrame) / (60 * FramesPerSecond); // [min]

			isStart = !isStart;
		}

		// if there is no end-mark the last segment goes to the end of the rec.
		if (!isStart)
			length += totalLength - (double)(start - 1 - diffIFrame) / (60 * FramesPerSecond); // [min]
	}

	// just to avoid, that the cutted length is bigger than the total length
	return (int)length > totalLength ? totalLength : (int)length;
}

bool cSkinPearlHDDisplayMenu::IsTextProgressbar(const char *Text)
{
  std::string text = Text;
  bool isprogress = false;
  if (text.length() > 5 && text[0] == '[' && text[text.length() - 1] == ']') {
    const char *p = text.c_str() + 1;
    isprogress = true;
      for (; *p != ']'; ++p) {
      if (*p != ' ' && *p != '|') {
        isprogress = false;
        break;
      }
    }
  }
  return isprogress;
}

// --- cSkinPearlHDDisplayReplay -----------------------------------------------

class cSkinPearlHDDisplayReplay : public cSkinDisplayReplay {
private:
  cOsd *osd;
  int x1, x2;
  int y1, y2;
  const cFont *fontSansBook27;
  char *current;
  char *total;
  int x1Message;
  int x2Message;
  int y1Message;
  int y2Message;
public:
  cSkinPearlHDDisplayReplay(bool ModeOnly);
  virtual ~cSkinPearlHDDisplayReplay();
  virtual void SetTitle(const char *Title);
  virtual void SetMode(bool Play, bool Forward, int Speed);
  virtual void SetProgress(int Current, int Total);
  virtual void SetCurrent(const char *Current);
  virtual void SetTotal(const char *Total);
  virtual void SetJump(const char *Jump);
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };

cSkinPearlHDDisplayReplay::cSkinPearlHDDisplayReplay(bool ModeOnly)
{
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);
  x1 = 0;
  x2 = cOsd::OsdWidth();
  y1 = 0;
  y2 = cOsd::OsdHeight();
  x1Message = 480;
  x2Message = cOsd::OsdWidth() - 1;
  y1Message = cOsd::OsdHeight() - 70;
  y2Message = cOsd::OsdHeight() - 1;

  osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop());
  tArea Areas[] = { { 0, 0, x2 - 1, y2 - 1, 32 } }; // TrueColor
     if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
        osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
     else {
        tArea Areas[] = { { 0, 0, x2 - 1, y2 - 1, 8 } }; // 256 colors
        if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
           osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
        }

  if (!ModeOnly)
  {
    osd->DrawRectangle(x2-360, y1, x2-1, y1+20, Theme.Color(clrMainLight));
    osd->DrawEllipse(x2-360, y1+20, x2-310, y1+70, Theme.Color(clrMainLight), 3);
    osd->DrawRectangle(x2-310, y1+20, x2-1, y1+70, Theme.Color(clrMainLight));
  
    osd->DrawEllipse(x1, y2-110, x1+50, y2-60, Theme.Color(clrMainLight), 2);
    osd->DrawRectangle(x1+51, y2-110, x2-1, y2-60, Theme.Color(clrMainLight));
    osd->DrawRectangle(x1, y2-60, x2-1, y2-40, Theme.Color(clrMainSolid));
  }
}

cSkinPearlHDDisplayReplay::~cSkinPearlHDDisplayReplay()
{
  delete osd;
}

void cSkinPearlHDDisplayReplay::SetTitle(const char *Title)
{
  osd->DrawText(x1+240, y2-95, Title, Theme.Color(clrFontColor), clrTransparent, fontSansBook27);
}

static const char *ReplaySymbols[2][2][5] = {
  { { tr("Pause"), tr("Slow rewind"), tr("Slow rewind x1"), tr("Slow rewind x2"), tr("Slow rewind x3") },
    { tr("Pause"), tr("Slow forward"), tr("Slow forward x1"), tr("Slow forward x2"), tr("Slow forward x3") }, },
  { { tr("Playing"),  tr("Fast rewind"), tr("Fast rewind x1"), tr("Fast rewind x2"), tr("Fast rewind x3") },
    { tr("Playing"),  tr("Fast forward"), tr("Fast forward x1"), tr("Fast forward x2"), tr("Fast forward x3") } }
  };

void cSkinPearlHDDisplayReplay::SetMode(bool Play, bool Forward, int Speed)
{
  if (Speed < -1)
     Speed = -1;
  if (Speed > 3)
     Speed = 3;
  osd->DrawText(x2-310,y1+20, ReplaySymbols[Play][Forward][Speed + 1], Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 290, 40, taRight);
}

void cSkinPearlHDDisplayReplay::SetProgress(int Current, int Total)
{
  cProgressBar pb(x2-x1-1, 39, Current, Total, marks, Theme.Color(clrBarFill), Theme.Color(clrBar), Theme.Color(clrKeep), Theme.Color(clrMark), clrTransparent);
  osd->DrawBitmap(x1, y2-40, pb);
}

void cSkinPearlHDDisplayReplay::SetCurrent(const char *Current)
{
  if (!Current)
    return;
  current = (char*) malloc(sizeof(Current));
  strcpy (current, Current);
}

void cSkinPearlHDDisplayReplay::SetTotal(const char *Total)
{
  if (!Total)
    return;
  total = (char*) malloc(sizeof(Total));
  strcpy (total, Total);
}

void cSkinPearlHDDisplayReplay::SetJump(const char *Jump)
{
  osd->DrawEllipse(x2-360, y2-160, x2-310, y2-111, Theme.Color(clrMainLight), 2);
  osd->DrawRectangle(x2-310, y2-160, x2-1, y2-111, Theme.Color(clrMainLight));
  osd->DrawText(x2-250, y2-150, Jump, Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 220, 39, taRight);
}

void cSkinPearlHDDisplayReplay::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
    osd->DrawRectangle(x1Message + 50, y1Message, x2Message, y1Message + 50, Theme.Color(clrMainLight));
	osd->DrawEllipse(x1Message, y1Message, x1Message + 50, y1Message + 50, Theme.Color(clrMainLight), 2);
	switch(Type) {
	  case mtInfo :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrMainSolid));
	    break;
	  case mtStatus :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrStatus));
	    break;
	  case mtWarning :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrWarning));
	    break;
	  case mtError :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrError));
	    break;
	}
	osd->DrawText(x1Message + 50, y1Message + 15, Text, Theme.Color(clrFontColor), clrTransparent, fontSansBook27, x2Message - (x1Message+50), 0, taRight);			
  }
  else {
    osd->RestoreRegion();
  }
}

void cSkinPearlHDDisplayReplay::Flush(void)
{
  if (current && total)
  {
	char timeplayed[50];
	snprintf (timeplayed, sizeof(timeplayed), "%s %s %s", current, tr("of"), total);
    osd->DrawText(x2-30-fontSansBook27->Width(timeplayed), y2-110, timeplayed, Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, fontSansBook27->Width(timeplayed), 50, taRight);
  }
  osd->Flush();
}

// --- cSkinPearlHDDisplayVolume -----------------------------------------------

class cSkinPearlHDDisplayVolume : public cSkinDisplayVolume {
private:
  cOsd *osd;
  int x1, x2;
  int y1, y2;
  const cFont *fontSansBook27;
public:
  cSkinPearlHDDisplayVolume(void);
  virtual ~cSkinPearlHDDisplayVolume();
  virtual void SetVolume(int Current, int Total, bool Mute);
  virtual void Flush(void);
  };

cSkinPearlHDDisplayVolume::cSkinPearlHDDisplayVolume(void)
{
  x1 = 0;
  x2 = cOsd::OsdWidth();
  y1 = 0;
  y2 = cOsd::OsdHeight();
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);

  osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop());
  tArea Areas[] = { { 0, 0, x2 - 1, y2 - 1, 32 } }; // TrueColor
     if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
        osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
     else {
        tArea Areas[] = { { 0, 0, x2 - 1, y2 - 1, 8 } }; // 256 colors
        if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
           osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
		   }
}

cSkinPearlHDDisplayVolume::~cSkinPearlHDDisplayVolume()
{
  delete osd;
}

void cSkinPearlHDDisplayVolume::SetVolume(int Current, int Total, bool Mute)
{
  osd->DrawRectangle(x2-360, y1, x2-1, y1+20, Theme.Color(clrMainLight));
  osd->DrawEllipse(x2-360, y1+20, x2-310, y1+70, Theme.Color(clrMainLight), 3);
  osd->DrawRectangle(x2-310, y1+20, x2-1, y1+70, Theme.Color(clrMainLight));

  if (!Mute)
  {
    osd->DrawText(x2-300, y1+20, tr("Volume"), Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27);
	int iVolLengthTotal = (x2-1) - x1;
	int iVolLengthCurrent = (int)((float)((float)Current / (float)Total) * (float)iVolLengthTotal);
	osd->DrawRectangle(x1, y2-40, x2-1, y2-1, Theme.Color(clrBar));
	osd->DrawRectangle(x1, y2-40, x1+iVolLengthCurrent, y2-1, Theme.Color(clrBarFill));
  }
  else
    osd->DrawRectangle(x1, y2-40, x2-1, y2-1, Theme.Color(clrBarBlock));
	
  char vol[5];
  snprintf(vol, sizeof(vol), "%d", Current);
  osd->DrawText(x2-100, y1+20, Mute ? tr("Mute") : vol, Theme.Color(clrFontColor), Theme.Color(clrMainLight), fontSansBook27, 80, 0, taRight);
}

void cSkinPearlHDDisplayVolume::Flush(void)
{
  osd->Flush();
}

// --- cSkinPearlHDDisplayTracks -----------------------------------------------

class cSkinPearlHDDisplayTracks : public cSkinDisplayTracks {
private:
  cOsd *osd;
  int x1, x2;
  int y1, y2;
  int lineHeight;
  int currentIndex;
  void SetItem(const char *Text, int Index, bool Current);
  const cFont *fontSansBook27;
  const cFont *fontSansBook37;
public:
  cSkinPearlHDDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
  virtual ~cSkinPearlHDDisplayTracks();
  virtual void SetTrack(int Index, const char * const *Tracks);
  virtual void SetAudioChannel(int AudioChannel);
  virtual void Flush(void);
  };

cSkinPearlHDDisplayTracks::cSkinPearlHDDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
  lineHeight = 50;
  currentIndex = -1;
  x1 = 0;
  x2 = cOsd::OsdWidth();
  y1 = 0;
  y2 = cOsd::OsdHeight();
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);
  fontSansBook37 = cFont::CreateFont("VDRSymbols Sans:Book", 37);

  osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop());
  tArea Areas[] = { { 0, 0, x2 - 1, y2 - 1, 32 } }; // TrueColor
  if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
    osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
  else {
    tArea Areas[] = { { 0, 0, x2 - 1, y2 - 1, 8 } }; // 256 colors
    if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
      osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
  }
		
  osd->DrawEllipse(x1+480, y1+345, x1+530, y1+395, Theme.Color(clrMainSolid), 2);
  osd->DrawRectangle(x1+531, y1+345, x2-480, y1+395, Theme.Color(clrMainSolid));
  osd->DrawRectangle(x1+480, y1+396, x2-480, y1+444, Theme.Color(clrMainSolid));
  osd->DrawRectangle(x1+480, y1+444, x2-480, y1+446, Theme.Color(clrDivider));
  osd->DrawRectangle(x1+480, y1+446, x2-480, y2-406, Theme.Color(clrMainLight));
  osd->DrawRectangle(x1+480, y2-405, x2-530, y2-355, Theme.Color(clrMainLight));
  osd->DrawEllipse(x2-530, y2-405, x2-480, y2-355, Theme.Color(clrMainLight), 4);
  osd->DrawText(x1+730, y1+380, Title, Theme.Color(clrFontColor), clrTransparent, fontSansBook37, (x2-500) - (x1+730), 0, taRight);

  for (int i = 0; i < NumTracks; i++)
      SetItem(Tracks[i], i, false);
}

cSkinPearlHDDisplayTracks::~cSkinPearlHDDisplayTracks()
{
  delete osd;
}

void cSkinPearlHDDisplayTracks::SetItem(const char *Text, int Index, bool Current)
{
  int x1Item=x1+480;
  int y1Item=y1+460 + (lineHeight * Index);
  int x2Item=x2-480;
  int y2Item=y1Item + lineHeight;

  osd->DrawRectangle(x1Item+74, y1Item, x2Item-1, y2Item-6, Theme.Color(Current ? clrMainSolid : clrLoLight));
  osd->DrawEllipse(x1Item+60, y1Item, x1Item+74, y1Item+15, Theme.Color(Current ? clrMainSolid : clrLoLight), 2);
  osd->DrawRectangle(x1Item+60, y1Item+14, x1Item+74, y2Item-21, Theme.Color(Current ? clrMainSolid : clrLoLight));
  osd->DrawEllipse(x1Item+60, y2Item-21, x1Item+74, y2Item-6, Theme.Color(Current ? clrMainSolid : clrLoLight), 3);
  osd->DrawText(x1Item+80, y1Item+5, Text, Theme.Color(Current ? clrFontColor : clrFontColorInactive), clrTransparent, fontSansBook27, (x2Item-21) - (x1Item+80), 0, taRight);
  
  if (Current)
  {
    osd->DrawRectangle(x1Item+74, y1Item, x2Item-1, y2Item-6, Theme.Color(clrMainSolid));
    osd->DrawEllipse(x1Item+60, y1Item, x1Item+74, y1Item+15, Theme.Color(clrMainSolid), 2);
    osd->DrawRectangle(x1Item+60, y1Item+14, x1Item+74, y2Item-21, Theme.Color(clrMainSolid));
    osd->DrawEllipse(x1Item+60, y2Item-21, x1Item+74, y2Item-6, Theme.Color(clrMainSolid), 3);
    osd->DrawText(x1Item+80, y1Item+5, Text, Theme.Color(clrFontColor), clrTransparent, fontSansBook27, (x2Item-21) - (x1Item+80), 0, taRight);
	currentIndex = Index;
  }
  else if (currentIndex == Index)
  {
    osd->DrawRectangle(x1Item+74, y1Item, x2Item-1, y2Item-6, Theme.Color(clrLoLight));
    osd->DrawEllipse(x1Item+60, y1Item, x1Item+74, y1Item+15, Theme.Color(clrLoLight), 2);
    osd->DrawRectangle(x1Item+60, y1Item+14, x1Item+74, y2Item-21, Theme.Color(clrLoLight));
    osd->DrawEllipse(x1Item+60, y2Item-21, x1Item+74, y2Item-6, Theme.Color(clrLoLight), 3);
    osd->DrawText(x1Item+80, y1Item+5, Text, Theme.Color(clrFontColorInactive), clrTransparent, fontSansBook27, (x2Item-21) - (x1Item+80), 0, taRight);
  }
}

void cSkinPearlHDDisplayTracks::SetTrack(int Index, const char * const *Tracks)
{
  if (currentIndex >= 0)
     SetItem(Tracks[currentIndex], currentIndex, false);
  SetItem(Tracks[Index], Index, true);
}

void cSkinPearlHDDisplayTracks::SetAudioChannel(int AudioChannel)
{
  int x1Icon = x1+555;
  int y1Icon = y1+390;

  switch (AudioChannel) {
    case 0: 
	  osd->DrawRectangle(x1Icon+2, y1Icon+2, x1Icon+23, y1Icon+18, Theme.Color(clrFontColor));
	  osd->DrawRectangle(x1Icon+3, y1Icon+3, x1Icon+22, y1Icon+17, Theme.Color(clrMainLight));
	  osd->DrawEllipse(x1Icon+5, y1Icon+6, x1Icon+13, y1Icon+14, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+6, y1Icon+7, x1Icon+14, y1Icon+13, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+12, y1Icon+6, x1Icon+20, y1Icon+14, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+13, y1Icon+7, x1Icon+19, y1Icon+13, Theme.Color(clrFontColor));
	  break;
    case 1: 
	  osd->DrawRectangle(x1Icon+2, y1Icon+2, x1Icon+23, y1Icon+18, Theme.Color(clrFontColor));
	  osd->DrawRectangle(x1Icon+3, y1Icon+3, x1Icon+22, y1Icon+17, Theme.Color(clrMainLight));
	  osd->DrawEllipse(x1Icon+5, y1Icon+6, x1Icon+13, y1Icon+14, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+6, y1Icon+7, x1Icon+14, y1Icon+13, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+12, y1Icon+6, x1Icon+20, y1Icon+14, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+13, y1Icon+7, x1Icon+19, y1Icon+13, Theme.Color(clrMainLight));
	  break;
    case 2: 
	  osd->DrawRectangle(x1Icon+2, y1Icon+2, x1Icon+23, y1Icon+18, Theme.Color(clrFontColor));
	  osd->DrawRectangle(x1Icon+3, y1Icon+3, x1Icon+22, y1Icon+17, Theme.Color(clrMainLight));
	  osd->DrawEllipse(x1Icon+5, y1Icon+6, x1Icon+13, y1Icon+14, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+6, y1Icon+7, x1Icon+14, y1Icon+13, Theme.Color(clrMainLight));
	  osd->DrawEllipse(x1Icon+12, y1Icon+6, x1Icon+20, y1Icon+14, Theme.Color(clrFontColor));
	  osd->DrawEllipse(x1Icon+13, y1Icon+7, x1Icon+19, y1Icon+13, Theme.Color(clrFontColor));
	  break;
    default: ;
    }
}

void cSkinPearlHDDisplayTracks::Flush(void)
{
  osd->Flush();
}

// --- cSkinPearlHDDisplayMessage ----------------------------------------------

class cSkinPearlHDDisplayMessage : public cSkinDisplayMessage {
private:
  cOsd *osd;
  int x1Message;
  int x2Message;
  int y1Message;
  int y2Message;
  const cFont *fontSansBook27;
public:
  cSkinPearlHDDisplayMessage(void);
  virtual ~cSkinPearlHDDisplayMessage();
  virtual void SetMessage(eMessageType Type, const char *Text);
  virtual void Flush(void);
  };

cSkinPearlHDDisplayMessage::cSkinPearlHDDisplayMessage(void)
{
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);
  x1Message = 480;
  x2Message = cOsd::OsdWidth() - 1;
  y1Message = cOsd::OsdHeight() - 70;
  y2Message = cOsd::OsdHeight() - 1;
  fontSansBook27 = cFont::CreateFont("VDRSymbols Sans:Book", 27);

  osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop());
  tArea Areas[] = { { 0, 0, x2Message, y2Message, 32 } }; // TrueColor
    if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
      osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
    else {
      tArea Areas[] = { { 0, 0, x2Message, y2Message, 8 } }; // 256 colors
      if (osd->CanHandleAreas(Areas, sizeof(Areas) / sizeof(tArea)) == oeOk)
        osd->SetAreas(Areas, sizeof(Areas) / sizeof(tArea));
    }
}

cSkinPearlHDDisplayMessage::~cSkinPearlHDDisplayMessage()
{
  delete osd;
}

void cSkinPearlHDDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
  if (Text) {
    osd->DrawRectangle(x1Message + 50, y1Message, x2Message, y1Message + 50, Theme.Color(clrMainLight));
	osd->DrawEllipse(x1Message, y1Message, x1Message + 50, y1Message + 50, Theme.Color(clrMainLight), 2);
	switch(Type) {
	  case mtInfo :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrMainSolid));
	    break;
	  case mtStatus :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrStatus));
	    break;
	  case mtWarning :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrWarning));
	    break;
	  case mtError :
	    osd->DrawRectangle(x1Message, y1Message + 50, x2Message, y2Message, Theme.Color(clrError));
	    break;
	}
	osd->DrawText(x1Message + 50, y1Message + 15, Text, Theme.Color(clrFontColor), clrTransparent, fontSansBook27, x2Message - (x1Message+50), 0, taRight);			
  }
}

void cSkinPearlHDDisplayMessage::Flush(void)
{
  osd->Flush();
}

// --- cSkinPearlHD ----------------------------------------------------------

cSkinPearlHD::cSkinPearlHD() : cSkin("PearlHD(native)", &::Theme)
{
  // Get the "classic" skin to be used as fallback skin if any of the OSD
  // menu fails to open.
  skinFallback = Skins.First();
   for (cSkin *Skin = Skins.First(); Skin; Skin = Skins.Next(Skin)) {
     if (strcmp(Skin->Name(), "classic") == 0) {
      skinFallback = Skin;
      break;
    }
   }
}

const char *cSkinPearlHD::Description(void)
{
  return tr("PearlHD(native)");
}

cSkinDisplayChannel *cSkinPearlHD::DisplayChannel(bool WithInfo)
{
  try {
    return new cSkinPearlHDDisplayChannel(WithInfo);
  } catch(...) {
    return skinFallback->DisplayChannel(WithInfo);
  }
}

cSkinDisplayMenu *cSkinPearlHD::DisplayMenu(void)
{
  try {
    return new cSkinPearlHDDisplayMenu;
  } catch (...) {
    return skinFallback->DisplayMenu();
  }
}

cSkinDisplayReplay *cSkinPearlHD::DisplayReplay(bool ModeOnly)
{
  try {
    return new cSkinPearlHDDisplayReplay(ModeOnly);
  } catch (...) {
    return skinFallback->DisplayReplay(ModeOnly);
  }
}

cSkinDisplayVolume *cSkinPearlHD::DisplayVolume(void)
{
  try {
    return new cSkinPearlHDDisplayVolume;
  } catch (...) {
    return skinFallback->DisplayVolume();
  }
}

cSkinDisplayTracks *cSkinPearlHD::DisplayTracks(const char *Title, int NumTracks, const char *const *Tracks)
{
  try {
    return new cSkinPearlHDDisplayTracks(Title, NumTracks, Tracks);
  } catch (...) {
    return skinFallback->DisplayTracks(Title, NumTracks, Tracks);
  }
}

cSkinDisplayMessage *cSkinPearlHD::DisplayMessage(void)
{
  try {
    return new cSkinPearlHDDisplayMessage;
  } catch (...) {
    return skinFallback->DisplayMessage();
  }
}
