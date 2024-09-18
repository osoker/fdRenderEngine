//=======================================================================================================
/*!  \file
*   Filename: bugCommit.c
*   Desc:     these functions is initialize the bug commit data and construct the dialog of committing 
*				dialog...
*
*   His:      tzz created @ 2009-1-14
*
*  
*/
//=======================================================================================================

# include "stdafx.h"

# include <Commdlg.h>
# include "./include/sgdebug.h"
# include "bugCommit.h"
# include "Resource.h"




HANDLE g_hModule = NULL;

#ifdef _BUG_COMMITTER

BugManage		g_bugMgr;

std::string		g_strFixInterpret;

//! the title of fix interpret dialog
std::string		g_strDlgTitle;



#pragma comment(lib,"comctl32.lib")

static BOOL g_bugSortAscend[BugListDlg::scm_iMaxColumn] = 
{
	0
};

inline void ForceWrite(void* _p,size_t _size,size_t _i,FILE* _file)
{
	const size_t t_wrote = fwrite(_p,_size,_i,_file);

	if(t_wrote != _i && _size != 0){

		::MessageBox(NULL,"写入文件出错！！！马上用SVN恢复到以前的版本","严重错误！",MB_OK);

	}
}

inline void ForceRead(void* _p,size_t _size,size_t _i ,FILE* _file)
{
	const size_t t_read = fread(_p,_size,_i,_file);

	if(t_read != _i && _size != 0){

		::MessageBox(NULL,"读入文件出错！！！马上用SVN恢复到以前的版本","严重错误！",MB_OK);

	}
}
void DisplayStringByNotePad(const std::string& _string)
{
	guard;

	using namespace std;

	const std::string& t_tmpDir = getenv("TEMP");

	string t_pathName ;

	if(t_tmpDir.size()){
		t_pathName = t_tmpDir + "\\" + "tmpLog.txt";
	}else{
		t_pathName = ".\\bug\\tmpLog.txt";
	}

	ofstream t_file(t_pathName.c_str());

	std::ostreambuf_iterator<char> t_beg(t_file);
	copy(_string.begin(),_string.end(),t_beg);
	t_file.close();

	string t_cmd = "notepad " + t_pathName;
	system(t_cmd.c_str());
	

	unguard;
}

BOOL FindBugFixByKeyWord(const Bug* _bug,const std::string _keyWord)
{
	guard;

	if(_keyWord.size() == 0){
		return TRUE;
	}

	if(_bug->GetBugName().find(_keyWord) != -1){
		return TRUE;
	}

	if(_bug->GetLogContain().find(_keyWord) != -1){
		return TRUE;
	}

	if(_bug->GetDesc().find(_keyWord) != -1){
		return TRUE;
	}

	return FALSE;

	unguard;
}
int CompareTime(const SYSTEMTIME& _a ,const SYSTEMTIME& _b)
{
	guard;

	if(_a.wYear > _b.wYear){
		return 1;
	}else if(_a.wYear > _b.wYear){
		return -1;
	}

	if(_a.wMonth > _b.wMonth){
		return 1;
	}else if(_a.wMonth < _b.wMonth){
		return -1;
	}

	if(_a.wDay > _b.wDay){
		return 1;
	}else if(_a.wDay < _b.wDay){
		return -1;
	}

	if(_a.wHour > _b.wHour){
		return 1;
	}else if(_a.wHour < _b.wHour){
		return -1;
	}

	if(_a.wMinute > _b.wMinute){
		return 1;
	}else if(_a.wMinute < _b.wMinute){
		return -1;
	}

	if(_a.wSecond > _b.wSecond){
		return 1;
	}else if(_a.wSecond < _b.wSecond){
		return -1;
	}

	return 0;
	unguard;
}

int CALLBACK CompareFunc(LPARAM _lParam1,LPARAM _lParam2,LPARAM _lParamSort)
{
	guard;
	
	Bug* t_pBug1	= ((BugListDlg::ItemParam*) _lParam1)->m_bug;
	Bug* t_pBug2	= ((BugListDlg::ItemParam*) _lParam2)->m_bug;

	Fix* t_pFix1	= ((BugListDlg::ItemParam*) _lParam1)->m_fix;
	Fix* t_pFix2	= ((BugListDlg::ItemParam*) _lParam2)->m_fix;

	int t_bResult = 0;

	switch(_lParamSort){

		case 0: //! the  bug owner sort
			
 			if(g_bugSortAscend[0]){
 				t_bResult = t_pBug1->GetBugOwner() <= t_pBug2->GetBugOwner();
			}else{
				t_bResult = t_pBug1->GetBugOwner() > t_pBug2->GetBugOwner();
			}

			break;
		case 1: //! the data of bug
			t_bResult = CompareTime(t_pBug1->GetBugTime(),t_pBug2->GetBugTime());
			if(g_bugSortAscend[1]){
				t_bResult = -t_bResult;
			}
			break;
		case 2: //! the bug name....
			break;
		case 3: //! the bug severity
			t_bResult = (t_pBug1->GetSeverity() - t_pBug2->GetSeverity());
 			if(g_bugSortAscend[3]){
 				t_bResult = -t_bResult;
 			}
			break;
		case 4:
			if(t_pFix1 == NULL){
				if(t_pFix2 == NULL){
					t_bResult = 0;
				}else{
					t_bResult = 1;
				}
			}else{
				if(t_pFix2 == NULL){
					t_bResult = -1;
				}else{

					t_bResult = t_pFix1->GetBugState() - t_pFix2->GetBugState();

				}
			}

			if(g_bugSortAscend[4]){
				t_bResult = -t_bResult;
			}
			break;
	
	}
	
	

	return t_bResult;
	
	unguard;
}

