========================================================================
    动态链接库 : fdrEngine 项目概况
========================================================================
River @ 2005-2-6: 整理旧的lib库渲染引擎为动态链接库的渲染引擎.
windy @ 2005-8-31 :关于增加坐骑的更新:
在角色I_skinMeshObj 中添加:SetRide和DelRide接口，如下:
	//!得到马的指针
	m_ptrRideObj = _ptrRideChar->get_chrInterface();
	//!设置马的指针，即上马
	m_ptrChrInterface->SetRide(m_ptrRideObj);
	//!下马
	m_ptrChrInterface->();
此时移动马，人物便跟随了。马的cha文件要升级到版本号301.由美术制作完成后转换。
其它程序创建过程不变.



