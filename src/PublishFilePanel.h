#ifndef PUBLISHFILEPANEL_H
#define PUBLISHFILEPANEL_H

#include <FilePanel.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Query.h>
#include <VolumeRoster.h>
#include <Volume.h>

#include "CommonFunctions.h"

#define PUBLISH_TYPE 'ptyp'

class PublishFilePanel : public BFilePanel
{
	public:
					PublishFilePanel(BMessenger* target);
		BMenuField*	publishTypeMenuField;
		BMenu*		publishTypeMenu;
		
	private:
		BMessage*	pubMsg;
};
#endif