inline BOOL IsFileExist(const char* _szFilename)
{

	WIN32_FIND_DATA t_findData;
	ZeroMemory(&t_findData,sizeof(t_findData));

	HANDLE t_file = ::FindFirstFile(_szFilename,&t_findData);

	BOOL t_bResult = FALSE;

	if(t_file != INVALID_HANDLE_VALUE

	&& !t_findData.nFileSizeHigh										// is small than the 4GB

	&& !(t_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){		// is not directory

		t_bResult = TRUE;
	}

	::FindClose(t_file);

	return t_bResult;

}

void WriteString(const std::string& _string,FILE* _file)
{
	guard;

	osassert(_file);

	size_t t_dwLen = _string.length();

	ForceWrite(&t_dwLen,sizeof(size_t),1,_file);
	ForceWrite((void*)_string.c_str(),t_dwLen,1,_file);
    

	unguard;
}
void ReadString(std::string& _string,FILE* _file)
{	
	guard;

	osassert(_file)	;

	size_t t_dwLen = 0;
	ForceRead(&t_dwLen,sizeof(t_dwLen),1,_file);
	char* t_szTmp = new char[t_dwLen + 1 ];
	ForceRead(t_szTmp,t_dwLen,1,_file);
	t_szTmp[t_dwLen] = 0;

	_string = t_szTmp;

	delete t_szTmp;

	unguard;
}
//! bug manage dialog callback function...
BOOL CALLBACK BugManageDlgProc(HWND _hDlg, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	guard;

	g_bugMgr.MainMsgProcesure(_msg,_wParam,_lParam);	


	return FALSE;


	unguard;
}

//! the bug commit callback function
BOOL CALLBACK BugCommitDlgProc(HWND _hDlg, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	guard;

	g_bugMgr.AddBugMsgProc(_msg,_wParam,_lParam);

	return FALSE;

	unguard;
}


//! the bug list dialog message proc
BOOL CALLBACK BugListDlgProc(HWND _hDlg,UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	guard;

	g_bugMgr.BugListMsgProc(_msg,_wParam,_lParam);

	return FALSE;

	unguard;
}

//! the fix interpret dialog box callback function( modal dialog)
BOOL CALLBACK FixInterpreDlg(HWND _hDlg,UINT _msg,WPARAM _wParam, LPARAM _lParam)
{
	guard;

	BOOL t_bResult = FALSE;

	char* t_strInterpret = NULL;
	DWORD t_dwStrLen = 0;
	switch(_msg){
		case WM_INITDIALOG:
			::SetWindowTextA(_hDlg,g_strDlgTitle.c_str());
			::SetFocus(::GetDlgItem(_hDlg,IDCANCEL));
			break;
		case WM_COMMAND:
			
			switch(LOWORD(_wParam)){

				case IDOK:

					//@{ get the edit control text
					t_dwStrLen = Edit_GetTextLength(::GetDlgItem(_hDlg,IDC_INTERPRET));
					t_strInterpret = new char[t_dwStrLen + 1];
					Edit_GetText(::GetDlgItem(_hDlg,IDC_INTERPRET),t_strInterpret,t_dwStrLen + 1);
					g_strFixInterpret = t_strInterpret;
					delete t_strInterpret;				
					//@}

					EndDialog(_hDlg,1);

					break;
				case IDCANCEL:
					EndDialog(_hDlg,0);
					t_bResult = TRUE;
					break;
			}
			
		
	
	}

	return t_bResult;

	unguard;
}

//! popup the bug commit dialog and (non-modal dialog) prepare a bug
int PopupBugCommitDlg(void)
{
	guard;	
	g_bugMgr.ShowDlg(TRUE);	

	return 0;
	unguard;
}
//====================================================================================================================
//
//	BugManage class to manage the whole bug commit system
//
//====================================================================================================================


//! instance of whole programme to read the resource
HINSTANCE		BugManage::sm_instance  = NULL;

BugManage::BugManage(void):m_currBugListFile(NULL),
							m_currFixListFile(NULL)
{
	m_szUserName[0]		= 0;
	m_szComputerName[0] = 0;
}
BugManage::~BugManage(void)
{
	if(m_currBugListFile){
		::fclose(m_currBugListFile);
	}
	if(m_currFixListFile){
		::fclose(m_currFixListFile);
	}
	m_currFixListFile = m_currBugListFile = NULL;
}


void BugManage::InitCommonCtrl(void)
{
	guard;

	// must initialize the extend common lib 
	//
	InitCommonControls();

	INITCOMMONCONTROLSEX   t_ctrlex;   
	t_ctrlex.dwSize   =   sizeof(   INITCOMMONCONTROLSEX   );   
	t_ctrlex.dwICC   =   ICC_LISTVIEW_CLASSES;   

	InitCommonControlsEx(   &t_ctrlex   );   

	unguard;
}

void BugManage::CreateDlg(HINSTANCE _ins)
{
	guard;

	sm_instance = _ins;

	m_hBugCommitDialog = ::CreateDialog((HINSTANCE)_ins,MAKEINTRESOURCE(IDD_BUG_COMMIT),NULL,(DLGPROC)BugManageDlgProc);
	::ShowWindow(m_hBugCommitDialog,SW_HIDE);

	InitInstance(_ins);
	
	unguard;
}

void BugManage::ShowDlg(BOOL _bShow)
{
	guard;

	osassert(::IsWindow(m_hBugCommitDialog));

	if(_bShow){
		::ShowWindow(m_hBugCommitDialog,SW_SHOW);
	}else{
		::ShowWindow(m_hBugCommitDialog,SW_HIDE);
	}

	unguard;
}

void BugManage::InitInstance(HINSTANCE _ins)
{
	guard;

	osassert(_ins);
	osassert(m_hBugCommitDialog);

	DWORD t_size = (DWORD)scm_iUserNameLen;
	if(!::GetUserName(m_szUserName,&t_size)){

		::MessageBox(NULL,"无法获得当前用户名，无法进行bug提交","Error",MB_OK);

		return;
	}

	t_size = scm_iComputerNameLen;
	if(!::GetComputerName(m_szComputerName,&t_size)){
		::MessageBox(NULL,"无法获得当前计算机名，无法进行bug提交","Error",MB_OK);

		return; 
	}

	// make a dir
	//
	char t_szDirName[scm_iUserNameLen + scm_iComputerNameLen + sizeof(".\\bug\\ @ ") ];
	sprintf(t_szDirName,".\\bug\\%s @ %s",m_szUserName,m_szComputerName);
	::_mkdir(".\\bug");
	::_mkdir(t_szDirName);

	// set the name of main dialog to know the name of QA
	//
	char t_szTileName[128 + 32 + sizeof("Bug提交< @ >") + 1] ;
	sprintf(t_szTileName,"Bug提交<%s @ %s> 按 F1 查看帮助文档",m_szUserName,m_szComputerName);
	::SetWindowTextA(m_hBugCommitDialog,t_szTileName);

	m_addBugDlg.InitDialog(_ins,this,IDD_ADD_BUG);
	m_bugListDlg.InitDialog(_ins,this,IDD_BUG_LIST);


	sprintf(t_szTileName,"%s @ %s",m_szUserName,m_szComputerName);
	m_bugListDlg.InsertBugOwnerFilterString(t_szTileName);


	m_tab.IniTab(m_hBugCommitDialog,IDC_TAB1);
	m_tab.AddPage(m_addBugDlg,"Bug提交");
	m_tab.AddPage(m_bugListDlg,"Bug列表");

	m_tab.ShowDialog(1);

	// open (or create )a bug and fix file
	//
	OpenBugFixFile();


	unguard;
}

void BugManage::MainMsgProcesure(UINT _msg,WPARAM _wParam ,LPARAM _lParam)
{
	guard;

	switch(_msg){

		case WM_INITDIALOG:
			
			break;
		case WM_COMMAND:

			switch(LOWORD(_wParam)){
				case IDCANCEL:

					::ShowWindow(*this,SW_HIDE);

					break;
				
			}			
			break;
		case msgCommitBug:
			CommitBug();
			break;
		default:
			break;
	}

	m_tab.MsgProc(m_hBugCommitDialog,_msg,_wParam,_lParam);


	if(::GetAsyncKeyState(VK_F1) & 0x8000){
		ShowHelpDlg();
	}

	unguard;
}

void BugManage::CommitBug(void)
{
	guard;

	BugList* t_list = m_bugListDlg.GetBugList();

	t_list->AddBug(m_addBugDlg.GetTempBug());

	m_bugListDlg.RefreshListView();

	m_currUserBugList.AddBug(m_addBugDlg.GetTempBug(),m_currBugListFile,scm_dwBugSysVersion);

	unguard;
}

void BugManage::OpenBugFixFile(void)
{
	guard;
	
	osassert(m_szComputerName[0] && m_szUserName[0]);

	// open the 
	//
	std::string t_dir ;
	t_dir += ".\\bug\\";
	t_dir += m_szUserName;
	t_dir += " @ " ;
	t_dir += m_szComputerName;

	// read (create the bug list)
	//
	BOOL t_bFileIsExist = IsFileExist((t_dir + "\\bug.list").c_str());

	if(!t_bFileIsExist){
		m_currBugListFile = fopen((t_dir + "\\bug.list").c_str(),"wb");
		fclose(m_currBugListFile);
	}

	m_currBugListFile  = fopen((t_dir + "\\bug.list").c_str(),"rb+");
	osassert(m_currBugListFile);

	if(t_bFileIsExist){

		BugList* t_ptrList = m_bugListDlg.GetBugList();

		BOOL t_bResult = t_ptrList->ReadFromFile(m_currBugListFile)
			&& m_currUserBugList.ReadFromFile(m_currBugListFile);

		if(!t_bResult ){

			::MessageBox(*this,"读入本地Bug文件有错误!","Error",MB_OK);
		}	



	}

	// read (create the fix list)
	//
	t_bFileIsExist = IsFileExist((t_dir + "\\fix.list").c_str());

	if(!t_bFileIsExist){
		m_currFixListFile = fopen((t_dir + "\\fix.list").c_str(),"wb");
		fclose(m_currFixListFile);

	}
	m_currFixListFile = fopen((t_dir + "\\fix.list").c_str(),"rb+");
	osassert(m_currFixListFile);

	if(t_bFileIsExist){

		FixList* t_ptrList = m_bugListDlg.GetFixList();

		BOOL t_bResult = t_ptrList->ReadFromFile(m_currFixListFile)
			&& m_currUserFixList.ReadFromFile(m_currFixListFile);

		if(!t_bResult ){

			::MessageBox(*this,"读入本地Fix文件有错误!","Error",MB_OK);
		}	
	}


	// read the other bug list and fix list file
	//
	WIN32_FIND_DATA t_findData;
	HANDLE t_hFindFile = ::FindFirstFile(".\\bug\\*.*",&t_findData);

	osassert(t_hFindFile != INVALID_HANDLE_VALUE);
	
	std::string t_findDir;

	do{
		t_findDir = t_findData.cFileName;

		std::string t_openFile = ".\\bug\\" + t_findDir;


		if(t_findDir == "."
		|| t_findDir == ".."
		|| t_findDir == ".svn"
		|| t_openFile == t_dir){
			continue;
		}

		if(t_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){

			// is  a directory
			//
			const size_t t_flagPos = t_findDir.find("@");

			if( t_flagPos == -1){

				continue;

			}else{
				
				FILE* t_hList = fopen((t_openFile + "\\bug.list").c_str(),"rb");

				if(t_hList){
						
					BugList t_bugList;

					t_bugList.ReadFromFile(t_hList);

					BugList* t_pDisplayBugList = m_bugListDlg.GetBugList();

					t_bugList.MoveAllBugToOtherList(*t_pDisplayBugList);

					m_bugListDlg.InsertBugOwnerFilterString(t_findData.cFileName);

					::fclose(t_hList);

				}

				t_hList = fopen( (t_openFile + "\\fix.list").c_str(),"rb");

				if(t_hList){

					FixList t_fixList;

					t_fixList.ReadFromFile(t_hList);

					FixList* t_pDisplayFixBugList = m_bugListDlg.GetFixList();

					t_fixList.MoveAllFixToAnother(*t_pDisplayFixBugList);

					::fclose(t_hList);
				}
			}

		}

	}while(::FindNextFile(t_hFindFile,&t_findData));

	::FindClose(t_hFindFile);

	m_bugListDlg.RefreshListView(); 

	unguard;
}

void BugManage::ShowHelpDlg(void)
{
	guard;

	const std::string& t_helpString = 	 "每一个bug的提交人都是和本地计算机名 + 用户名相关，他会自动在工作目录下生成一个\n"
										"bug目录，同时再根据提交人的名字生成相关的子目录，子目录下有两个文件:\n"
										"【bug.list】bug列表\n"
										"【 fix.list】fix列表\n"
										"两个文件首次运行时没有加入到SVN上面的，并且都是空的，没有任何内容，两个列表中fix\n"
										"列表可以减小，但是bug列表不能减少，意味着bug提交后不能删除,这一点十分重要。\n\n"
										"bug提交后会自动保存，但是不会提交到SVN上面，这个时候需要提交人手动将其 【增加】 到SVN\n"
										"但是如果已经增加，只要 【提交】 就可以了\n\n"
										"每一个bug都有一个提交人，提交人自己不能修正这个bug，原因是如果bug被能够修正的人发现，\n"
										"没有必要在提交了，每一个人可以标记别人的所提交的bug，标记状态有下面几种:\n\n"
										"\tFixed<修正待检查>：\n"
										"\t\t修正者（程序员，美工，设计人员）,发现已经提交的bug后，修正过了，\n"
										"\t\t正在等待提交人验证。\n\n"
										"\tFixed OK<修正完毕>: \n"
										"\t\t提交人发现这个bug已经修正好了，将其标记，这个时候Bug就完整的结束了\n\n"
										"\tNot A Bug<这不是一个bug>:\n"
										"\t\t修正者（美工，设计人员）发现提交上来的bug不是真正意\n"
										"\t\t义上的bug。但是如果提交人不同意，需要向修正者沟通，交流如何处理这个bug。\n\n"
										"\tWill Not Fix<不修正这个Bug>: \n"
										"\t\t修正者（设计人员）放弃这个bug的修改，或是忽略这个bug\n"
										"\t\t如果提交人不同意，需要与修正者进行沟通，它不是bug终结点\n\n"
										"在 bug列表 窗口里面右键点击最左边的提交人名称，出来的菜单中就可以对bug的状态进行设置"


										;

	DisplayStringByNotePad(t_helpString);

	unguard;
}

void BugManage::AddFixToUserFixFile(const Fix& _fix)
{
	guard;

	m_currUserFixList.AddFix(_fix,m_currFixListFile,scm_dwBugSysVersion);

	unguard;
}
//====================================================================================================================
//
//	BugTab class
//
//====================================================================================================================
//! tab control class
BugTab::BugTab(void):m_hTab(NULL),
					m_uTabID(0),
					m_iShowPageIndex(0)
{
	
}
BugTab::~BugTab(void)
{

}

//! initialize the tab
void BugTab::IniTab(HWND _hParentWnd,UINT _uTabID)
{
	guard;

	osassert(!m_hTab);

	m_hTab = ::GetDlgItem(_hParentWnd,_uTabID);
	m_uTabID = _uTabID;



	unguard;
}
//! initialize the tab
void BugTab::AddPage(HWND _hDialogWnd,const char* _szDialogName)
{
	guard;

	osassert(_hDialogWnd);
	osassert(::IsWindow(_hDialogWnd));

	InsertItem((int)m_vPageWnd.size(),_szDialogName);

	m_vPageWnd.push_back(_hDialogWnd);

	// set the dialog position by the tab control's pos
	RefreshPagePos((int)m_vPageWnd.size() - 1 );


	unguard;
}

//! set the dialog(index) position by the tab control
void BugTab::RefreshPagePos(int _iPageIndex)
{
	guard;
	
	osassert(_iPageIndex >= 0 && _iPageIndex < (int)m_vPageWnd.size());
	osassert(::IsWindow(m_hTab));

	HWND t_page = m_vPageWnd[_iPageIndex];

	RECT t_tabRect ; ZeroMemory(&t_tabRect,sizeof(RECT));
	RECT t_mainRect; ZeroMemory(&t_mainRect,sizeof(RECT));

	::GetWindowRect(m_hTab,& t_tabRect);
	::GetWindowRect(::GetParent(m_hTab),&t_mainRect);

	const LONG t_x = t_tabRect.left - t_mainRect.left - ::GetSystemMetrics(SM_CYEDGE);
	const LONG t_y = t_tabRect.bottom - t_mainRect.top - ::GetSystemMetrics(SM_CYCAPTION) - 2 * ::GetSystemMetrics(SM_CXEDGE);

	::SetWindowPos(t_page,HWND_BOTTOM,t_x,t_y,0,0,SWP_NOSIZE);

	unguard;
}

//! refresh the all page dialog by the tab control
void BugTab::RefreshAllPage(void)
{
	guard;

	for(int i = 0;i< (int)m_vPageWnd.size();i++){

		RefreshPagePos(i);

	}

	unguard;
}

//! message procedure
BOOL BugTab::MsgProc(HWND _hParentWnd,UINT _msg,WPARAM _wParam,LPARAM _lParam)
{
	guard;
	
	BOOL t_bProcessed = FALSE;

	switch(_msg){
		case WM_INITDIALOG:
			RefreshAllPage();
			break;

		case WM_NOTIFY:
			if(_wParam == m_uTabID){
				ClickedNotify(_hParentWnd ,_lParam);
				t_bProcessed = TRUE;
			
			}		
			break;
		default:

			break;

	}
	

	return t_bProcessed;

	unguard;
}
//! show or hide the dialog by the index
void BugTab::ShowDialog(size_t _iPageIndex)
{
	guard;

	if(_iPageIndex >= m_vPageWnd.size() || _iPageIndex < 0) return;

	TabCtrl_SetCurSel(*this,_iPageIndex);

	for(size_t i =0;i < m_vPageWnd.size();i++){

		if(i ==  _iPageIndex){
			::ShowWindow(m_vPageWnd[i],SW_SHOW);
		}else{
			::ShowWindow(m_vPageWnd[i],SW_HIDE);
		}
	}

	m_iShowPageIndex = _iPageIndex;

	unguard;
}
//! notify message tab
void BugTab::ClickedNotify(HWND _hParentWnd,LPARAM _lParam)
{
	guard;

	const int t_curSel = GetCurSel();

	ShowDialog(t_curSel);


	unguard;
}

//! severity string and note(interpret value)
const std::string BugSeverityString::CantTest_A("不能测试(A)");
const std::string BugSeverityString::CantTest_A_note("这种bug极其少见，主要是执行文件和资源文件版本不一致，导致一开始载入资源的时候就会出现错误，这个"
													 "时候有可能是没有assert文件，可能会有dump文件，如果出现这种bug，就要与程序员进行沟通，马上解决。"
													 "这个bug的优先级最高。");

const std::string BugSeverityString::Assert_B("Assert Bug(B)");
const std::string BugSeverityString::Assert_B_note("这种bug在初期的时候会很多，常常伴随着Assert异常，产生log文件，必须将其加入到Bug里面去。这个是"
												    "一种\"不可饶恕的bug\"，也是一种严重的bug，必须优先解决。");

const std::string BugSeverityString::Major_C("Major(C)");
const std::string BugSeverityString::Major_C_note("这种Bug严重影响游戏进程、玩家情绪的逻辑bug，阻碍到游戏继续玩下去的可能，同时严重违背游戏的设计文档、"
												  "或者是很普遍的客观事实，这个应当配合较为详细的描述");

const std::string BugSeverityString::Medium_D("Medium(D)");
const std::string BugSeverityString::Medium_D_note("一般的Bug，操作不便，新手指导错误、图像、动画、声音等等，这种bug在前期很少，但是到游戏的中后期"
												   "会非常多，估计占到总游戏bug的bug一半左右，应当由详细描述进行阐述");

const std::string BugSeverityString::Minor_E("Minor(E)");
const std::string BugSeverityString::Minor_E_note("文本错误。常常是由设计人员造成的文本编辑错误，但是由于程序设计问题，导致文本显示不正常，也属于此类。");

const std::string BugSeverityString::Question_F("问题(F)");
const std::string BugSeverityString::Question_F_note("无法确定这种现象是不是bug，还是一种设计要求，将在bug描述中进行阐述。");

const std::string BugSeverityString::Suggest_G("建议(G)");
const std::string BugSeverityString::Suggest_G_note("建议如何去改善某种不方便或是自己认为不合理的地方，有自己的个人因素在里面，在描述中详细说明即可。");

const size_t BugSeverityString::scm_maxStringLenth = 32;
//! location string...
const std::string BugLocationString::ActionPhase("游戏进行中");
const std::string BugLocationString::ActionPhase_note("游戏进行中，在很多时候，只要游戏正在进行（玩家正在操作），就是这种情况。");

const std::string BugLocationString::All("全局");
const std::string BugLocationString::All_note("游戏打开到游戏关闭为止。");

const std::string BugLocationString::Documentation("文本时");
const std::string BugLocationString::Documentation_note("当出现某种文本的时候，这种对应的<严重程度>为Minor(E)");

const std::string BugLocationString::Fix("固定几处");
const std::string BugLocationString::Fix_note("在游戏固定的几处，知道有可能在哪里会出现");

const std::string BugLocationString::GameMenu("游戏菜单");
const std::string BugLocationString::GameMenu_note("游戏界面的错误，按钮的摆放不合理，菜单的设计和方便等等");

const std::string BugLocationString::Loading("读取时");
const std::string BugLocationString::Loading_note("明显正在<Loading>的时候出现的问题,这种错误常常比较严重");

const std::string BugLocationString::GameScene("游戏场景某处");
const std::string BugLocationString::GameScene_note("游戏场景的不合理，或是走到某处的时候被卡住，或是场景的显示比例等等属于此类");

const int BugLocationString::scm_maxStringLenth = 32;

//! reproduce rate
const std::string BugReproRateString::EasyReproKnowRule("容易重现，知道规则");
const std::string BugReproRateString::EasyReproUnknownRule("容易重现，不知道规则");
const std::string BugReproRateString::HardReproKnowRule("很难重现，知道规则");
const std::string BugReproRateString::HardReproUnknownRule("很难重现，不知道规则");

const int BugReproRateString::scm_maxStringLenth = 24;



//====================================================================================================================
//
//	AddBugDlg class to manage the dialog and bug data
//
//====================================================================================================================

Bug AddBugDlg::m_tmpBug;

AddBugDlg::AddBugDlg(void):m_hDlg(NULL),
							m_uDlgID(0),
							m_pCurrBug(NULL),
							m_severityList(NULL)
{
	//nothing to do now
}
AddBugDlg::~AddBugDlg(void)
{
	//nothing to do now
}

//! initialize the dialog
void AddBugDlg::InitDialog(HINSTANCE _ins,BugManage* _bugMgr , UINT _uDlgID)
{
	guard;

	osassert(::IsWindow(*_bugMgr));
	
	m_hDlg = CreateDialogA((HINSTANCE)_ins,MAKEINTRESOURCE(IDD_ADD_BUG),*_bugMgr,BugCommitDlgProc);
	osassert(::IsWindow(m_hDlg));

	::ShowWindow(m_hDlg,SW_HIDE);

 	m_severityList = ::GetDlgItem(m_hDlg,IDC_SEVERITY_LIST);
	m_severityNote = ::GetDlgItem(m_hDlg,IDC_SEVERITY_INTEPRET);

	ComboBox_AddString(m_severityList,BugSeverityString::CantTest_A.c_str());
	ComboBox_AddString(m_severityList,BugSeverityString::Assert_B.c_str());
	ComboBox_AddString(m_severityList,BugSeverityString::Major_C.c_str());	
	ComboBox_AddString(m_severityList,BugSeverityString::Medium_D.c_str());
	ComboBox_AddString(m_severityList,BugSeverityString::Minor_E.c_str());
	ComboBox_AddString(m_severityList,BugSeverityString::Question_F.c_str());	
	ComboBox_AddString(m_severityList,BugSeverityString::Suggest_G.c_str());
	
	m_locationList = ::GetDlgItem(m_hDlg,IDC_LOCATION_LIST);
	m_locationNote = ::GetDlgItem(m_hDlg,IDC_LOCATION_INTERPRET);

	ComboBox_AddString(m_locationList,BugLocationString::ActionPhase.c_str());
	ComboBox_AddString(m_locationList,BugLocationString::All.c_str());
	ComboBox_AddString(m_locationList,BugLocationString::Documentation.c_str());
	ComboBox_AddString(m_locationList,BugLocationString::Fix.c_str());
	ComboBox_AddString(m_locationList,BugLocationString::GameMenu.c_str());
	ComboBox_AddString(m_locationList,BugLocationString::Loading.c_str());
	ComboBox_AddString(m_locationList,BugLocationString::GameScene.c_str());

	m_reproList = ::GetDlgItem(m_hDlg,IDC_REPRO_LIST);

	ComboBox_AddString(m_reproList,BugReproRateString::EasyReproKnowRule.c_str());
	ComboBox_AddString(m_reproList,BugReproRateString::EasyReproUnknownRule.c_str());
	ComboBox_AddString(m_reproList,BugReproRateString::HardReproKnowRule.c_str());
	ComboBox_AddString(m_reproList,BugReproRateString::HardReproUnknownRule.c_str());
	

	m_tmpBug.NewBug((_bugMgr->GetUserName() + " @ " + _bugMgr->GetComputerName()).c_str());

	const SYSTEMTIME& t_time = m_tmpBug.GetBugTime();

	// get the bug time and set the text of control
	//
	HWND t_wnd = ::GetDlgItem(*this,IDC_BUG_TIME);
	char t_timeString[80];
	sprintf(t_timeString,"%d年%d月%d日,%d时%d分%d秒",
		t_time.wYear,t_time.wMonth,t_time.wDay,
		t_time.wHour,t_time.wMinute,t_time.wSecond);
	Static_SetText(t_wnd,t_timeString);

	// set game name
	//
	t_wnd= ::GetDlgItem(*this,IDC_GAME_NAME);
	Static_SetText(t_wnd,"Tang");

	// set the version
	//
	t_wnd = ::GetDlgItem(*this,IDC_GAME_VERSION);
	Static_SetText(t_wnd,"白盒测试版本");

	unguard;
}

//! message procedure
BOOL AddBugDlg::MsgProcedure(UINT _msg,WPARAM _wParam ,LPARAM _lParam)
{
	guard;

	BOOL t_bProcessed = FALSE;

	if(_msg == WM_COMMAND){

		if(HIWORD(_wParam) == CBN_SELCHANGE){
			
			t_bProcessed = TRUE;

			switch(LOWORD(_wParam)){
				case IDC_SEVERITY_LIST:
					SeverityListSelChange();
					break;
				case IDC_LOCATION_LIST:
					LocationListSelChange();
					break;
				case IDC_REPRO_LIST:
					ReproListSelChange();
					break;
				default:
					t_bProcessed = FALSE;
					break;
			}
			
		}else{
			switch(LOWORD(_wParam)){

				case IDC_LOG_BUTTON:
					if((::GetAsyncKeyState(VK_CONTROL) & 0x8000 )&& m_tmpBug.GetLogFilename().size()){

						OnShowLogContain();

					}else{

						OnBugLogFile();
					}
					
					break;
				case IDC_ADD_STEP:
					OnAddStep(FALSE);
					break;
				case IDC_INSERT_STEP:
					OnAddStep(TRUE);
					break;
				case IDC_DEL_STEP:
					OnDelStep();
					break;
				case IDC_COMMIT_BUG:
					OnCommitBug();
					break;
			}
		}
	}

	return t_bProcessed;

	unguard;
}


void AddBugDlg::SeverityListSelChange(void)
{
	guard;
	
	osassert(::IsWindow(m_severityList));
	osassert(::IsWindow(m_severityNote));

	
	const int t_iSelIndex = ComboBox_GetCurSel(m_severityList);

	char t_szSelString[BugSeverityString::scm_maxStringLenth + 1];

	ComboBox_GetLBText(m_severityList,t_iSelIndex,t_szSelString);

	const std::string* t_note = NULL;
	BOOL t_bLogButton = FALSE;

	if(BugSeverityString::Assert_B == t_szSelString){
		t_bLogButton = TRUE;
		t_note = &BugSeverityString::Assert_B_note;
		m_tmpBug.SetSeverity(Bug::e_assert_B);
	}else if(BugSeverityString::CantTest_A ==t_szSelString){
		t_note = &BugSeverityString::CantTest_A_note;
		m_tmpBug.SetSeverity(Bug::e_cantTest_A);
	}else if(BugSeverityString::Major_C == t_szSelString){
		t_note = &BugSeverityString::Major_C_note;
		m_tmpBug.SetSeverity(Bug::e_major_C);
	}else if(BugSeverityString::Medium_D == t_szSelString){
		t_note = &BugSeverityString::Medium_D_note;
		m_tmpBug.SetSeverity(Bug::e_medium_D);
	}else if(BugSeverityString::Minor_E == t_szSelString){
		t_note = &BugSeverityString::Minor_E_note;
		m_tmpBug.SetSeverity(Bug::e_minor_E);
	}else if(BugSeverityString::Question_F == t_szSelString){
		t_note = &BugSeverityString::Question_F_note;
		m_tmpBug.SetSeverity(Bug::e_question_F);
	}else if(BugSeverityString::Suggest_G == t_szSelString){
		t_note = &BugSeverityString::Suggest_G_note;
		m_tmpBug.SetSeverity(Bug::e_suggest_G);
	}

	osassert(t_note);
	Static_SetText(m_severityNote,t_note->c_str());
	::EnableWindow(::GetDlgItem(*(this),IDC_LOG_BUTTON),t_bLogButton);

	if(!t_bLogButton){
		m_tmpBug.SetLogFilename("");
		m_tmpBug.SetLogContain("");
	}

	unguard;
}


void AddBugDlg::LocationListSelChange(void)
{
	guard;

	osassert(::IsWindow(m_locationList));
	osassert(::IsWindow(m_locationNote));
	
	const int t_iSelIndex = ComboBox_GetCurSel(m_locationList);

	char t_szSelString[BugLocationString::scm_maxStringLenth + 1];

	ComboBox_GetLBText(m_locationList,t_iSelIndex,t_szSelString);

	const std::string* t_note = NULL;

	if(BugLocationString::ActionPhase == t_szSelString){
		t_note = &BugLocationString::ActionPhase_note;
		m_tmpBug.SetLocation(Bug::e_actionPhase);
	}else if(BugLocationString::All == t_szSelString){
		t_note = &BugLocationString::All_note;
		m_tmpBug.SetLocation(Bug::e_all);
	}else if(BugLocationString::Documentation == t_szSelString){
		t_note = &BugLocationString::Documentation_note;
		m_tmpBug.SetLocation(Bug::e_documentation);
	}else if(BugLocationString::Fix == t_szSelString){
		t_note = &BugLocationString::Fix_note;
		m_tmpBug.SetLocation(Bug::e_fixLocation);
	}else if(BugLocationString::GameMenu == t_szSelString){
		t_note = &BugLocationString::GameMenu_note;
		m_tmpBug.SetLocation(Bug::e_gameMenu);
	}else if(BugLocationString::GameScene == t_szSelString){
		t_note = &BugLocationString::GameScene_note;
		m_tmpBug.SetLocation(Bug::e_gameScene);
	}else if(BugLocationString::Loading == t_szSelString){
		t_note = &BugLocationString::Loading_note;
		m_tmpBug.SetLocation(Bug::e_loading);
	}

	osassert(t_note);
	Static_SetText(m_locationNote,t_note->c_str());
	

	unguard;
}
void AddBugDlg::ReproListSelChange(void)
{
	guard;

	osassert(::IsWindow(m_reproList));

	const int t_iSelIndex = ComboBox_GetCurSel(m_reproList);

	char t_szSelString[BugReproRateString::scm_maxStringLenth + 1];

	ComboBox_GetLBText(m_reproList,t_iSelIndex,t_szSelString);

	if(BugReproRateString::EasyReproKnowRule == t_szSelString){
		m_tmpBug.SetReproRate(Bug::e_easyRepro_knowRule);
	}else if(BugReproRateString::EasyReproUnknownRule == t_szSelString){
		m_tmpBug.SetReproRate(Bug::e_easyRepro_unknownRule);
	}else if(BugReproRateString::HardReproKnowRule == t_szSelString){
		m_tmpBug.SetReproRate(Bug::e_hardRepro_knowRule);
	}else if(BugReproRateString::HardReproUnknownRule == t_szSelString){
		m_tmpBug.SetReproRate(Bug::e_hardRepro_unknownRule);

	}

	
	unguard;
}

void AddBugDlg::OnBugLogFile(void)
{
	guard;

	char t_szFilename[MAX_PATH] = {0};
	char t_path[MAX_PATH] = {0};
	_getcwd(t_path,MAX_PATH);
	std::string t_temp(t_path);
	t_temp += ".\\log";

	OPENFILENAME ofn ;
	ZeroMemory( &ofn,sizeof( OPENFILENAME ) );

	ofn.lStructSize		= sizeof( OPENFILENAME );
	ofn.hwndOwner		= *this;
	ofn.nFilterIndex	= 1;
	ofn.lpstrFilter		= "Log Files (.txt)\0*.txt\0\0";
	ofn.lpstrFile		= t_szFilename;
	ofn.nMaxFile		= sizeof(t_szFilename);
	ofn.lpstrFileTitle	= NULL; //"保存相机轨迹文件"
	ofn.nMaxFileTitle	= 0;
	ofn.Flags			= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = t_temp.c_str();

	if( GetOpenFileNameA( &ofn ) ){

		// get the filename not full path
		//
		t_temp = t_szFilename;
		strcpy(t_szFilename,t_temp.c_str() + t_temp.rfind("\\") + 1);
		
		m_tmpBug.SetLogFilename(t_szFilename);
		m_tmpBug.SetLogContain(GetBugLogContain(t_temp.c_str()).c_str());	

		// set the button title
		strcat(t_szFilename," < 按住Ctrl可以查看log文件内容> ");
		::SetWindowText(::GetDlgItem(*this,IDC_LOG_BUTTON),t_szFilename);
	
	}

	unguard;
}

void AddBugDlg::OnShowLogContain(void)
{
	guard;

	DisplayStringByNotePad(m_tmpBug.GetLogContain());

	unguard;
}
void AddBugDlg::OnAddStep(BOOL _bInsert)
{
	guard;
	
	HWND t_stepCtrl = ::GetDlgItem(*this,IDC_STEP);
	osassert(::IsWindow(t_stepCtrl));

	const int t_stepStringLen =	Edit_GetTextLength(t_stepCtrl);

	if(t_stepStringLen){

		char* t_string = new char[t_stepStringLen + 1];

		Edit_GetText(t_stepCtrl,t_string,t_stepStringLen + 1);

		HWND t_listBox = ::GetDlgItem(*this,IDC_STEP_LIST);
		osassert(::IsWindow(t_listBox));

		if(_bInsert){
			const int t_curSel = ListBox_GetCurSel(t_listBox);

			if(t_curSel == -1){


				::MessageBox(*this,"清先选择在哪一步前面插入","提示",MB_OK);


			}else{

				m_tmpBug.GetStepVect().insert(m_tmpBug.GetStepVect().begin() + t_curSel,t_string);

				RefreshStepList(t_curSel);

				Edit_SetText(t_stepCtrl,"");

			}

		}else{

			m_tmpBug.GetStepVect().push_back(t_string);

			RefreshStepList((int)(m_tmpBug.GetStepVect().size()) -1 );

			Edit_SetText(t_stepCtrl,"");
		}
			

		delete t_string;
		

	}else{

		::MessageBox(*this,"没有写步骤，请先填写","提示",MB_OK);

	}
	

	
	unguard;
}

void AddBugDlg::RefreshStepList(int _curSel)
{
	guard;

	osassert(_curSel >= 0);

	HWND t_listBox = ::GetDlgItem(*this,IDC_STEP_LIST);
	const int t_count = ListBox_GetCount(t_listBox);

	for(int i = 0 ;i< t_count;i++){

		ListBox_DeleteString(t_listBox,0);

	}

	std::vector<std::string>& t_v = m_tmpBug.GetStepVect();

	std::string t_listString;
	for(size_t i = 0;i < t_v.size();i++  ){

		t_listString.resize(t_v[i].size() + 1 + sizeof("100000"));
		t_listString = va("%d:",i + 1) + t_v[i];

		ListBox_AddString(t_listBox,t_listString.c_str());

	}

	ListBox_SetCurSel(t_listBox,(size_t)_curSel >= t_v.size()? t_v.size() -1:_curSel);

	unguard;
}
void AddBugDlg::OnDelStep(void)
{
	guard;

	HWND t_listbox = ::GetDlgItem(*this,IDC_STEP_LIST);
	osassert(::IsWindow(t_listbox));

	const int t_curSel = ListBox_GetCurSel(t_listbox);

	if(t_curSel != -1){

		m_tmpBug.GetStepVect().erase(m_tmpBug.GetStepVect().begin() + t_curSel);

		RefreshStepList(t_curSel);
	}

	unguard;
}

void AddBugDlg::OnCommitBug(void)
{
	guard;

	
	if(m_tmpBug.GetSeverity() == Bug::e_unselected_sev){
		
		::MessageBox(*this,"请选择<严重程度>","提示",MB_OK);

		return;
	}

	if(m_tmpBug.GetSeverity() == Bug::e_assert_B && m_tmpBug.GetLogFilename() == ""){

		::MessageBox(*this,"选择了assert bug的严重程度，需要选择log文件","提示",MB_OK);

		OnBugLogFile();

		return ;

	}

	if(m_tmpBug.GetLocation() == Bug::e_unselected_loc){

		::MessageBox(*this,"请选择<出现位置>","提示",MB_OK);

		return;
	}

	if(m_tmpBug.GetReproRate() == Bug::e_unselected_repro){

		::MessageBox(*this,"请选择<重现机率>","提示",MB_OK);

		return;
	}

	HWND t_bugNameEdit = ::GetDlgItem(*this,IDC_BUG_NAME);
	osassert(::IsWindow(t_bugNameEdit));

	int t_iLen = Edit_GetTextLength(t_bugNameEdit);

	if(t_iLen == 0){

		::MessageBox(*this,"请填写bug 的名字（简单描述一下Bug)","提示",MB_OK);

		::SetFocus(t_bugNameEdit);

		return;

	}else{

		char* t_str = new char[t_iLen + 1];

		Edit_GetText(t_bugNameEdit,t_str,t_iLen + 1);

		m_tmpBug.SetBugName(t_str);

		delete t_str;

	}


	HWND t_descEdit = ::GetDlgItem(*this,IDC_BUG_DESC);
	osassert(::IsWindow(t_descEdit));

	t_iLen = Edit_GetTextLength(t_descEdit);

	if(t_iLen == 0){
		
		if(::MessageBox(*this,"真的不想填写详细描述就要提交Bug ？","提示",MB_YESNO) == IDNO){

			return;
		}
	}else{

		char* t_str = new char[t_iLen +1 ];
		Edit_GetText(t_descEdit,t_str,t_iLen+1);
		
		m_tmpBug.SetDesc(t_str);

		delete t_str;
	}
	
	if(m_tmpBug.GetStepVect().size() == 0){

		if(::MessageBox(*this,"真的不想填写步骤就要提交Bug ？","提示",MB_YESNO) == IDNO){

			return;

		}
	}
	

	std::string t_bugWholeNote;

	m_tmpBug.GetWholeBugNote(t_bugWholeNote);

	if(::MessageBox(*this,t_bugWholeNote.c_str(),"你想要确定提交这个Bug？(提交后不能删除)",MB_YESNO) == IDYES){
		
		::SendMessage(::GetParent(*this),BugManage::msgCommitBug,0,0);
		

		// clear the add bug infomation in the control
		//
		ClearBugInfo();
		
	}


	unguard;
}

void AddBugDlg::ClearBugInfo(void)
{
	guard;

	HWND t_ctrl = ::GetDlgItem(*this,IDC_BUG_NAME);
	osassert(::IsWindow(t_ctrl));

	Edit_SetText(t_ctrl,"");

	t_ctrl = ::GetDlgItem(*this,IDC_SEVERITY_LIST);osassert(::IsWindow(t_ctrl));
	
	ComboBox_SetCurSel(t_ctrl,-1);

	t_ctrl = ::GetDlgItem(*this,IDC_SEVERITY_INTERPRET);osassert(::IsWindow(t_ctrl));

	::SetWindowText(t_ctrl,"<没有选择，不能提交Bug>");

	t_ctrl = ::GetDlgItem(*this,IDC_LOCATION_LIST);osassert(::IsWindow(t_ctrl));

	ComboBox_SetCurSel(t_ctrl,-1);

	t_ctrl = ::GetDlgItem(*this,IDC_LOCATION_INTERPRET);osassert(::IsWindow(t_ctrl));

	::SetWindowText(t_ctrl,"<没有选择，不能提交Bug>");

	t_ctrl = ::GetDlgItem(*this,IDC_RELATION_BUG);osassert(::IsWindow(t_ctrl));

	::SetWindowText(t_ctrl,"<无>");

	t_ctrl = ::GetDlgItem(*this,IDC_REPRO_LIST);osassert(::IsWindow(t_ctrl));

	ComboBox_SetCurSel(t_ctrl,-1);

	t_ctrl = ::GetDlgItem(*this,IDC_LOG_BUTTON);osassert(::IsWindow(t_ctrl));

	::SetWindowText(t_ctrl,"选择出错生成的log文件");
	::EnableWindow(t_ctrl,FALSE);

	t_ctrl = ::GetDlgItem(*this,IDC_BUG_DESC);osassert(::IsWindow(t_ctrl));
	
	Edit_SetText(t_ctrl,"");

	t_ctrl = ::GetDlgItem(*this,IDC_STEP_LIST);osassert(::IsWindow(t_ctrl));
	
	while(ListBox_DeleteString(t_ctrl,0) != LB_ERR) ;

	t_ctrl = ::GetDlgItem(*this,IDC_STEP);osassert(::IsWindow(t_ctrl));

	::Edit_SetText(t_ctrl,"");
	
	std::string t_bugOwner = g_bugMgr.GetUserName();
	t_bugOwner += " @ ";
	t_bugOwner += g_bugMgr.GetComputerName();

	m_tmpBug.NewBug(t_bugOwner.c_str());

	const SYSTEMTIME& t_time = m_tmpBug.GetBugTime();

	// get the bug time and set the text of control
	//
	HWND t_wnd = ::GetDlgItem(*this,IDC_BUG_TIME);
	char t_timeString[80];
	sprintf(t_timeString,"%d年%d月%d日,%d时%d分%d秒",
		t_time.wYear,t_time.wMonth,t_time.wDay,
		t_time.wHour,t_time.wMinute,t_time.wSecond);
	Static_SetText(t_wnd,t_timeString);

	unguard;
}
const std::string& AddBugDlg::GetBugLogContain(const char* _szFilename)
{
	guard;

	using namespace std;

	ifstream t_file(_szFilename);

	m_strLogContain.clear();

	if(t_file.rdbuf()->is_open()){
		
		std::istreambuf_iterator<char> t_beg(t_file), t_ed;
		std::string t_fullContain(t_beg, t_ed);

		// find the begin flag
		const size_t t_begin = t_fullContain.find("<#$>") ;
		const size_t t_end = t_fullContain.find("<$#>");

		if(t_begin != -1 && t_end != -1){
			
			const int t_size = (int)(t_end - t_begin + 1 + 4);

			if(t_size <= 0){

				m_strLogContain  = "";

			}else{

				m_strLogContain.resize(t_size);

				// search the begin 
				//
				copy(t_fullContain.begin() + t_begin + 4,t_fullContain.begin() + t_end,m_strLogContain.begin());
			}
			
						
		}else{

			m_strLogContain = t_fullContain;
		}

	}

	return m_strLogContain;

	unguard;
}
//====================================================================================================================
//
//	BugListDlg class to manage the dialog and bug list (fix list)data
//
//====================================================================================================================
BugListDlg::BugListDlg(void):m_hListView(NULL),
							m_hDlg(NULL),
							m_uDlgID(-1),
							m_timeFilter(e_unselected),
							m_severityFilter(Bug::e_unselected_sev),
							m_bugStateFilter(Fix::e_unselected_fix)
{
	//nothing to do now
}
BugListDlg::~BugListDlg(void)
{
	
	DeleteAllItemParam();
}

//! initialize the dialog
void BugListDlg::InitDialog(HINSTANCE _ins,BugManage* _bugMgr, UINT _uDlgID)
{
	guard;

	osassert(::IsWindow(*_bugMgr));
	
	m_hDlg = ::CreateDialog((HINSTANCE)_ins,MAKEINTRESOURCE(IDD_BUG_LIST),*_bugMgr,BugListDlgProc);
	osassert(::IsWindow(m_hDlg));

	::ShowWindow(m_hDlg,SW_HIDE);
	m_hListView = ::GetDlgItem(m_hDlg,IDC_BUG_LIST);


	InsertColumn(0,"创建者",150);
	InsertColumn(1,"创建时间",120);
	InsertColumn(2,"Bug名称",250);
	InsertColumn(3,"严重等级",60);
	InsertColumn(4,"Bug状态",200);

	m_hOwnerFilterList = ::GetDlgItem(*this,IDC_OWNER_FILTER_LIST);
	osassert(::IsWindow(m_hOwnerFilterList));

	m_hTimeFilterList = ::GetDlgItem(*this,IDC_TIME_FILTER_LIST);
	osassert(::IsWindow(m_hTimeFilterList));

	ComboBox_AddString(m_hTimeFilterList,"前一天");
	ComboBox_AddString(m_hTimeFilterList,"前两天");
	ComboBox_AddString(m_hTimeFilterList,"前三天");
	ComboBox_AddString(m_hTimeFilterList,"前一个星期");
	ComboBox_AddString(m_hTimeFilterList,"前两个星期");
	ComboBox_AddString(m_hTimeFilterList,"前一个月");


	m_hSeverityFilterList = ::GetDlgItem(*this,IDC_SEVERITY_FILTER_LIST);
	osassert(::IsWindow(m_hSeverityFilterList));

	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::CantTest_A.c_str());
	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::Assert_B.c_str());
	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::Major_C.c_str());
	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::Medium_D.c_str());
	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::Minor_E.c_str());
	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::Question_F.c_str());
	ComboBox_AddString(m_hSeverityFilterList,BugSeverityString::Suggest_G.c_str());

	m_hBugStateFilterList = ::GetDlgItem(*this,IDC_FILTER_BUG_STATE);
	osassert(::IsWindow(m_hBugStateFilterList));

	ComboBox_AddString(m_hBugStateFilterList,"[Fixed]");
	ComboBox_AddString(m_hBugStateFilterList,"[Fixed OK]");
	ComboBox_AddString(m_hBugStateFilterList,"[Is Not A Bug]");
	ComboBox_AddString(m_hBugStateFilterList,"[Will Not Fixed]");
	ComboBox_AddString(m_hBugStateFilterList,"[Waiting...]");

	m_hFilterString = ::GetDlgItem(*this,IDC_FILTER_STRING);

	
	unguard;
}

