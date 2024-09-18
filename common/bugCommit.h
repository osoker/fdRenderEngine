//=======================================================================================================
/*!  \file
*   Filename: bugCommit.h
*   Desc:     these functions is initialie the bug commit data and construct the dialog of committing 
*				dialog...
*
*   His:      tzz created @ 2009-1-14
*
*  
*/
//=======================================================================================================

# pragma once

#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <algorithm>
#include <assert.h>
#include <map>
#include <sstream>
#include <iomanip>


#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <direct.h>

//! bug commit bug dialog callback function...
BOOL CALLBACK BugManageDlgProc(HWND _hDlg, UINT _msg, WPARAM _wParam, LPARAM _lParam);

//! the bug commit callback function
BOOL CALLBACK BugCommitDlgProc(HWND _hDlg, UINT _msg, WPARAM _wParam, LPARAM _lParam);

//! the bug list dialog message proc
BOOL CALLBACK BugListDlgProc(HWND _hDlg,UINT _msg, WPARAM _wParam, LPARAM _lParam);

/*! \class the tab control 
*/
class BugTab
{
	//! the hwnd hand of tab
	HWND				m_hTab;

	//! ID
	UINT				m_uTabID;


	//! the page dialog HWND
	std::vector<HWND>	m_vPageWnd;

	//! the show index....
	size_t				m_iShowPageIndex;

public://constructor and destructor

	//! ...
	BugTab(void);
	~BugTab(void);

public: 

	//!initialize the tab
	void IniTab(HWND _hParentWnd,UINT _uTabID);

	//! add a page
	void AddPage(HWND _hDialogWnd,const char* _szDialogName);

	//! the handle convert function
	operator HWND()const{return m_hTab;}

public:

	//! message procedure
	BOOL MsgProc(HWND _hParentWnd,UINT _msg,WPARAM _wParam,LPARAM _lParam);

	//! show the dialog by the page index
	void ShowDialog(size_t _iPageIndex);




private:

	//! on notify message
	void ClickedNotify(HWND _hParentWnd,LPARAM _lParam);

	//! get the current selected index
	inline int GetCurSel()const 
	{
		if(!m_hTab){return -1;}
		return (int)::SendMessage(m_hTab, TCM_GETCURSEL, 0, 0L); 
	}

	//! add a tab unit
	inline int InsertItem(int _index,const char* _szName)const
	{
		if(!m_hTab){return -1;}

		TCITEM t_item;
		t_item.mask			= TCIF_TEXT  ;
		t_item.pszText		= (LPSTR)_szName;


		return (int)::SendMessage(m_hTab, TCM_INSERTITEM, _index, (LPARAM)&t_item);

	}

	//! set the dialog(index) position by the tab control
	void RefreshPagePos(int _iPageIndex);

	//! set the all dialog position by the tab's positiion
	void RefreshAllPage(void);


};
/*! \class bug data encapsulate the bug data
*/
class Bug
{

public:

	enum
	{
		e_unselected = -1,		//! unselected the initialize state

	};

	//! the severity enum
	enum Severity
	{
		e_cantTest_A		=0,		//! cannot test most high bug
		e_assert_B			=1,			//! assert bug
		e_major_C			=2,			//! major bug
		e_medium_D			=3,			//! medium bug
		e_minor_E			=4,			//! minor bug
		e_question_F		=5,		//! a question for GD, is a bug?
		e_suggest_G			=6,		//! a suggest for GD, artist

		e_unselected_sev = -1,//! unselected the initialize state
	};

	//! the location of bug
	enum Location
	{
		e_actionPhase,			//! the game is running
		e_all,					//! is a music bug
		e_documentation,		//! the text bug
		e_fixLocation,			//! some fix location
		e_gameMenu,				//! game menu or GUI
		e_loading,				//! when game loading
		e_gameScene,			//! in game Game scene (the map bug)

		e_unselected_loc = -1,		//! unselected the initialize state
	};

	//! reproduction rate
	enum ReproRate
	{

