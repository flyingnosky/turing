function : check perssion, open file and prepare to exec
do_open_execat(int fd, struct filename *name, int flags)
	AT_SYMLINK_NOFOLLOW --------LOOKUP_FOLLOW=0
	AT_EMPTY_PATH---------------LOOKUP_EMPTY=1
	do_filp_open
		open_namei(fd,name,flags,&nd) ---------------open whole path, and save the entry and vfsmount to nameidata
		nameidata_to_filp(&nd,flags) ----------------init a file object according to nameidata
	S_ISREG(file_inod(file)) ??
	//judge whether mount
	deny_write_access(file)  ----------------------------deny write before exec
