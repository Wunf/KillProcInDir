Section "Uninstall"
	ReadRegStr $1 HKLM "${PRODUCT_DIR_REGKEY}" "Install_Dir"
	KillProcInDir::KillProcInDir $1
	RMDir /r $1
SectionEnd