		e_easyRepro_knowRule,		//! easy reproduce and know the rule
		e_easyRepro_unknownRule,		//! easy reproduce and unknown the rule
		e_hardRepro_knowRule,		//! hard reproduce and know the rule
		e_hardRepro_unknownRule,		//! hard reproduce and unknown the rule

		e_unselected_repro = -1,//! unselected the initialize state
	};
private:

	//! the time data
	SYSTEMTIME			m_time;

	//! the bug owner
	std::string			m_strBugOwner;

	/*! game version

		is SVN subversion now

		next version may be major + minor
	*/
	DWORD				m_dwGameVersion;

	//! bug name (abstract of whole bug)
	std::string			m_strBugName;

	//! the severity and the interpret text
	enum Severity		m_severity;

	//! location
	enum Location		m_location;

	//! the relation bug
	SYSTEMTIME			m_relationBug;

	//! the reproduce rate
	enum ReproRate		m_reproRate;

	//! the log file name and contain
	std::string			m_strLogFilename;
	std::string			m_strLogFileContain;

	//! the desc of this bug
	std::string			m_strDesc;

	//! the reproduce step
	std::vector<std::string> m_vReproStep;


public: //constructor and destructor

	//!...
	Bug(void);
	~Bug(void);

	Bug(const Bug& _bug)
	{
		*this = _bug;
	}

public:

	//!copy the bug
	Bug& operator=(const Bug& _bug);

	//! read from the file
	void ReadFromFile(FILE* _hFile);

	//! write to the file
	void WriteToFile(FILE* _hFile,DWORD _dwVersion);

	//! new bug .... initialize the data to create a new bug
	void NewBug(const char* _bugOwner = NULL );


public:

	//! get bug name
	const std::string& GetBugName(void)const {return m_strBugName;}
	void SetBugName(const char* _szName)  {m_strBugName = _szName;}

	//! get bug time
	const SYSTEMTIME& GetBugTime(void)const {return m_time;}
	const char*	 GetBugTimeString(void)const ;
	
	//! get/set the bug owner
	const std::string& GetBugOwner(void)const {return m_strBugOwner;}
	void SetBugOwner(const char* _szOwner) {m_strBugOwner = _szOwner;}

	//! get/set the version of game
	const DWORD GetGameVersion(void)const {return m_dwGameVersion;}
	void SetGameVersion(DWORD _dwGameVersion)
	{
		assert(_dwGameVersion != 0 && _dwGameVersion != -1);
		m_dwGameVersion =  _dwGameVersion;
	}

	//! get/set the severity
	enum Severity GetSeverity(void)const {return m_severity;}
	void SetSeverity(enum Severity _s){m_severity = _s;}

	//! get/set the location
	enum Location GetLocation(void)const {return m_location;}
	void SetLocation(enum Location _l){m_location = _l;}

	//! get/set reproduce rate
	enum ReproRate GetReproRate(void)const {return m_reproRate;}
	void SetReproRate(enum ReproRate _r){m_reproRate = _r;}

	//! get/set the log filename
	const std::string& GetLogFilename(void)const {return m_strLogFilename;}
	void SetLogFilename(const char* _szFilename)
	{
		assert(_szFilename);
		m_strLogFilename = _szFilename;
	}

	//! get/set the log contain
	const std::string& GetLogContain(void)const {return m_strLogFileContain;}
	void SetLogContain(const char* _szLogContain)
	{
		assert(_szLogContain);
		m_strLogFileContain = _szLogContain;
	}

	//! get/set the describe text
	const std::string& GetDesc(void)const {return m_strDesc;}
	void SetDesc(const char* _szDesc)
	{
		assert(_szDesc);
		m_strDesc = _szDesc;
	}

	//! get the contain of step to reprocdure
	std::vector<std::string>& GetStepVect(void){return m_vReproStep;}

	//! get the whole bug note
	void GetWholeBugNote(std::string& _str)const;

	//! get the severity name int buglist
	const char* GetSeverityDesc(BOOL _bAbstract = TRUE)const;

	//! get the severity name in bug
	const char* GetLocationDesc(BOOL _bAbstract = TRUE)const;

	//! get the reproduce rate desc string
	const char* GetReproRateDesc(BOOL _bAbstract = TRUE)const ;


};

