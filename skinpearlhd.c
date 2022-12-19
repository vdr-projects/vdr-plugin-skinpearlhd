/*
 * skinpearlhd.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */
#include <getopt.h>
#include <string>

#include "pearlhd.h"
#include "setup.h"
#include "config.h"

#include <vdr/plugin.h>

static const char *VERSION        = "0.0.2";
static const char *DESCRIPTION    = "PearlHD Skin";

class cPluginSkinpearlhd : public cPlugin {
private:

public:
  cPluginSkinpearlhd(void);
  virtual ~cPluginSkinpearlhd();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual time_t WakeupTime(void);
  virtual const char *MainMenuEntry(void) { return NULL; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginSkinpearlhd::cPluginSkinpearlhd(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginSkinpearlhd::~cPluginSkinpearlhd()
{
  // Clean up after yourself!
}

const char *cPluginSkinpearlhd::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return "  -e DIR,   --epgimages=DIR      search in DIR for epg-images\n";
}

bool cPluginSkinpearlhd::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  static struct option long_options[] = {
       { "epgimages",      required_argument, NULL, 'e' },
       { NULL,       no_argument,       NULL,  0  }
     };

  int c;
  while ((c = getopt_long(argc, argv, "e:", long_options, NULL)) != -1) {
        switch (c) {
          case 'e':
		    PearlHDConfig.SetEpgImagesDir(optarg);
          break;
          default:
		  break;
          }
        }
  return true;
}

bool cPluginSkinpearlhd::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  return true;
}

bool cPluginSkinpearlhd::Start(void)
{
  // Start any background activities the plugin shall perform.
  new cSkinPearlHD;
  return true;
}

void cPluginSkinpearlhd::Stop(void)
{
  // Stop any background activities the plugin is performing.
}

void cPluginSkinpearlhd::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginSkinpearlhd::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginSkinpearlhd::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

time_t cPluginSkinpearlhd::WakeupTime(void)
{
  // Return custom wakeup time for shutdown script
  return 0;
}

cOsdObject *cPluginSkinpearlhd::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginSkinpearlhd::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return new cSkinPearlHDSetup();
}

bool cPluginSkinpearlhd::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return PearlHDConfig.SetupParse(Name, Value);
}

bool cPluginSkinpearlhd::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginSkinpearlhd::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginSkinpearlhd::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}

VDRPLUGINCREATOR(cPluginSkinpearlhd); // Don't touch this!