//! insert a column
int BugListDlg::InsertColumn(int _index,const char* _szName,int _iWidth)const 
{
	guard;

	osassert(::IsWindow(m_hDlg));

	LVCOLUMN t_column;

	t_column.mask		= LVCF_TEXT|LVCF_FMT|LVCF_WIDTH|LVCF_SUBITEM;
	t_column.pszText	= (LPTSTR)_szName;
	t_column.fmt		= LVCFMT_LEFT;
	t_column.iSubItem	= _index;
	t_column.cx			= _iWidth;

	osassert(::IsWindow(m_hListView));

	return (int)::SendMessage(m_hListView, LVM_INSERTCOLUMN, _index, (LPARAM)&t_column);

	unguard;
}

//! compare the time to filter the bug by the time filter
BOOL BugListDlg::FilterPhase(const SYSTEMTIME& _bugTime,const SYSTEMTIME& _nowTime)const 
{
	guard;

	if(m_timeFilter == e_unselected){
		return TRUE;
	}

	BOOL t_bResult = FALSE;

	FILETIME t_bugTime;
	FILETIME t_nowTime;

	::SystemTimeToFileTime(&_bugTime,&t_bugTime);
	::SystemTimeToFileTime(&_nowTime,&t_nowTime);

	ULONGLONG t_bugT				= t_bugTime.dwHighDateTime;
	t_bugT = t_bugT << 32;
	t_bugT |= t_bugTime.dwLowDateTime;
	ULONGLONG t_nowT				= t_nowTime.dwHighDateTime;
	t_nowT = t_nowT << 32;
	t_nowT |= t_nowTime.dwLowDateTime;

	ULONGLONG t_oneDayTime				= 24l * 60l * 60l ;
	t_oneDayTime *= 10000000l;

	const ULONGLONG t_oneWeekTime		= 7 * t_oneDayTime;
	const ULONGLONG t_oneMonthTime		= 30 * t_oneDayTime;


	if(m_timeFilter == e_oneDay){
		if(t_nowT - t_bugT <= t_oneDayTime){
			t_bResult = TRUE;
		}

	}else if(m_timeFilter == e_twoDays){
		if(t_nowT - t_bugT <= 2 * t_oneDayTime){
			t_bResult = TRUE;
		}
		
	}else if(m_timeFilter == e_threeDays){
		if(t_nowT - t_bugT <= 3 * t_oneDayTime){
			t_bResult = TRUE;
		}

	}else if(m_timeFilter == e_oneWeek){
		if(t_nowT - t_bugT <= t_oneWeekTime){
			t_bResult = TRUE;
		}

	}else if(m_timeFilter == e_twoWeeks ){
		if(t_nowT - t_bugT <= 2 * t_oneWeekTime){
			t_bResult = TRUE;
		}
	}else if(m_timeFilter ==  e_oneMonth){
		if(t_nowT - t_bugT <= t_oneMonthTime){
			t_bResult = TRUE;
		}
	}

	return t_bResult;


	unguard;
}