/*! \class fix data encapsulate the bug data
*/
class Fix
{
public:
	//! the style of fixed
	enum FixStyle
	{
		e_fixed			= 0,			//! fixed
		e_fixedOK		= 1,			//! fix OK....
		e_isNotBug		= 2,			//! is not bug (is design)
		e_willNotFix	= 3,			//! will not fix it
		e_reopen		= 4,			//! fix it again

		// just used by bug list filter
		//
		e_waiting		= 4,

		e_unselected_fix = -1,
	};

private:

	//! which bug has been fixed (for search the bug)
	SYSTEMTIME			m_bugTime;

	//! the bug owner (for search the bug)
	std::string			m_bugOwner;

	//! fixer name
	std::string			m_fixerName;

	//! how to fixed
	enum FixStyle		m_fixStyle;

	//! fix time
	SYSTEMTIME			m_fixTime;

	//! interpret string (why set the flag)
	std::string			m_strInterpret;

public: //constructor and destructor

	//!...
	Fix(void);
	~Fix(void);

	//! the copy constructor
	Fix(const Fix& _fix){
		*this = _fix;
	}

	Fix& operator = (const Fix& _fix);

public: 

	//! read from the file
	void ReadFromFile(FILE* _hFile,DWORD _dwVersion);

	//! write to the file
	void WriteToFile(FILE* _hFile,DWORD _dwVersion);

	//! get/set the bug state
	enum FixStyle GetBugState(void)const {return m_fixStyle;}
	void SetBugState(enum FixStyle _style){m_fixStyle = _style;}

	//! get/set fix time
	const SYSTEMTIME& GetFixTime(void)const {return m_fixTime;}
	SYSTEMTIME GetFixTime(void){return m_fixTime;}
	void SetFixTime(const SYSTEMTIME& _time){m_fixTime = _time;}

	//! get/set bug time
	const SYSTEMTIME& GetBugTime(void)const {return m_bugTime;}
	SYSTEMTIME GetBugTime(void){return m_bugTime;}
	void SetBugTime(const SYSTEMTIME& _time){m_bugTime = _time;}

	//! get/set the bug owner
	const std::string& GetBugOwner(void)const {return m_bugOwner;}
	void SetBugOwner(const char* _owner){m_bugOwner = _owner;}

	//! get/set the fixer name
	const std::string& GetFixerName(void)const {return m_fixerName;}
	void SetFixerName(const char* _fixer){m_fixerName = _fixer;}

	//! get the fix state string
	const char* GetFixStateString(void)const;

	//! get the fix whole description
	void GetFixDesc(std::string& _desc)const ;
	

	//! get and set the interpret string
	void SetInterpretString(const std::string& _str){m_strInterpret = _str;}
	void SetInterpretString(const char* _szInterpret){m_strInterpret = _szInterpret;}
	const std::string& GetInterpretString(void)const {return m_strInterpret;}
	std::string GetInterpretString(void){return m_strInterpret;}


};


/*! \class bug list
*/
class BugList
{
	/*! bug header struct
	*/
	struct  BugHeader
	{
		//! header magic "BUG\0"
		char		m_szMagic[4];

		//! size of header (to confirm)
		DWORD		m_dwHeaderSize;

		//! the version
		DWORD		m_dwVersion;

		//! the number of bug
		DWORD		m_dwBugNum;

	};

	//! the vector of bug (cu
	std::list<Bug*>		m_listBug;

	//! the bug header
	BugHeader				m_header;

public: //constructor and destructor

	//! ...
	BugList(void);
	~BugList(void);

public:

	//! add a bug(copy it) just in memory
	void AddBug(const Bug& _bug,FILE* _hFile = NULL,DWORD _dwVersion = 0);

	//! read from the file
	BOOL ReadFromFile(FILE* _hFile);

	//! Move the bug to the other list
	void MoveAllBugToOtherList(BugList& _destList);

	//! add reference bug pointer
	void AddRefBugPtr(Bug* _bug);

	//! get the list of bug
	const std::list<Bug*>& GetList(void)const {return m_listBug;}

};

//! the fix list
class FixList
{
	struct FixHeader 
	{
		//! header magic "FIX\0"
		char		m_szMagic[4];

