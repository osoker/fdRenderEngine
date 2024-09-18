// 下列 ifdef 块是创建使从 DLL 导出更简单的
//宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 COMMON_EXPORTS
// 符号编译的。在使用此 DLL 的
//任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将 
// COMMON_API 函数视为是从此 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
# pragma once

#ifdef COMMON_EXPORTS
	#ifdef _USE_COMMON_STATIC_LIB
		// some project use the common static lib
		//
		#define COMMON_API 
	#else
		#define COMMON_API __declspec(dllexport)
	#endif _USE_STATIC_LIB
#else
	#ifdef _USE_COMMON_STATIC_LIB
		// some project use the common static lib
		//
		#define COMMON_API 
	#else
		#define COMMON_API __declspec(dllimport)
	#endif //_USE_STATIC_LIB
#endif