//! message procedure
BOOL BugListDlg::MsgProcedure(UINT _msg,WPARAM _wParam ,LPARAM _lParam)
{
	guard;

	BOOL t_bProcessed = FALSE;

	if(_msg == WM_NOTIFY){
		
		LPNMHDR t_pnmhdr = (LPNMHDR)_lParam;

		if (t_pnmhdr->idFrom == IDC_BUG_LIST ){

			if(t_pnmhdr->code == LVN_COLUMNCLICK){
				
				OnColumnClicked((LPNMLISTVIEW)_lParam);
				
				t_bProcessed = TRUE;
			}else if(t_pnmhdr->code == LVN_ITEMCHANGED){
				
				OnItemChange((LPNMLISTVIEW)_lParam);
				t_bProcessed = TRUE;

			}else if(t_pnmhdr->code == LVN_ITEMACTIVATE){
				OnCheckBug();
				t_bProcessed = TRUE;
			}
		}

	}
	if(_msg == WM_CONTEXTMENU){

		POINT t_screenPos = {LOWORD(_lParam),HIWORD(_lParam)};

		OnContextMenu(&t_screenPos);

	}
	
	if(_msg == WM_COMMAND){
		
		

		if(HIWORD(_wParam) == CBN_SELCHANGE){

			switch(LOWORD(_wParam)){
	
				case IDC_SEVERITY_FILTER_LIST:
					OnSeverityFilterListChange();
					break;
				case IDC_TIME_FILTER_LIST:
					OnTimeFilterListChange();
					break;
				case IDC_OWNER_FILTER_LIST:
					OnBugOwnerFilterListChange();
					break;
				case IDC_FILTER_BUG_STATE:
					OnBugStateFilterListChange();
					break;
			}

		}else{
			
			switch(LOWORD(_wParam)){

				case ID_CHECK_BUG:
					OnCheckBug();
					break;
				case IDC_DISPLAY_ALL:
					OnDislayAll();
					break;
				case IDC_STATISTIC_BUT:
					OnStatistic();
					break;
				case ID_BUG_COMMIT_HELP:
					g_bugMgr.ShowHelpDlg();
					break;
				case ID_QA_FIXED_OK:
					OnQAFixedOK();
					break;
				case ID_FIX_FIXED:
					OnFixFixed();
					break;
				case ID_FIX_NAB:
					OnFixNAB();
					break;
				case ID_FIX_WNF:
					OnFixWNF();
					break;
				case IDC_FILTER_STRING_BUT:
					OnFiliterStringBut();
					break;

			}

		}

	}

	

	return t_bProcessed;
	

	unguard;
}

