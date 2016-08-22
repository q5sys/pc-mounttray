#ifndef _PCBSD_SYSTEM_CONFIG_DEVICE_DATABASE_H
#define _PCBSD_SYSTEM_CONFIG_DEVICE_DATABASE_H

#include <QString>
#include <QStringList>
#include <QFile>

class DEVDB{
public:
	static QStringList deviceFilter(){
	  QStringList filter;
	    filter << "da*" << "ad*" << "mmcsd*" << "cd*" << "acd*" << "md*";
	  return filter;
	}
	static QStringList invalidDeviceList(){
	  //List any built-in or invalid devices that still fall into the filters above
	  QStringList out;
	    out << "mdctl";
	  return out;
	}
	static QString deviceTypeByNode(QString node){
	  //Make sure each device filter above has a corresponding entry here
	  if(node.startsWith("da")){ return "USB"; }
	  else if(node.startsWith("ad")){ return "SATA"; }
	  else if(node.startsWith("mmcsd")){ return "SD"; }
	  else if(node.startsWith("cd")){ return "CD"; }
	  else if(node.startsWith("acd")){ return "CD"; }
	  else if(node.startsWith("md")){ return "ISO"; }
	  else{ return ""; }
	}
	
	static QStringList knownFilesystems(){
	  QStringList list;
	  list << "FAT" << "NTFS" << "EXFAT" << "EXT" << "EXT4" << "CD9660" \
		<< "UFS" << "REISERFS" << "XFS" << "UDF" << "ZFS" << "MTPFS";
	  return list;
	}
	
	static QStringList disktypeDetectionStrings(){
	  //Format Note: <String>::::<Filesystem> where <filesystem> matches one that is known
	  QStringList list;	
	  list << "FAT::::FAT" << "NTFS::::NTFS" << "EXT::::EXT" << "ISO9660::::CD9660" \
		<< "UFS::::UFS" << "Reiser::::REISERFS" << "XFS::::XFS" << "UDF::::UDF" << "ZFS::::ZFS";
	  return list;
	}
	
	static QStringList MountCmdsForFS(QString fs, bool useLocale){
	  //Returns: Commands to run with "%1" in place of the device path (/dev/da0)
	  //  and "%2" in place of the mountpoint path (/media/myusb)
	  // (optional) "%2a" for mountpoint base dir, "%2b" for mountpoint name
	  //  and "%3" for the LANG CODE placeholder (useLocale = false for "en_US" locale)
	  //  and "%4" for the UID of the current user
	  //  and "%5" for the GID of the operator group

	  fs = fs.toLower();
	  QStringList cmds;
	  if(fs=="fat"){ 
	    if(useLocale){ cmds << "mount -t msdosfs -o large,longnames,-m=777,-L=%3,-u=%4,-g=%5 %1 %2"; }
	    else{ cmds << "mount -t msdosfs -o large,longnames,-m=777,-u=%4,-g=%5 %1 %2"; }
          }else if(fs=="exfat"){ cmds << "mount.exfat-fuse -o uid=%4,gid=%5 %1 %2"; }
	  else if(fs=="ntfs"){ 
	    if(useLocale){ cmds << "ntfs-3g -o permissions,allow_other,locale=%3 %1 %2"; }
	    else{ cmds << "ntfs-3g -o permissions,allow_other %1 %2"; }
          }else if(fs=="ext"){ cmds << "mount -t ext2fs %1 %2"; }
	  else if(fs=="ext4"){ cmds << "ext4fuse %1 %2"; }
	  else if(fs=="cd9660"){ cmds << "mount -t cd9660 %1 %2"; }
	  else if(fs=="ufs"){ cmds << "mount -t ufs %1 %2"; }
	  else if(fs=="reiserfs"){ cmds << "mount -t reiserfs %1 %2"; }
	  else if(fs=="xfs"){ cmds << "mount -t xfs %1 %2"; }
	  else if(fs=="udf"){ cmds << "mount -t udf %1 %2"; }
	  else if(fs=="zfs"){ cmds << "zpool import -R %2a %1 %2b"; }
	  else if(fs=="mtpfs"){ cmds << "simple-mtpfs %1 %2 -o allow_other"; }
	  return cmds;
	}
	
	static QStringList UnmountCmdsForFS(QString fs, bool force){
	  //Returns: Commands to run to unmount the device (same field codes as mounting above)
	  fs = fs.toLower();
	  QStringList cmds;
	  
	  if(fs=="zfs"){
	    cmds << "zpool export %2b"; //only uses the pool name
	  }else{
	    //All Other FS's at the moment just need to run "umount"
	    if(force){ cmds << "umount -f %2"; }
	    else{ cmds << "umount %2"; }
          }
	  return cmds;
	}
	
	static bool isFSSupported(QString fs){
	  //Returns whether the utilities to mount/use the filesystem are currently available
	  fs = fs.toLower();
	  QString cmd;
	  if(fs=="fat"){ cmd = "/sbin/mount_msdosfs"; }
	  else if(fs=="exfat"){ cmd = "/usr/local/bin/mount.exfat-fuse"; }
	  else if(fs=="ntfs"){ cmd = "/usr/local/bin/ntfs-3g"; }
	  else if(fs=="ext"){ cmd = "/sbin/mount"; } //needs to be enhanced later
	  else if(fs=="ext4"){ cmd = "/usr/local/bin/ext4fuse"; }
	  else if(fs=="cd9660"){ cmd = "/sbin/mount_cd9660"; }
	  else if(fs=="ufs"){ cmd = "/sbin/mount"; } //UFS support built-in by default
	  else if(fs=="reiserfs"){ cmd = "/sbin/mount"; } //needs to be enhanced later
	  else if(fs=="xfs"){ cmd = "/sbin/mount"; } //needs to be enhanced later
	  else if(fs=="udf"){ cmd = "/sbin/mount_udf"; }
	  else if(fs=="zfs"){ cmd = "/sbin/zpool"; }
	  else if(fs=="mtpfs"){ cmd = "/usr/local/bin/simple-mtpfs"; }
	  if(cmd.isEmpty()){ return false; }
	  else{ return QFile::exists(cmd); }
	}
};

#endif