		//! size of header (to confirm)
		DWORD		m_dwHeaderSize;

		//! the version
		DWORD		m_dwVersion;

		//! the fix number
		DWORD		m_dwFixNum;
	};
	
	//! the list of fix
	std::list<Fix*> m_listFix;

	//! the header of fix
	FixHeader		m_header;


public:

	FixList(void);
	~FixList(void);

	//! add a fix (or write to file?)
	void AddFix(const Fix& _fix,FILE* _hFile = NULL,DWORD _dwVersion = 0);

	//! read from the file
	BOOL ReadFromFile(FILE* _hFile);

	//! move all fix to another fix list
	void MoveAllFixToAnother(FixList& _list);

	//! add the ref bug
	void AddRefFixPtr(Fix* _fix);

	//! find the right fix by the bug
	Fix* FindRightFix(const Bug* _bug);

	//! Search the Fix pointer
	Fix* SearchFix(const std::string& _strBugOwner,const SYSTEMTIME& _fixTime);

	//! search the Fix array
	void SearchFix(const std::string& _strBugOwner,const SYSTEMTIME& _bugTime,Fix* _arrFix[],int& _fixNum);


private:
	//! confirm the fix whether change the original state
	//! if just change ,(not add ),delete the parameter and return NULL
	const Fix* ConfirmFix(const Fix& _fix);

};
//! the severity prompt string(and interpret string)
struct  BugSeverityString
{
	static const std::string	CantTest_A;	static const std::string CantTest_A_note;
	static const std::string 	Assert_B;	static const std::string Assert_B_note;
	static const std::string	Major_C;	static const std::string Major_C_note;
	static const std::string	Medium_D;	static const std::string Medium_D_note;
	static const std::string	Minor_E;	static const std::string Minor_E_note;
	static const std::string	Question_F;	static const std::string Question_F_note;
	static const std::string	Suggest_G;	static const std::string Suggest_G_note;

	static const size_t scm_maxStringLenth;	static const size_t scm_maxStringLenth_note;
};

//! the location prompt string (and interpret string)
struct BugLocationString
{
	static const std::string	ActionPhase;	static const std::string ActionPhase_note;
	static const std::string	All;			static const std::string All_note;
	static const std::string	Documentation;	static const std::string Documentation_note;
	static const std::string	Fix;			static const std::string Fix_note;
	static const std::string	GameMenu;		static const std::string GameMenu_note;
	static const std::string	Loading;		static const std::string Loading_note;
	static const std::string	GameScene;		static const std::string GameScene_note;

	static const int		scm_maxStringLenth;
};

//! the reproduce rate
struct BugReproRateString
{
	static const std::string	EasyReproKnowRule;
	static const std::string	EasyReproUnknownRule;
	static const std::string	HardReproKnowRule;
	static const std::string	HardReproUnknownRule;

	static const int		scm_maxStringLenth;
};

/*! \class add bug dialog window manage class
*/
class BugManage;
class AddBugDlg
{
	
	
private:
	//! the window's handle
	HWND			m_hDlg;

	//! the dialog ID
	UINT			m_uDlgID;

	//! current bug pointer
	Bug*			m_pCurrBug;

	//! the handle of severity combo box
	HWND			m_severityList;

	//! the note static text
	HWND			m_severityNote;

	//! the handle of location combo box
	HWND			m_locationList;

	//! the note of static location
	HWND			m_locationNote;

	//! the reproduce rate combo box
	HWND			m_reproList;

	//! the tmp bug
	static Bug		m_tmpBug;

	//! the tmp string to holde the log contain(whole contain)
	std::string		m_strLogContain;

public: //constructor and destrcutor

	//!...
	AddBugDlg(void);
	~AddBugDlg(void);

public:

	//! initialize the dialog
	void InitDialog(HINSTANCE _ins,BugManage* _bugMgr , UINT _uDlgID);

	//! message procedure
	BOOL MsgProcedure(UINT _msg,WPARAM _wParam ,LPARAM _lParam);

public:

	//! get the handle of window
	HWND GetHWND(void)const{return m_hDlg;}

