#ifndef __SKINPEARLHD_SETUP_H
#define __SKINPEARLHD_SETUP_H

#include <vdr/plugin.h>
#include "config.h"


class cSkinPearlHDSetup : public cMenuSetupPage
{
private:
   cSkinPearlHDConfig tmpPearlHDConfig;
   void Setup(void);

protected:
   virtual eOSState ProcessKey(eKeys Key);
   virtual void Store(void);

public:
   cSkinPearlHDSetup(void);
   virtual ~cSkinPearlHDSetup();
};

#endif //__SKINPEARLHD_SETUP_H
