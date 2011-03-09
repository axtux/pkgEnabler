//CFW Enabler - An application for PS3 that enables and disables the CFW 3.55.
//Author:	jjolano
//			Cyberskunk
//			Axtux

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

#include <io/pad.h>
#include <io/msg.h>

#include <sysutil/events.h>

#include <lv2/process.h>

#include <psl1ght/lv2.h>
#include <psl1ght/lv2/filesystem.h>

#include "rsxutil.h"
#include "filesystem_mount.h"

const char* MOUNT_POINT = "/dev_wflash";
const char* APPID = "AXTUX0003";
int currentBuffer = 0;
msgButton dlg_action;

void handledialog(msgButton button, void *userdata)
{
	dlg_action = button;
}

void eventHandler(u64 status, u64 param, void * userdata)
{
	if(status == EVENT_REQUEST_EXITAPP)
	{
		sysProcessExit(0);
	}
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
	
	sysRegisterCallback(EVENT_SLOT0, eventHandler, NULL);
	
	init_screen();
	ioPadInit(7);
	
	waitFlip();
	
	Lv2FsStat buf;
	lv2FsStat("/dev_flash/vsh/resource/explore/xmb/category_game.xml", &buf);
	
	int OFW = ((unsigned long long)buf.st_size == 24314);
	
	/*
	if(OFW)
		showmessage(mdialogok, "You are on OFW.");
	else
		showmessage(mdialogok, "You are on CFW.");
	//*/
	/*
	Lv2FsStat entry;
	int is_mounted = lv2FsStat(MOUNT_POINT, &entry);
		
		showmessage(mdialogyesno, (is_mounted != 0) ? "Do you want to mount dev_wflash and enable \"Install package file\" ?" : "Do you want to enable \"Install package file\" ?");
		
		
		if(dlg_action == MSGDIALOG_BUTTON_YES)
		{
			if(is_mounted != 0)
				showmessage(mdialogok, (lv2FsMount(DEV_FLASH1, FS_FAT32, MOUNT_POINT, 0) == 0) ? "Successfully mounted dev_wflash." : "An error occured while mounting dev_wflash.");
			else
				showmessage(mdialogok, "dev_wflash is already mounted.");
			
			//char SRC1[128], SRC2[128], DST1[128], DST2[128];
			//sprintf(FILE1, "/dev_hdd0/game/%s/USRDIR/OFW/nas_plugin.sprx", APPID);
			//sprintf(FILE2, "/dev_hdd0/game/%s/USRDIR/OFW/gategory_game.xml", APPID);
			
	
			
			showmessage(mdialogok, (copyfile(FILE1, FILE2) == 0) ? "Successfully mounted dev_wflash." : "An error occured while mounting dev_wflash.");
			
						if(copyfile("/dev_flash/rebug/debug_menu_1/sysconf_plugin.sprx", "/dev_blind/vsh/module/sysconf_plugin.sprx") == 0)
						{
							strcpy(status, "Status: Debug Menu 1 installed");
						}
						else
						{
							strcpy(status, "Status: Debug Menu 1 install failed");
						}
						
						sleep(1);
					}
					if(paddata.BTN_CIRCLE)
					{
						if(copyfile("/dev_flash/rebug/debug_menu_2/sysconf_plugin.sprx", "/dev_blind/vsh/module/sysconf_plugin.sprx") == 0)
						{
							strcpy(status, "Status: Debug Menu 2 installed");
						}
						else
						{
							strcpy(status, "Status: Debug Menu 2 install failed");
						}
						
						sleep(1);
					}
					if(paddata.BTN_SQUARE)
					{
						if(copyfile("/dev_flash/rebug/debug_xmb/vsh.self", "/dev_blind/vsh/module/vsh.self") == 0)
						{
							strcpy(status, "Status: Debug XMB installed");
						}
						else
						{
							strcpy(status, "Status: Debug XMB install failed");
						}
						
						sleep(1);
					}
					if(paddata.BTN_TRIANGLE)
					{
						if(copyfile("/dev_flash/rebug/retail_xmb/vsh.self", "/dev_blind/vsh/module/vsh.self") == 0)
						{
							strcpy(status, "Status: Retail XMB installed");
						}
						else
						{
							strcpy(status, "Status: Retail XMB install failed");
						}
						
						sleep(1);
					}
					if(paddata.BTN_START)
					{
						if(copyfile("/dev_usb000/xRegistry.sys", "/dev_flash2/etc/xRegistry.sys") == 0)
						{
							strcpy(status, "Status: xRegistry.sys RESTORE SUCCESSFUL - PLEASE REBOOT");
						}
						else
						{
							strcpy(status, "Status: xRegistry.sys RESTORE FAILED");
						}
						
						sleep(1);
					}
					if(paddata.BTN_SELECT)
					{
						if(copyfile("/dev_flash2/etc/xRegistry.sys", "/dev_usb000/xRegistry.sys") == 0)
						{
							strcpy(status, "Status: xRegistry.sys BACKUP SUCCESSFULL");
						}
						else
						{
							strcpy(status, "Status: xRegistry.sys BACKUP FAILED");
						}
						if(copyfile("/dev_flash2/etc/xRegistry.sys", "/dev_usb000/xRegistry.sys") == 0)
						{
							strcpy(status, "Status: xRegistry.sys BACKUP SUCCESSFULL");
						}
						else
						{
							strcpy(status, "Status: xRegistry.sys BACKUP FAILED");
						}
						
						sleep(1);
					}
	
		
			if(is_mounted != 0)
				showmessage(mdialogok, (lv2FsUnmount(MOUNT_POINT) == 0) ? "Successfully unmounted dev_wflash." : "An error occured while unmounting dev_wflash.");
		}
		//*/

	
	return 0;
}