	operator HWND()const {return m_hDlg;}

	//! get the temp bug
	static const Bug& GetTempBug(void){return m_tmpBug;}



private:

	//! the severity list item select changed
	void SeverityListSelChange(void);

	//! the location list item select changed
	void LocationListSelChange(void);

	//! the reproduce rate list select change...
	void ReproListSelChange(void);

	//! clicked the log file button
	void OnBugLogFile(void);

	//! get the bug log (assert) file contain
	const std::string& GetBugLogContain(const char* _szFilename);

	//! show the contain of log file
	void OnShowLogContain(void);

	//! on add a step string
	void OnAddStep(BOOL _bInsert);

	//! refresh the step list by the temp bug
	void RefreshStepList(int _curSel);

	//! delete the selected step;
	void OnDelStep(void);

	//! commit this bug
	void OnCommitBug(void);

	//! clear the add bug infomation in the control 
	void ClearBugInfo(void);



};

/*! \class bug list dialog window manage class
*/
class BugListDlg
{

public:

	//! the max column number;
	static const int		scm_iMaxColumn = 5;

	//! the enum of time filter
	enum TimeFilter
	{	
	
		e_oneDay	=0,
		e_twoDays	=1,
		e_threeDays	=2,
		e_oneWeek	=3,
		e_twoWeeks	=4,
		e_oneMonth	=5,

		e_unselected =-1,
	};

	struct ItemParam
	{
		//! the bug pointer...must be NOT NULL
		Bug*			m_bug;

		//! the fix pointer... may be NULL
		Fix*			m_fix;
	};

private:
	//! the window's handle
	HWND			m_hDlg;

	//! the handle of list view control
	HWND			m_hListView;

	//! the owner list filter handle
	HWND			m_hOwnerFilterList;

	//! the edit handle of filter string
	HWND			m_hFilterString;

	//! the bug state list handle
	HWND			m_hBugStateFilterList;

	//! the time list filter handle
	HWND			m_hTimeFilterList;

	//! the severity list filter handle
	HWND			m_hSeverityFilterList;

	//! the dialog ID
	UINT			m_uDlgID;

	//! current bug list 
	BugList			m_currBugList;

	//! current fix list 
	FixList			m_currFixList;

	//! the owner filter string
	std::string		m_strOwnerFilter;

	//! the search string filter 
	std::string		m_strFilterString;

	//! the time filter enum
	enum TimeFilter	m_timeFilter;

	Fix::FixStyle	m_bugStateFilter;

	//! the severity filter
	Bug::Severity	m_severityFilter;

	//! the allocator for the item parameter
	std::vector<ItemParam*> m_vectItemParam;


	

public: //constructor and destructor

	//!...
	BugListDlg(void);
	~BugListDlg(void);


public:

	//! initialize the dialog
	void InitDialog(HINSTANCE _ins,BugManage* _bugMgr , UINT _uDlgID);

	//! message procedure
	BOOL MsgProcedure(UINT _msg,WPARAM _wParam ,LPARAM _lParam);

	//! get the Bug(Fix) list
	BugList* GetBugList(void){return &m_currBugList;}

	//! get the fix list
	FixList* GetFixList(void){return &m_currFixList;}

	//! refresh the bug list view
	void RefreshListView(void);

	//! insert the bug owner filter string
	void InsertBugOwnerFilterString(const char* _bugOwner)const ;

	

public:

	//! get the handle of window
	HWND GetHWND(void)const{return m_hDlg;}

	operator HWND()const {return m_hDlg;}

private:

	//! insert a column
	int InsertColumn(int _index,const char* _szName,int _iWidth)const;

	//! compare the time to filter the bug by the time filter
	BOOL FilterPhase(const SYSTEMTIME& _bugTime,const SYSTEMTIME& _nowTime)const ;

	//! process the item sort
	void OnColumnClicked(LPNMLISTVIEW _pnm);

	//! process the item change
	void OnItemChange(LPNMLISTVIEW _pnm);

	//! process WM_CONTEXTMENU msg
	void OnContextMenu(POINT* _pos);

	//! process the menu item check bug command
	void OnCheckBug(void);