void BugListDlg::OnQAFixedOK(void)
{
	guard;

	g_strDlgTitle = "的确要标记[Fixed OK]吗？";
	if(!DialogBox(BugManage::sm_instance,MAKEINTRESOURCE(IDD_INTERPRET_DLG),g_bugMgr,FixInterpreDlg)){
		return;
	}
	
	ItemParam* t_itemData = GetItemParamInSel();

	// initialize the fix to added
	//
	Fix t_fix;

	t_fix.SetBugOwner(t_itemData->m_bug->GetBugOwner().c_str());
	t_fix.SetBugState(Fix::e_fixedOK);
	t_fix.SetBugTime(t_itemData->m_bug->GetBugTime());
	t_fix.SetFixerName(AddBugDlg::GetTempBug().GetBugOwner().c_str());
	t_fix.SetInterpretString(g_strFixInterpret);

	SYSTEMTIME t_time;
	::GetLocalTime(&t_time);
	t_fix.SetFixTime(t_time);

	g_bugMgr.AddFixToUserFixFile(t_fix);
	m_currFixList.AddFix(t_fix);

	t_itemData->m_fix = m_currFixList.SearchFix(t_fix.GetBugOwner(),t_time);
	osassert(t_itemData->m_fix);

	RefreshListView();
    

	unguard;
}
//! on fix fixed;
void BugListDlg::OnFixFixed()
{
	guard;

	g_strDlgTitle = "的确要标记[Fixed]吗？";
	if(!DialogBox(BugManage::sm_instance,MAKEINTRESOURCE(IDD_INTERPRET_DLG),g_bugMgr,FixInterpreDlg)){
		return;
	}

	ItemParam* t_itemData = GetItemParamInSel();
	osassert(t_itemData);

	// initialize the fix to added
	//
	Fix t_fix;

	t_fix.SetBugOwner(t_itemData->m_bug->GetBugOwner().c_str());
	t_fix.SetBugState(Fix::e_fixed);
	t_fix.SetBugTime(t_itemData->m_bug->GetBugTime());
	t_fix.SetFixerName(AddBugDlg::GetTempBug().GetBugOwner().c_str());
	t_fix.SetInterpretString(g_strFixInterpret);

	SYSTEMTIME t_time;
	::GetLocalTime(&t_time);
	t_fix.SetFixTime(t_time);

	g_bugMgr.AddFixToUserFixFile(t_fix);
	m_currFixList.AddFix(t_fix);

	t_itemData->m_fix = m_currFixList.SearchFix(t_fix.GetBugOwner(),t_time);
	osassert(t_itemData->m_fix);

	RefreshListView();

	

	unguard;
}
//! on fix Not a Bug
void BugListDlg::OnFixNAB()
{
	guard;

	g_strDlgTitle = "的确要标记[Not A Bug]吗？";
	if(!DialogBox(BugManage::sm_instance,MAKEINTRESOURCE(IDD_INTERPRET_DLG),g_bugMgr,FixInterpreDlg)){
		return;
	}

	ItemParam* t_itemData = GetItemParamInSel();
	osassert(t_itemData);

	// initialize the fix to added
	//
	Fix t_fix;

	t_fix.SetBugOwner(t_itemData->m_bug->GetBugOwner().c_str());
	t_fix.SetBugState(Fix::e_isNotBug);
	t_fix.SetBugTime(t_itemData->m_bug->GetBugTime());
	t_fix.SetFixerName(AddBugDlg::GetTempBug().GetBugOwner().c_str());
	t_fix.SetInterpretString(g_strFixInterpret);

	SYSTEMTIME t_time;
	::GetLocalTime(&t_time);
	t_fix.SetFixTime(t_time);

	g_bugMgr.AddFixToUserFixFile(t_fix);
	m_currFixList.AddFix(t_fix);

	t_itemData->m_fix = m_currFixList.SearchFix(t_fix.GetBugOwner(),t_time);
	osassert(t_itemData->m_fix);

	RefreshListView();

	unguard;
}

//! on fix Will not Fix
void BugListDlg::OnFixWNF()
{
	guard;

	g_strDlgTitle = "的确要标记[Will Not Fix]吗？";
	if(!DialogBox(BugManage::sm_instance,MAKEINTRESOURCE(IDD_INTERPRET_DLG),g_bugMgr,FixInterpreDlg)){
		return;
	}

	ItemParam* t_itemData = GetItemParamInSel();
	osassert(t_itemData);

	// initialize the fix to added
	//
	Fix t_fix;

	t_fix.SetBugOwner(t_itemData->m_bug->GetBugOwner().c_str());
	t_fix.SetBugState(Fix::e_willNotFix);
	t_fix.SetBugTime(t_itemData->m_bug->GetBugTime());
	t_fix.SetFixerName(AddBugDlg::GetTempBug().GetBugOwner().c_str());
	t_fix.SetInterpretString(g_strFixInterpret);

	SYSTEMTIME t_time;
	::GetLocalTime(&t_time);
	t_fix.SetFixTime(t_time);

	g_bugMgr.AddFixToUserFixFile(t_fix);
	m_currFixList.AddFix(t_fix);

	t_itemData->m_fix = m_currFixList.SearchFix(t_fix.GetBugOwner(),t_time);
	osassert(t_itemData->m_fix);

	RefreshListView();

	unguard;
}

void BugListDlg::OnFiliterStringBut(void)
{
	guard;

	osassert(::IsWindow(m_hFilterString));

	const int t_len = Edit_GetTextLength(m_hFilterString);
	char* t_text = new char[t_len +1];

	Edit_GetText(m_hFilterString,t_text,t_len + 1);
	
	m_strFilterString = t_text;
	delete t_text;

	RefreshListView();

	unguard;
}

void BugListDlg::OnColumnClicked(LPNMLISTVIEW _pnm)
{
	guard;
	
	
	osassert(::IsWindow(m_hListView));

	ListView_SortItems(m_hListView,CompareFunc,_pnm->iSubItem);

	g_bugSortAscend[_pnm->iSubItem] = !g_bugSortAscend[_pnm->iSubItem];
	
	
	
	unguard;
}

void BugListDlg::OnItemChange(LPNMLISTVIEW _pnm)
{
	guard;
	
	
	
	unguard;
}

BugListDlg::ItemParam* BugListDlg::GetItemParamInSel(void)const
{
	guard;

	osassert(::IsWindow(m_hListView));

	const int t_iCurSel = ListView_GetNextItem(m_hListView,-1,LVIS_SELECTED);

	osassert(t_iCurSel != -1);

	LVITEM t_item;
	ZeroMemory(&t_item,sizeof(t_item));
	t_item.iItem = t_iCurSel;
	t_item.mask = LVIF_PARAM;

	ListView_GetItem(m_hListView,&t_item);
	osassert(t_item.lParam);

	return (ItemParam*)t_item.lParam;

	unguard;
}

