//pkgEnabler - An application for PS3 that enables/disables the "Install Package Files" on OFW/CFW.
//Author:	Axtux.tk
//Based on tools by jjolano and Cyberskunk

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include <io/pad.h>
#include <io/msg.h>

#include <sysutil/events.h>

#include <psl1ght/lv2/filesystem.h>

#include "rsxutil.h"

const char* MOUNT_POINT = "/dev_rwflash";
const char* APPID = "PKGENABLE";
int currentBuffer = 0;
msgButton dlg_action;

void handledialog(msgButton button, void *userdata)
{
	dlg_action = button;
}

void showmessage(msgType type, const char* message)
{
	msgDialogOpen(type, message, handledialog, 0, NULL);
	
	dlg_action = 0;
	while(!dlg_action)
	{
		sysCheckCallback();
		
		flip(currentBuffer);
		waitFlip();
		currentBuffer = !currentBuffer;
	}
	
	msgDialogClose();
}

int exists(const char* path)
{
	Lv2FsStat entry;
	return lv2FsStat(path, &entry);
}

unsigned long long size(const char* file)
{
	Lv2FsStat entry;
	lv2FsStat(file, &entry);
	return (unsigned long long)entry.st_size;
}

int copyfile(const char* fn_src, const char* fn_dst)
{
	u64 pos;
	u64 read;
	u64 write;
	
	Lv2FsFile src = -1;
	Lv2FsFile dst = -1;
	
	if(lv2FsOpen(fn_src, LV2_O_RDONLY, &src, 0, NULL, 0) != 0 || lv2FsOpen(fn_dst, LV2_O_WRONLY | LV2_O_CREAT | LV2_O_TRUNC, &dst, 0, NULL, 0) != 0)
	{
		return -1;
	}
	
	lv2FsChmod(fn_dst, S_IFMT | 0777);
	
	lv2FsLSeek64(src, 0, 0, &pos);
	lv2FsLSeek64(dst, 0, 0, &pos);
	
	char buffer[32768];
	
	while(lv2FsRead(src, buffer, sizeof(buffer) - 1, &read) == 0 && read > 0)
	{
		lv2FsWrite(dst, buffer, read, &write);
	}
	
	if(read != 0)
	{
		return -1;
	}
	
	lv2FsClose(src);
	lv2FsClose(dst);
	
	return 0;
}

int main(int argc, const char* argv[])
{
	msgType mdialogok	= MSGDIALOG_NORMAL | MSGDIALOG_BUTTON_TYPE_OK;
	msgType mdialogyesno	= MSGDIALOG_NORMAL | MSGDIALOG_BUTTON_TYPE_YESNO;
	
	init_screen();
	ioPadInit(7);
	
	waitFlip();
	
	unsigned long long sizeCG = size("/dev_flash/vsh/resource/explore/xmb/category_game.xml");
	int OFW = (sizeCG == 24314 || sizeCG == 23830); //check if OFW or CFW
	
	showmessage(mdialogyesno, OFW ? "Package Enabler by @Axtux (http://Axtux.tk)\n\nDo you want to enable \"Install package files\" ?" : "Package Enabler by @Axtux (http://Axtux.tk)\n\nDo you want to disable \"Install package files\" ?");
	
	if(dlg_action == MSGDIALOG_BUTTON_YES)
	{
		int rwflash = (exists(MOUNT_POINT) == 0); //check if writable flash is mounted
		char error[128];
		
		if(!rwflash) // mount writable flash if not mounted
		{
			if(Lv2Syscall8(837, (u64)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)"CELL_FS_FAT", (u64)MOUNT_POINT, 0, 0, 0, 0, 0) != 0)
			{
				sprintf(error, "An error occured while mounting %s.", MOUNT_POINT);
				showmessage(mdialogok, error);
				return 0; //quit if writable flash mount fails
			}
		}
		
		char SRC1[128], SRC2[128], DST1[128], DST2[128];
		
		if(OFW)
		{
			sprintf(SRC1, "/dev_hdd0/game/%s/USRDIR/CFW/category_game.xml", APPID);
			sprintf(SRC2, "/dev_hdd0/game/%s/USRDIR/CFW/nas_plugin.sprx", APPID);
		}
		else
		{
			sprintf(SRC1, "/dev_hdd0/game/%s/USRDIR/OFW/category_game.xml", APPID);
			sprintf(SRC2, "/dev_hdd0/game/%s/USRDIR/OFW/nas_plugin.sprx", APPID);
		}
		
		sprintf(DST1, "%s/vsh/resource/explore/xmb/category_game.xml", MOUNT_POINT);
		sprintf(DST2, "%s/vsh/module/nas_plugin.sprx", MOUNT_POINT);
		
		if(copyfile(SRC1, DST1) != 0) //replace category_game.xml
		{
			sprintf(error, "An error occured while copying %s to %s.", SRC1, DST1);
			showmessage(mdialogok, error);
		}
		
		if(copyfile(SRC2, DST2) != 0) //replace nas_plugin.sprx
		{
			sprintf(error, "An error occured while copying %s to %s.", SRC2, DST2);
			showmessage(mdialogok, error);
		}
		
		if(!rwflash) //unmount writable flash if it wasn't mounted
			if(Lv2Syscall1(838, (u64)MOUNT_POINT) != 0)
				showmessage(mdialogok, "An error occured while unmounting writable flash.");
		
		//Must find a way to shutdown or restart PS3
		if(strcasecmp(error, "") == 0)
			showmessage(mdialogok, OFW ? "Package Enabler by @Axtux (http://Axtux.tk)\n\nSuccessfully enabled \"Install package files\"\n\nNow DON'T quit game, turn off the system." : "Package Enabler by @Axtux (http://Axtux.tk)\n\nSuccessfully disabled \"Install package files\"\n\nNow DON'T quit game, turn off the system.");
		
		while(1)
			showmessage(mdialogok, "Package Enabler by @Axtux (http://Axtux.tk)\n\nNow DON'T quit game, turn off the system.");
	}
	
	return 0;
}