	//! process the display all button
	void OnDislayAll(void);

	//! process the statistic of bug
	void OnStatistic(void);

	//! on fix ok
	void OnQAFixedOK(void);

	//! on fix fixed;
	void OnFixFixed(void);

	//! on fix Not a Bug
	void OnFixNAB(void);
	
	//! on fix Will not Fix
	void OnFixWNF(void);

	//! search the filter string button clicked
	void OnFiliterStringBut(void);

	//! the filter list changed
	void OnSeverityFilterListChange();
	void OnTimeFilterListChange();
	void OnBugOwnerFilterListChange();
	void OnBugStateFilterListChange();

	//! enable/disable menu item by state of current user
	void SetMenuState(HMENU _menu,ItemParam* _itemData,BOOL _isQA);

	//! get (construct) the param ptr
	ItemParam* GetItemParamPtr(Bug* _bug);

	//! delete  all item param
	void DeleteAllItemParam(void);

	//! get list view item item param by Selected
	BugListDlg::ItemParam* GetItemParamInSel(void)const ;

};

/*! /class the bug manage class(global class)
*/
class BugManage
{

public:

	//! the name length of username and computer name
	static const int			scm_iUserNameLen		= 128;
	static const int			scm_iComputerNameLen	= 32; 

	//! the message to add a bug
	enum
	{
		//! commit a bug
		msgCommitBug = WM_USER + 1,
	};

	//! the version of bug commit
	static const DWORD			scm_dwBugSysVersion = 11;

private:
	//! the main dlg tab control
	BugTab			m_tab;

	//! add bug dialog frame
	AddBugDlg		m_addBugDlg;

	//! bug list dialog frame
	BugListDlg		m_bugListDlg;

	//! the handle of bug commit dialog(main dialog)
	HWND			m_hBugCommitDialog ;

	//! user name (count name in computer)
	char		m_szUserName[scm_iUserNameLen];

	//1 computer name
	char		m_szComputerName[scm_iComputerNameLen];

	//! the handle of bug list file
	FILE*			m_currBugListFile;

	//! the handle of fix list file
	FILE*			m_currFixListFile;

	//! the bug list class of current user
	BugList			m_currUserBugList;

	//! the fix list for current user
	FixList			m_currUserFixList;

	////! the vector of bug list file handle
	//std::vector<FILE*> m_vBugListFileHandle;

	////! the vector of bug list
	//std::vector<BugList*> m_vBugList;



public:

	BugManage(void);
	~BugManage(void);

	//! instance of whole programme to read the resource
	static HINSTANCE		sm_instance;

public:

	//! create window
	void CreateDlg(HINSTANCE _ins);

	//! show the dialog window
	void ShowDlg(BOOL _bShow);

	//! Main message procedure
	void MainMsgProcesure(UINT _msg,WPARAM _wParam ,LPARAM _lParam);

	//! add bug message procedure
	void AddBugMsgProc(UINT _msg,WPARAM _wParam,LPARAM _lParam)
	{
		m_addBugDlg.MsgProcedure(_msg,_wParam,_lParam);
	}

	//! bug list message procedure
	void BugListMsgProc(UINT _msg,WPARAM _wParam,LPARAM _lParam)
	{
		m_bugListDlg.MsgProcedure(_msg,_wParam,_lParam);
	}

	static void InitCommonCtrl(void);

	//! operator to convert the main dialog
	operator HWND()const{return m_hBugCommitDialog;}

	//! get the user name and computer name
	const std::string GetUserName(void)const {return std::string(m_szUserName);}
	const std::string GetComputerName(void)const {return std::string(m_szComputerName);}

	//! show the help dialog 
	void ShowHelpDlg(void);

	//! get the user fix file handle
	FILE* GetUserFixFileHandle(void)const {return m_currFixListFile;}

	//! add a fix to the user fix list
	void AddFixToUserFixFile(const Fix& _fix);

private:

	//!initialize the bug manage instance
	void InitInstance(HINSTANCE _ins);

	//! commit a bug from the add bug dialog
	void CommitBug(void);

	//!open (or create )a bug and fix file
	void OpenBugFixFile(void);

	

};
