//pkgEnabler - An application for PS3 that enables and disables the "Install Package Files".
//Author:	jjolano
//			Cyberskunk
//			Axtux

#include <stdio.h>
#include <fcntl.h>

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

int size(const char* file)
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
	
	int OFW = (size("/dev_flash/vsh/resource/explore/xmb/category_game.xml") == 24314); //check if OFW or CFW
	
	showmessage(mdialogyesno, OFW ? "Do you want to enable \"Install package files\" ?" : "Do you want to disable \"Install package files\" ?");
	
	if(dlg_action == MSGDIALOG_BUTTON_YES)
	{
		int rwflash = (exists(MOUNT_POINT) == 0); //check if writable flash is mounted
		char error[128];
		
		if(!rwflash)
		{
			if(Lv2Syscall8(837, (u64)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)"CELL_FS_FAT", (u64)MOUNT_POINT, 0, 0, 0, 0, 0) != 0) // mount writable flash if not mounted
			{
				sprintf(error, "An error occured while mounting %s.", MOUNT_POINT);
				showmessage(mdialogok, error);
				return 0; //quit if mount writable flash fail
			}
		}
		
		char SRC1[128], SRC2[128], DST1[128], DST2[128];
		
		if(OFW)
		{
			sprintf(SRC1, "/dev_hdd0/game/%s/USRDIR/CFW/gategory_game.xml", APPID);
			sprintf(SRC2, "/dev_hdd0/game/%s/USRDIR/CFW/nas_plugin.sprx", APPID);
		}
		else
		{
			sprintf(SRC1, "/dev_hdd0/game/%s/USRDIR/OFW/gategory_game.xml", APPID);
			sprintf(SRC2, "/dev_hdd0/game/%s/USRDIR/OFW/nas_plugin.sprx", APPID);
		}
		
		sprintf(DST1, "%s/vsh/resource/explore/xmb/category_game.xml", MOUNT_POINT);
		sprintf(DST2, "%s/vsh/module/nas_plugin.sprx", MOUNT_POINT);
		/* Not usefull because copyfile replace file
		if(unlink(DST1) != 0)
		{
			sprintf(error, "An error occured while deleting %s.", DST1);
			showmessage(mdialogok, error);
		}
		
		if(unlink(DST2) != 0)
		{
			sprintf(error, "An error occured while deleting %s.", DST2);
			showmessage(mdialogok, error);
		}
		//*/
		
		if(copyfile(SRC1, DST1) != 0)
		{
			sprintf(error, "An error occured while copying %s to %s.", SRC1, DST1);
			showmessage(mdialogok, error);
		}
		
		if(copyfile(SRC2, DST2) != 0)
		{
			sprintf(error, "An error occured while copying %s to %s.", SRC2, DST2);
			showmessage(mdialogok, error);
		}
		
		if(!rwflash) //unmount writable flash if it wasn't mounted
			if(Lv2Syscall1(838, (u64)MOUNT_POINT) != 0)
				showmessage(mdialogok, "An error occured while unmounting writable flash.");
		
		//Must find a way to shutdown(0) or restart(1) PS3
		//lv1_panic(0);
		//Lv1Syscall2(255, 0);
		
		while(1)
			showmessage(mdialogok, "You must turn off the system manually.\nHold POWER button for 5 seconds.");
	}
	
	return 0;
}