void BugListDlg::OnCheckBug(void)
{
	guard;

	osassert(::IsWindow(m_hListView));

	const int t_selItem = ListView_GetNextItem(m_hListView,-1,LVIS_SELECTED);

	if(t_selItem >= 0){

		LVITEM t_item = {0};
		t_item.iItem = t_selItem;
		t_item.mask = LVIF_PARAM;
		ListView_GetItem(m_hListView,&t_item);

		if( t_item.iItem == -1) return;
		osassert(t_item.lParam);

		ItemParam* t_itemParam = (ItemParam*)t_item.lParam;

		Bug* t_bug = t_itemParam->m_bug;

		std::string t_desc;

		t_bug->GetWholeBugNote(t_desc);

		// get the fix of this bug
		// and append the string to it...
		Fix* t_parrFix[1024] = {0};
		int t_fixNum = 0;
		m_currFixList.SearchFix(t_bug->GetBugOwner(),t_bug->GetBugTime(),t_parrFix,t_fixNum);
		
		for(int i = 0;i<t_fixNum;i++){
			t_desc += "<========================Fixer Record===========================>\n\n";
			t_parrFix[i]->GetFixDesc(t_desc);
		}

		DisplayStringByNotePad(t_desc);

	}

	unguard;
}
void BugListDlg::OnDislayAll(void)
{
	guard;

	osassert(::IsWindow(m_hSeverityFilterList));
	osassert(::IsWindow(m_hTimeFilterList));
	osassert(::IsWindow(m_hOwnerFilterList));
	osassert(::IsWindow(m_hBugStateFilterList));
	osassert(::IsWindow(m_hFilterString));

	ComboBox_SetCurSel(m_hSeverityFilterList,-1);
	ComboBox_SetCurSel(m_hTimeFilterList,-1);
	ComboBox_SetCurSel(m_hOwnerFilterList,-1);
	ComboBox_SetCurSel(m_hBugStateFilterList,-1);

	Edit_SetText(m_hFilterString,"");

	
	m_severityFilter	= Bug::e_unselected_sev;
	m_timeFilter		= e_unselected;
	m_bugStateFilter	= Fix::e_unselected_fix;

	m_strOwnerFilter.clear();
	m_strFilterString.clear();

	RefreshListView();

	unguard;
}

void BugListDlg::OnStatistic(void)
{
	guard;

	using namespace std;

	map<string,vector<int> > t_bugCommiter;
	const list<Bug*>& t_bugList = m_currBugList.GetList();

	for(list<Bug*>::const_iterator it = t_bugList.begin();
	it != t_bugList.end();
	it++){
		const Bug* t_bug = *it;
		map<string,vector<int> >::iterator t_findBug = t_bugCommiter.find(t_bug->GetBugOwner());
		if(t_findBug == t_bugCommiter.end()){
			// add a new committer to statistic...
			//
			vector<int> t_statistic;
			t_statistic.resize(Fix::e_waiting + 1);
			
			Fix* t_fix = m_currFixList.FindRightFix(t_bug);
			if(t_fix){
				t_statistic[t_fix->GetBugState()]++;
			}else{
				t_statistic[Fix::e_waiting]++;
			}

			const pair<map<string,vector<int> >::iterator,BOOL> t_pair = 
				t_bugCommiter.insert(make_pair(t_bug->GetBugOwner(),t_statistic));

			osassert(t_pair.second);
			
		}else{
			// find the right fix to increase the bug state
			//
			Fix* t_fix = m_currFixList.FindRightFix(t_bug);
			if(t_fix){
				t_findBug->second.at(t_fix->GetBugState())++;
			}else{
				t_findBug->second.at(Fix::e_waiting)++;
			}
		}
	}

	ostringstream  t_output;

	t_output << left << setw(80) << "Committer" 
			<< left <<setw(16) <<"Waiting"
			<< left <<setw(16) <<"Fixed" 
			<< left <<setw(16) <<"Fixed OK"
			<< left <<setw(16) <<"Not A Bug"
			<< left <<setw(16) <<"Will Not Fix"
			<< left <<setw(16) <<"全部" << endl;
	int t_totalBug = 0;
	for(map<string,vector<int> >::iterator it = t_bugCommiter.begin();
	it != t_bugCommiter.end();
	it++){

		int t_bugCommitterNum = it->second.at(Fix::e_waiting)+
								it->second.at(Fix::e_fixed) +
								it->second.at(Fix::e_fixedOK) +
								it->second.at(Fix::e_isNotBug) +
								it->second.at(Fix::e_willNotFix);
		
		t_output << left <<setw(64) << it->first;
		t_output << left <<setw(20) << (int)(it->second.at(Fix::e_waiting));
		t_output << left <<setw(20) << (int)(it->second.at(Fix::e_fixed));
		t_output << left <<setw(20) << it->second.at(Fix::e_fixedOK);
		t_output << left <<setw(20) << it->second.at(Fix::e_isNotBug);
		t_output << left <<setw(20) << it->second.at(Fix::e_willNotFix);
		t_output << left <<setw(20) << t_bugCommitterNum;

		t_output <<endl;

		t_totalBug += t_bugCommitterNum;
	
	}

	t_output << endl <<"所有Bug数量:  "<<t_totalBug;

	::MessageBox(*this,t_output.str().c_str(),"Bug 统计:",MB_OK);

	unguard;
}

void BugListDlg::OnSeverityFilterListChange()
{
	guard;
	
	osassert(::IsWindow(m_hSeverityFilterList));

	m_severityFilter = (Bug::Severity)ComboBox_GetCurSel(m_hSeverityFilterList);

	RefreshListView();
	
	unguard;
}
void BugListDlg::OnTimeFilterListChange()
{
	guard;
	
	osassert(::IsWindow(m_hTimeFilterList));

	m_timeFilter = (TimeFilter)ComboBox_GetCurSel(m_hTimeFilterList);
		
	RefreshListView();
	
	unguard;
}
void BugListDlg::OnBugOwnerFilterListChange(void)
{
	guard;

	osassert(::IsWindow(m_hOwnerFilterList));

	const int t_len = BugManage::scm_iComputerNameLen + BugManage::scm_iUserNameLen + sizeof(" @ ");
	char t_owner[t_len];

	const int t_selIndex = ComboBox_GetCurSel(m_hOwnerFilterList);

	osassert(t_selIndex != -1);

	ComboBox_GetText(m_hOwnerFilterList,t_owner,t_len);

	m_strOwnerFilter = t_owner;

    RefreshListView();

	unguard;
}

void BugListDlg::OnBugStateFilterListChange(void)
{
	guard;

	osassert(::IsWindow(m_hBugStateFilterList));

	const int t_iCurSel = ComboBox_GetCurSel(m_hBugStateFilterList);

	osassert(t_iCurSel >= Fix::e_fixed && t_iCurSel <= Fix::e_waiting);

	m_bugStateFilter = (Fix::FixStyle)t_iCurSel;

	RefreshListView();


	unguard;
}

void BugListDlg::OnContextMenu(POINT* _pos)
{
	guard;

	osassert(::IsWindow(m_hListView));

	RECT t_listRect;
	::GetWindowRect(m_hListView,&t_listRect);

	if(::PtInRect(&t_listRect,*_pos)){

		int t_itemIndex = ListView_GetNextItem(m_hListView,-1,LVIS_SELECTED);

		LVITEM t_item = {0};

		t_item.iItem	= t_itemIndex;
		t_item.mask		= LVIF_PARAM;

		ListView_GetItem(m_hListView,&t_item);

		if(t_item.iItem == -1) return;

		osassert(t_item.lParam);

		BugListDlg::ItemParam* t_itemParam = (BugListDlg::ItemParam*)t_item.lParam;

		Bug* t_bug = t_itemParam->m_bug;

		HMENU t_menu = NULL;
		
		BOOL t_QA = TRUE;
		if(t_bug->GetBugOwner() == AddBugDlg::GetTempBug().GetBugOwner()){
			t_menu = ::LoadMenu(BugManage::sm_instance,MAKEINTRESOURCE(IDR_QA_MENU));
		}else{
			t_menu = ::LoadMenu(BugManage::sm_instance,MAKEINTRESOURCE(IDR_FIXER));
			t_QA = FALSE;
		}

		if(!t_menu) return;

		HMENU t_trackMenu  = ::GetSubMenu(t_menu,0);

		SetMenuState(t_trackMenu,t_itemParam,t_QA);

		TrackPopupMenu(t_trackMenu, 
			TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
			_pos->x, _pos->y, 0, *this, NULL); 

		

		::DestroyMenu(t_menu);

	}


	unguard;
}

//! get (construct) the param ptr
BugListDlg::ItemParam* BugListDlg::GetItemParamPtr(Bug* _bug)
{
	guard;


	ItemParam* t_itemParam  = new ItemParam;

	t_itemParam->m_bug = _bug;
	t_itemParam->m_fix = m_currFixList.FindRightFix(_bug);

	m_vectItemParam.push_back(t_itemParam);

	return t_itemParam;

	unguard;
}

//! delete  all item param
void BugListDlg::DeleteAllItemParam(void)
{
	guard;

	for(size_t i= 0;i<m_vectItemParam.size();i++){
		
		delete m_vectItemParam[i];

	}

	m_vectItemParam.clear();

	unguard;
}

void BugListDlg::SetMenuState(HMENU _menu,ItemParam* _itemData ,BOOL _isQA)
{
	guard;

	osassert(_menu);
	
	MENUITEMINFO t_mi;
	ZeroMemory(&t_mi,sizeof(t_mi));

	t_mi.cbSize = sizeof(t_mi);
	t_mi.fMask = MIIM_STATE;

	if(_isQA){
		t_mi.fState = MFS_DISABLED;

		if(_itemData->m_bug->GetBugOwner() == AddBugDlg::GetTempBug().GetBugOwner()){
			if(_itemData->m_fix){
				const Fix::FixStyle t_fixStyle = _itemData->m_fix->GetBugState();
				if( t_fixStyle == Fix::e_fixed){
					t_mi.fState = MFS_ENABLED;
				}
			}else{
				t_mi.fState = MFS_ENABLED;
			}
		}
		
		::SetMenuItemInfo(_menu,ID_QA_FIXED_OK,FALSE,&t_mi);

		t_mi.fState = MFS_DISABLED;
		::SetMenuItemInfo(_menu,ID_QA_CLEAR,FALSE,&t_mi);

	}else{

		t_mi.fState = MFS_ENABLED;
		if(_itemData->m_fix){
			if(_itemData->m_fix->GetBugState() == Fix::e_fixedOK
			||_itemData->m_fix->GetBugState() == Fix::e_willNotFix){
				t_mi.fState = MFS_DISABLED;
			}
		}
		
		::SetMenuItemInfo(_menu,ID_FIX_FIXED,FALSE,&t_mi);
		::SetMenuItemInfo(_menu,ID_FIX_NAB,FALSE,&t_mi);
		::SetMenuItemInfo(_menu,ID_FIX_WNF,FALSE,&t_mi);


	}

	

	unguard;
}

//! refresh the bug list
void BugListDlg::RefreshListView(void)
{
	guard;

	osassert(::IsWindow(m_hListView));
	
	//! delete all items
	ListView_DeleteAllItems(m_hListView);
	DeleteAllItemParam();

	const std::list<Bug*>& t_list = m_currBugList.GetList();


	SYSTEMTIME t_time;
	::GetLocalTime(&t_time);

	for(std::list<Bug*>::const_iterator it = t_list.begin();
	it != t_list.end();
	it++){

		Bug* t_pBug = *it;

		if(t_pBug->GetSeverity() != m_severityFilter && m_severityFilter != Bug::e_unselected_sev){
			continue;
		}

		if(t_pBug->GetBugOwner() != m_strOwnerFilter && m_strOwnerFilter.size()){
			continue;
		}

		if(!FilterPhase(t_pBug->GetBugTime(),t_time)){
			continue;
		}

		if(m_bugStateFilter != Fix::e_unselected_fix){

			Fix* t_fix = m_currFixList.FindRightFix(t_pBug);

			if(t_fix != NULL){
				if(t_fix->GetBugState() != m_bugStateFilter){
					continue;
				}
			}else{
				if(m_bugStateFilter != Fix::e_waiting){
					continue;
				}
			}
		}
		

		if(!FindBugFixByKeyWord(t_pBug,m_strFilterString)){
			continue;
		}

		ItemParam* t_param = GetItemParamPtr(t_pBug);

		LV_ITEM t_item = {0};
		t_item.mask = LVIF_TEXT|LVIF_PARAM;
		t_item.iItem = 0;
		t_item.lParam = (LPARAM)t_param;
		t_item.pszText = (LPSTR)t_pBug->GetBugOwner().c_str();

		

		ListView_InsertItem(m_hListView,&t_item);
		ListView_SetItemText(m_hListView,0,1,(LPSTR)t_pBug->GetBugTimeString());
		ListView_SetItemText(m_hListView,0,2,(LPSTR)t_pBug->GetBugName().c_str());
		ListView_SetItemText(m_hListView,0,3,(LPSTR)t_pBug->GetSeverityDesc(TRUE));

		if(t_param->m_fix){

			ListView_SetItemText(m_hListView,0,4,(LPSTR)t_param->m_fix->GetFixStateString());		

		}else{

			ListView_SetItemText(m_hListView,0,4,(LPSTR)"waiting...");		
		}
		
		
	}
	

	unguard;
}
void BugListDlg::InsertBugOwnerFilterString(const char* _bugOwner)const 
{
	guard;

	osassert(::IsWindow(m_hOwnerFilterList));

	ComboBox_AddString(m_hOwnerFilterList,_bugOwner);

	unguard;
}




