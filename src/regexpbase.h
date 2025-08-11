
struct RegexpBase {
	struct Library reb_LibNode;
	struct ExecBase *reb_SysBase;
	struct DosLibrary *reb_DOSBase;
	APTR *reb_SegList;
};