//====================================================================================================================
//
//	Bug class to manage the bug data
//
//====================================================================================================================
Bug::Bug(void):m_dwGameVersion(0),
				m_severity(e_unselected_sev),
				m_location(e_unselected_loc),
				m_reproRate(e_unselected_repro)
{

	ZeroMemory(&m_time,sizeof(m_time));
	ZeroMemory(&m_relationBug,sizeof(m_relationBug));

}
Bug::~Bug(void)
{
	//nothing to do
}
void Bug::NewBug(const char* _bugOwner /* = NULL */)
{
	guard;

	::GetLocalTime(&m_time);
	ZeroMemory(&m_relationBug,sizeof((m_relationBug)));

	m_dwGameVersion = 0;

	m_strBugName.clear();
	m_strBugOwner.clear();
	m_severity = e_unselected_sev;
	m_location = e_unselected_loc;
	m_reproRate = e_unselected_repro;
	m_strLogFilename.clear();
	m_strLogFileContain.clear();
	m_strDesc.clear();
	m_vReproStep.clear();

	if(_bugOwner){
		SetBugOwner(_bugOwner);
	}
	
	unguard;
}

Bug& Bug::operator=(const Bug& _bug)
{
	guard;

	m_time				= _bug.m_time;
	m_strBugOwner		= _bug.m_strBugOwner;
	m_dwGameVersion		= _bug.m_dwGameVersion;
	m_strBugName		= _bug.m_strBugName;
	m_severity			= _bug.m_severity;
	m_location			= _bug.m_location;
	m_relationBug		= _bug.m_relationBug;
	m_reproRate			= _bug.m_reproRate;
	m_strLogFilename	= _bug.m_strLogFilename;
	m_strLogFileContain	= _bug.m_strLogFileContain;
	m_strDesc			= _bug.m_strDesc;
	
	m_vReproStep.clear();
	m_vReproStep.resize(_bug.m_vReproStep.size());
	std::copy(_bug.m_vReproStep.begin(),_bug.m_vReproStep.end(),m_vReproStep.begin());

	return *this;

	unguard;
}

void Bug::GetWholeBugNote(std::string& _str)const
{
	guard;

	char t_tmpString[128];
	sprintf(t_tmpString,"%d月%d日%d时%d分(星期%d)",
		m_time.wMonth,m_time.wDay,m_time.wHour,m_time.wMinute,m_time.wDayOfWeek);

	_str += "[时间]:\t";
	_str += t_tmpString;
	_str += "\n\n";

	_str += "[提交人]:\t";
	_str += m_strBugOwner;
	_str += "\n\n";

	_str += "[Bug名称]:\t";
	_str += m_strBugName;
	_str += "\n\n";

	_str += "[严重程度]:\t";
	_str += GetSeverityDesc(FALSE);
	_str += "\n\n";

	_str += "[出现位置]:\t";
	_str += GetLocationDesc(FALSE);
	_str += "\n\n";

	_str += "[相关Bug]:\t";
	_str += "无";
	_str += "\n\n";

	_str += "[重现机率]:\t";
	_str +=  GetReproRateDesc(FALSE);
	_str += "\n\n";

	_str += "[Log文件 ";
	_str += m_strLogFilename;
	_str += " ]:\n";
	_str += m_strLogFileContain.size()?m_strLogFileContain:"\t无";
	_str += "\n\n";

	_str += "[Bug详细描述]:\n";
	_str += "\t";
	_str += m_strDesc.size()?m_strDesc:"\t无";
	_str += "\n\n";

	_str += "[Bug重现步骤]:\n";
	
	for(size_t i = 0;i< m_vReproStep.size();i++){
		sprintf(t_tmpString,"\t第%d步:",i +1);
		_str += t_tmpString;
		_str += m_vReproStep[i];
		_str += "\n";
	}
    
	unguard;
}

const char* Bug::GetBugTimeString(void)const 
{
	guard;

	static char st_string[128];

	sprintf(st_string,"%d月%d日%d时%d分",
		m_time.wMonth,m_time.wDay,m_time.wHour,m_time.wMinute);

	return st_string;

	unguard;
}

const char* Bug::GetSeverityDesc(BOOL _bAbstract /* = TRUE */)const
{
	guard;

	osassert(GetSeverity() != e_unselected_sev);

	const char* t_reStr = NULL;

	if(_bAbstract){
		switch(GetSeverity()){

			case e_cantTest_A:
				t_reStr = "A";			
				break;
			case e_assert_B:
				t_reStr = "B";
				break;
			case e_major_C:
				t_reStr = "C";
				break;
			case e_medium_D:
				t_reStr = "D";
				break;
			case e_minor_E:
				t_reStr = "E";
				break;
			case e_question_F:
				t_reStr = "F";
				break;
			case e_suggest_G:
				t_reStr = "G";
				break;
			default:
				osassert(false);
				break;
		}

	}else{
		switch(GetSeverity()){

			case e_cantTest_A:
				t_reStr = BugSeverityString::CantTest_A.c_str();			
				break;
			case e_assert_B:
				t_reStr = BugSeverityString::Assert_B.c_str();
				break;
			case e_major_C:
				t_reStr = BugSeverityString::Major_C.c_str();
				break;
			case e_medium_D:
				t_reStr = BugSeverityString::Medium_D.c_str();
				break;
			case e_minor_E:
				t_reStr = BugSeverityString::Minor_E.c_str();
				break;
			case e_question_F:
				t_reStr = BugSeverityString::Question_F.c_str();
				break;
			case e_suggest_G:
				t_reStr = BugSeverityString::Suggest_G.c_str();
				break;
			default:
				osassert(false);
				break;
		}
	}

	osassert(t_reStr);

	return t_reStr;

	unguard;
}
const char* Bug::GetLocationDesc(BOOL _bAbstract /* = TRUE */)const
{
	guard;

	osassert(GetLocation() != e_unselected_loc);

	const char* t_reStr = NULL;


	if(_bAbstract){
		switch(GetLocation()){

			case e_actionPhase:
				t_reStr = "进行中";			
				break;
			case e_all:
				t_reStr = "全局";
				break;
			case e_documentation:
				t_reStr = "文本";
				break;
			case e_fixLocation:
				t_reStr = "固定几处";
				break;
			case e_gameMenu:
				t_reStr = "菜单";
				break;
			case e_loading:
				t_reStr = "读取时";
				break;
			case e_gameScene:
				t_reStr = "场景";
				break;
			default:
				osassert(false);
				break;
		}

	}else{
		switch(GetLocation()){

			case e_actionPhase:
				t_reStr = BugLocationString::ActionPhase_note.c_str();			
				break;
			case e_all:
				t_reStr = BugLocationString::All_note.c_str();
				break;
			case e_documentation:
				t_reStr = BugLocationString::Documentation_note.c_str();
				break;
			case e_fixLocation:
				t_reStr = BugLocationString::Fix_note.c_str();
				break;
			case e_gameMenu:
				t_reStr = BugLocationString::GameMenu_note.c_str();
				break;
			case e_loading:
				t_reStr = BugLocationString::Loading_note.c_str();
				break;
			case e_gameScene:
				t_reStr = BugLocationString::GameScene_note.c_str();
				break;
			default:
				osassert(false);
				break;
		}
	}

	osassert(t_reStr);

	return t_reStr;
	

	unguard;
}

const char*  Bug::GetReproRateDesc(BOOL _bAbstract /* = TRUE */)const
{
	guard;

	osassert(GetReproRate() != e_unselected_repro);

	const char* t_reStr = NULL;
	if(_bAbstract){
		switch(GetReproRate()){
			case e_easyRepro_knowRule:
				t_reStr = "E_K";
				break;
			case e_easyRepro_unknownRule:
				t_reStr = "E_U";
				break;
			case e_hardRepro_knowRule:
				t_reStr = "H_K";
				break;
			case e_hardRepro_unknownRule:
				t_reStr = "H_U";
				break;
			default:
				osassert(false);
				break;
		}
	}else{

		switch(GetReproRate()){
				case e_easyRepro_knowRule:
					t_reStr = BugReproRateString::EasyReproKnowRule.c_str();
					break;
				case e_easyRepro_unknownRule:
					t_reStr = BugReproRateString::EasyReproUnknownRule.c_str();
					break;
				case e_hardRepro_knowRule:
					t_reStr = BugReproRateString::HardReproKnowRule.c_str();
					break;
				case e_hardRepro_unknownRule:
					t_reStr = BugReproRateString::HardReproUnknownRule.c_str();
					break;
				default:
					osassert(false);
					break;
		
		}
	}
	osassert(t_reStr);

	return t_reStr;
	unguard;
}
void Bug::WriteToFile(FILE* _hFile,DWORD _dwVersion)
{
	guard;

	osassert(_hFile);

	// write the version first
	//
	ForceWrite(&_dwVersion,sizeof(_dwVersion),1,_hFile);
	

	// write the contain ....
	// 
	ForceWrite(&m_time,sizeof(m_time),1,_hFile);
	ForceWrite(&m_dwGameVersion,sizeof(m_dwGameVersion),1,_hFile);
	
	int t_iTmp = (int)m_severity;
	ForceWrite(&t_iTmp,sizeof(t_iTmp),1,_hFile);
	
	t_iTmp = (int)m_location;
	ForceWrite(&t_iTmp,sizeof(t_iTmp),1,_hFile);

	t_iTmp = (int)m_reproRate;
	ForceWrite(&t_iTmp,sizeof(t_iTmp),1,_hFile);

	WriteString(m_strBugName,_hFile);
	WriteString(m_strBugOwner,_hFile);
	WriteString(m_strLogFilename,_hFile);
	WriteString(m_strLogFileContain,_hFile);
	WriteString(m_strDesc,_hFile);

	t_iTmp = (int)m_vReproStep.size();
	ForceWrite(&t_iTmp,sizeof(t_iTmp),1,_hFile);

	for(size_t i = 0;i< m_vReproStep.size();i++){
		WriteString(m_vReproStep[i],_hFile);
	}
	unguard;
}
void Bug::ReadFromFile(FILE* _hFile)
{
	guard;
	
	osassert(_hFile);

	// the version is store in every bug entity
	//
	DWORD t_dwVersion;
	ForceRead(&t_dwVersion,sizeof(t_dwVersion),1,_hFile);


	// read the contain
	//
	ForceRead(&m_time,sizeof(m_time),1,_hFile);
	ForceRead(&m_dwGameVersion,sizeof(m_dwGameVersion),1,_hFile);

	int t_iTmp = 0;
	ForceRead(&t_iTmp,sizeof(t_iTmp),1,_hFile);
	osassert(t_iTmp >= (int)e_cantTest_A && t_iTmp <= (int)e_suggest_G);
	m_severity = (enum Severity)t_iTmp;

	ForceRead(&t_iTmp,sizeof(t_iTmp),1,_hFile);
	osassert(t_iTmp >= (int)e_actionPhase && t_iTmp <= (int)e_gameScene);
	m_location = (enum Location)t_iTmp;

	ForceRead(&t_iTmp,sizeof(t_iTmp),1,_hFile);
	osassert(t_iTmp >= (int)e_easyRepro_knowRule && t_iTmp <= (int)e_hardRepro_unknownRule);
	m_reproRate = (enum ReproRate)t_iTmp;

	ReadString(m_strBugName,_hFile);
	ReadString(m_strBugOwner,_hFile);
	ReadString(m_strLogFilename,_hFile);
	ReadString(m_strLogFileContain,_hFile);
	ReadString(m_strDesc,_hFile);

	ForceRead(&t_iTmp,sizeof(t_iTmp),1,_hFile);
	m_vReproStep.resize(t_iTmp);

	for(int i = 0;i< t_iTmp;i++){
		ReadString(m_vReproStep[i],_hFile);
	}
	

	unguard;
}
//====================================================================================================================
//
//	Fix class to manage the Fix data
//
//====================================================================================================================
Fix::Fix(void):m_fixStyle(e_fixed)
{
	ZeroMemory(&m_bugTime,sizeof(m_bugTime));
	ZeroMemory(&m_fixTime,sizeof(m_fixTime));
}
Fix::~Fix(void)
{

}

Fix& Fix::operator =(const Fix& _fix)
{
	guard;

	m_bugTime		= _fix.m_bugTime;
	m_bugOwner		= _fix.m_bugOwner;
	m_fixerName		= _fix.m_fixerName;
	m_fixStyle		= _fix.m_fixStyle;
	m_fixTime		= _fix.m_fixTime;
	m_strInterpret	= _fix.m_strInterpret;

	return *this;

	unguard;
}
void Fix::WriteToFile(FILE* _hFile,DWORD _dwVersion)
{
	guard;

	osassert(_hFile);
	
	ForceWrite(&m_bugTime,sizeof(m_bugTime),1,_hFile);

	WriteString(m_bugOwner,_hFile);
	WriteString(m_fixerName,_hFile);

	int t_sytle = (int)m_fixStyle;
	ForceWrite(&t_sytle,sizeof(t_sytle),1,_hFile);

	ForceWrite(&m_fixTime,sizeof(m_fixTime),1,_hFile);

	if(_dwVersion == 11){
		WriteString(m_strInterpret,_hFile);
	}

	unguard;
}
void Fix::ReadFromFile(FILE* _hFile,DWORD _dwVersion)
{
	guard;

	ForceRead(&m_bugTime,sizeof(m_bugTime),1,_hFile);

	ReadString(m_bugOwner,_hFile);
	ReadString(m_fixerName,_hFile);

	int t_sytle = 0;
	ForceRead(&t_sytle,sizeof(t_sytle),1,_hFile);
	m_fixStyle = (enum FixStyle)t_sytle;

	ForceRead(&m_fixTime,sizeof(m_fixTime),1,_hFile);	

	if(_dwVersion == 11){
		ReadString(m_strInterpret,_hFile);
	}


	unguard;
}

//! get the fix state string
const char* Fix::GetFixStateString(void)const
{
	guard;

	static char st_string[256] = {0};

	switch(m_fixStyle){
		case e_fixed:
			sprintf(st_string,"[Fixed]  %s",GetFixerName().c_str());
			break;		
		case e_fixedOK:
			sprintf(st_string,"[Fixed OK]  %s",GetFixerName().c_str());
			break;
		case e_isNotBug:
			sprintf(st_string,"[Not A Bug]  %s",GetFixerName().c_str());
			break;
		case e_willNotFix:
			sprintf(st_string,"[Will Not Fix]  %s",GetFixerName().c_str());
			break;
		case e_reopen:
			sprintf(st_string,"[Re-Open]  %s",GetFixerName().c_str());
			break;
	}

	return st_string;

	unguard;
}


//! get the fix whole description
void Fix::GetFixDesc(std::string& _desc)const 
{
	guard;

	char t_tmpString[128];
	sprintf(t_tmpString,"%d月%d日%d时%d分(星期%d)",
		m_fixTime.wMonth,m_fixTime.wDay,m_fixTime.wHour,m_fixTime.wMinute,m_fixTime.wDayOfWeek);

	_desc += "[修正时间]:\t";
	_desc += t_tmpString;
	_desc += "\n\n";

	_desc += "[修正者]:\t";
	_desc += m_fixerName;
	_desc += "\n\n";

	_desc += "[修正类型]:\t";
	_desc += GetFixStateString();
	_desc += "\n\n";

	_desc += "[解释]:\n\t";
	_desc += GetInterpretString();
	_desc += "\n\n";


	unguard;
}


//====================================================================================================================
//
//	BugList class to manage all bugs (and fix) data
//
//====================================================================================================================
BugList::BugList(void)
{
	ZeroMemory(&m_header,sizeof(m_header));

	m_header.m_dwHeaderSize		= sizeof(m_header);
	m_header.m_szMagic[0]		='B';
	m_header.m_szMagic[1]		='U';
	m_header.m_szMagic[2]		='G';
	m_header.m_szMagic[3]		=0;
}
BugList::~BugList(void)
{
	for(std::list<Bug*>::iterator it = m_listBug.begin();
	it != m_listBug.end();
	it ++){

		Bug* t_ptrBug = *it;

		delete t_ptrBug;


	}

	m_listBug.clear();

}

void BugList::AddBug(const Bug& _bug,FILE* _hFile /* =NULL*/,DWORD _dwVersion /* =0*/)
{
	guard;

	Bug* t_pBug = new Bug(_bug);
	m_listBug.push_back(t_pBug);

	m_header.m_dwBugNum++;
	m_header.m_dwVersion = _dwVersion;

	if(_hFile){

		osassert(_dwVersion != 0);
		osassert(m_header.m_dwBugNum == m_listBug.size());

		fseek(_hFile,0,SEEK_SET);

		/*long t_pos = ftell(_hFile);
		osDebugOut(va("\n the file 's pointer is %d",t_pos));*/
		
		osDebugOut(va("\nBug number is %d",m_header.m_dwBugNum));
		ForceWrite(((void*)&m_header) ,sizeof(m_header) ,1,_hFile);

// 		ForceWrite(((void*)&m_header.m_dwHeaderSize) ,sizeof(m_header.m_dwHeaderSize) ,1,_hFile);
// 		ForceWrite(((void*)&m_header.m_dwVersion) ,sizeof(m_header.m_dwVersion) ,1,_hFile);
// 		ForceWrite(((void*)&m_header.m_dwBugNum) ,sizeof(m_header.m_dwBugNum) ,1,_hFile);
		
		fseek(_hFile,0,SEEK_END);

		// if the file is NOT empty , flush the bug to the file...
		//
		t_pBug->WriteToFile(_hFile,_dwVersion);

		fflush(_hFile);
	}

	unguard;
}

void BugList::AddRefBugPtr(Bug* _bug)
{
	guard;

	m_header.m_dwBugNum++;
	m_listBug.push_back(_bug);

	osassert(m_header.m_dwBugNum == m_listBug.size());

	unguard;
}

BOOL BugList::ReadFromFile(FILE* _hFile)
{
	guard;
	
	osassert(_hFile);
	
	fseek(_hFile,0,SEEK_SET);

	const size_t t_read = fread((void*)&m_header,sizeof(m_header),1,_hFile);

	if(t_read != 0){
		if(m_header.m_szMagic[0] != 'B'
			|| m_header.m_szMagic[1] != 'U'
			|| m_header.m_szMagic[2] != 'G'
			|| m_header.m_szMagic[3] != 0
			|| m_header.m_dwHeaderSize != sizeof(m_header)
			|| m_header.m_dwVersion < 0){

				return FALSE;
		}
	}else{
		return TRUE;
	}

	for(DWORD i= 0 ;i < m_header.m_dwBugNum;i++){
		
		Bug* t_ptrBug = new Bug;

		t_ptrBug->ReadFromFile(_hFile);

		m_listBug.push_back(t_ptrBug);

	}
	
	return TRUE;

	unguard;
}
void BugList::MoveAllBugToOtherList(BugList& _destList)
{
	guard;

	for(std::list<Bug*>::iterator it = m_listBug.begin();
	it != m_listBug.end();
	it ++){

		_destList.AddRefBugPtr(*it);

	}

	m_listBug.clear();

	unguard;
}

//====================================================================================================================
//
//	FixList class to manage all fixer data
//
//====================================================================================================================
FixList::FixList(void)
{
	ZeroMemory(&m_header,sizeof(m_header));

	m_header.m_dwHeaderSize = sizeof(m_header);
	m_header.m_szMagic[0] = 'F';
	m_header.m_szMagic[1] = 'I';
	m_header.m_szMagic[2] = 'X';
	m_header.m_szMagic[3] = 0;

	m_header.m_dwVersion = BugManage::scm_dwBugSysVersion;

}
FixList::~FixList(void)
{
	for(std::list<Fix*>::iterator it = m_listFix.begin();
	it != m_listFix.end();
	it++){

		Fix* t_fix = *it;
		delete t_fix;
	}

	m_listFix.clear();
}

void FixList::AddFix(const Fix& _fix,FILE* _hFile /* = NULL */,DWORD _dwVersion /* = 0 */)
{
	guard;

	const Fix* t_fixPtr = ConfirmFix(_fix);

	if(t_fixPtr){
		
		Fix* t_fix = new Fix(*t_fixPtr);
		m_listFix.push_back(t_fix);
		m_header.m_dwFixNum++;
	}
	

	if(_hFile){

		osassert(m_listFix.size() == m_header.m_dwFixNum);
		osassert(_dwVersion != 0);

		fseek(_hFile,0,SEEK_SET);
		
		m_header.m_dwVersion = _dwVersion;

		ForceWrite(&m_header,sizeof(m_header),1,_hFile);

		for(std::list<Fix*>::iterator it = m_listFix.begin();
		it != m_listFix.end();
		it++){

			Fix* t_fix = *it;
			t_fix->WriteToFile(_hFile,_dwVersion);

		}

		fflush(_hFile);
	}

	unguard;
}

//! confirm the fix whether change the original state
//! if just change ,(not add ),delete the parameter and return NULL
const Fix* FixList::ConfirmFix(const Fix& _fix)
{
	guard;


	for(std::list<Fix*>::iterator it = m_listFix.begin();
	it != m_listFix.end();
	it++){

		Fix* t_fix = *it;
		
		if(CompareTime(t_fix->GetBugTime(),_fix.GetBugTime()) == 0){
			if(t_fix->GetBugOwner() == _fix.GetBugOwner()){

				*t_fix = _fix;
				
				return NULL;
			}
		}

	}

	return &_fix;
	unguard;
}

BOOL FixList::ReadFromFile(FILE* _hFile)
{
	guard;

	osassert(_hFile);

	fseek(_hFile,0,SEEK_SET);

	const size_t t_read = fread(&m_header,sizeof(m_header),1,_hFile);

	if(t_read != 0){
		if(m_header.m_dwHeaderSize != sizeof(m_header)
		|| strcmp(m_header.m_szMagic,"FIX") != 0
		|| m_header.m_dwVersion < 0){

			return FALSE;

		}
	}else {
		return TRUE;
	}
	

	for(size_t i = 0;i< m_header.m_dwFixNum;i++){

		Fix* t_ptrFix = new Fix();

		t_ptrFix->ReadFromFile(_hFile,m_header.m_dwVersion);

		m_listFix.push_back(t_ptrFix);
	}

	return TRUE;

	unguard;
}
//! move all fix to another fix list
void FixList::MoveAllFixToAnother(FixList& _list)
{
	guard;
	
	for(std::list<Fix*>::iterator it = m_listFix.begin();
	it != m_listFix.end();
	it++){
		
		_list.AddRefFixPtr(*it);
	}
		
	m_listFix.clear();
	m_header.m_dwFixNum = 0;
	
	unguard;
}

//! add the ref bug
void FixList::AddRefFixPtr(Fix* _fix)
{
	guard;

	osassert(_fix);

	m_listFix.push_back(_fix);

	m_header.m_dwFixNum++;

	unguard;
}

//! find the right fix by the bug
Fix* FixList::FindRightFix(const Bug* _bug)
{
	guard;
	
	// the bug manager system kernel algorithm here
	// bug state
	//
	Fix* t_result = NULL;

	for(std::list<Fix*>::iterator it = m_listFix.begin();
	it != m_listFix.end();
	it++){
		
		Fix* t_fix = (*it);
					
		if(CompareTime(t_fix->GetBugTime(),_bug->GetBugTime()) == 0){

			if(t_fix->GetBugOwner() == AddBugDlg::GetTempBug().GetBugOwner()){
				
				t_result = t_fix;

				if(t_fix->GetBugState() == Fix::e_fixedOK){
					break;
				}

			}else if(t_fix->GetBugOwner() == _bug->GetBugOwner()){

				
				if(t_result){

					if(t_fix->GetBugState() == Fix::e_fixedOK){
						// find some fix is fixedOK flag
						//
						t_result = t_fix;

						break;

					}

					if((t_result->GetBugState() == Fix::e_isNotBug	|| t_result->GetBugState() == Fix::e_willNotFix)){

						if(t_fix->GetBugState() == Fix::e_reopen && (CompareTime(t_fix->GetFixTime(),t_result->GetFixTime()) > 0)){
							// the QA cannot agree this bug flag and set reopen flag.
							//
							t_result = t_fix;

						}
					}

				}else{

					t_result = t_fix;

					if(t_fix->GetBugState() == Fix::e_fixedOK){
						break;
					}
				}
			}
			
		}
	}

	return t_result;

	unguard;
}

//! Search the Fix pointer
Fix* FixList::SearchFix(const std::string& _strBugOwner,const SYSTEMTIME& _fixTime)
{
	guard;

	for(std::list<Fix*>::iterator it = m_listFix.begin();
	it != m_listFix.end();
	it++){

		Fix* t_fix = *it;

		if(CompareTime(t_fix->GetFixTime(),_fixTime) == 0){
			if(_strBugOwner == t_fix->GetBugOwner()){

				return t_fix;
			}
		}
	}

	return NULL;

	unguard;
}


//! search the Fix array
void FixList::SearchFix(const std::string& _strBugOwner,const SYSTEMTIME& _bugTime,Fix* _parrFix[],int& _fixNum)
{
	guard;

	int t_num = 0;

	for(std::list<Fix*>::iterator it = m_listFix.begin();
	it != m_listFix.end();
	it++){

		Fix* t_fix = *it;

		if(CompareTime(t_fix->GetBugTime(),_bugTime) == 0){

			if(_strBugOwner == t_fix->GetBugOwner()){

				_parrFix[t_num] = t_fix;

				t_num++;
				
			}
		}
	}

	_fixNum = t_num;


	unguard;
}

#endif //_BUG_COMMITER






















