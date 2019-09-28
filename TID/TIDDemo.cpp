#include "stdafx.h"
#include "Buffer.h"
#include "LogFile.h"
//通过管道启动TID.exe
BOOL CreateTIDProcess( HANDLE hClientPipeRead, HANDLE hClientPipeWrite )
{
	//step1:获取荷载计算程序路径
	TCHAR cmd_str[MAX_PATH]="E:\\Structure\\TID\\Debug\\";
#ifndef _DEBUG
	GetSysPath(cmd_str);
#endif
	strcat(cmd_str,"TID.exe -child");

	STARTUPINFO startInfo;
	memset( &startInfo, 0 , sizeof( STARTUPINFO ) );
	startInfo.cb= sizeof( STARTUPINFO );
	startInfo.dwFlags |= STARTF_USESTDHANDLES;
	startInfo.hStdError= 0;
	startInfo.hStdInput= hClientPipeRead;
	startInfo.hStdOutput= hClientPipeWrite;

	PROCESS_INFORMATION processInfo;
	memset( &processInfo, 0, sizeof(PROCESS_INFORMATION) );

	BOOL b=CreateProcess( NULL,cmd_str,
		NULL,NULL, TRUE,CREATE_NEW_CONSOLE, NULL, NULL,&startInfo,&processInfo);
	DWORD er=GetLastError();
	return b;

}
BOOL WriteToTIDClient(HANDLE hPipeWrite)
{
	if( hPipeWrite == INVALID_HANDLE_VALUE )
		return FALSE;
	//TODO:需要完成查找需要的TID文件
	CString sFilePath="D:\\TID_FILE\\ZMC2.tid";
	CBuffer file_buffer(10000);
	file_buffer.WriteString(sFilePath);	//写入TID文件路径
	//
	return file_buffer.WriteToPipe(hPipeWrite,1024);
}
void RunTIDProcess()
{
	//创建第一个管道: 用于服务器端向客户端发送内容
	SECURITY_ATTRIBUTES attrib;
	attrib.nLength = sizeof( SECURITY_ATTRIBUTES );
	attrib.bInheritHandle= true;
	attrib.lpSecurityDescriptor = NULL;
	HANDLE hPipeClientRead=NULL, hPipeSrcWrite=NULL;
	if(!CreatePipe( &hPipeClientRead, &hPipeSrcWrite, &attrib, 0 ) )
	{
		logerr.Log("创建匿名管道失败!GetLastError= %d\n", GetLastError() );
		return;
	}
	HANDLE hPipeSrcWriteDup=NULL;
	if( !DuplicateHandle( GetCurrentProcess(), hPipeSrcWrite, GetCurrentProcess(), &hPipeSrcWriteDup, 0, false, DUPLICATE_SAME_ACCESS ) )
	{
		logerr.Log("复制句柄失败,GetLastError=%d\n", GetLastError() );
		return;
	}
	CloseHandle(hPipeSrcWrite);
	//创建第二个管道，用于客户端向服务器端发送内容
	HANDLE hPipeClientWrite=NULL, hPipeSrcRead=NULL;
	if( !CreatePipe( &hPipeSrcRead, &hPipeClientWrite, &attrib, 0) )
	{
		logerr.Log("创建第二个匿名管道失败,GetLastError=%d\n", GetLastError() );
		return;
	}
	HANDLE hPipeSrcReadDup=NULL;
	if( !DuplicateHandle( GetCurrentProcess(), hPipeSrcRead, GetCurrentProcess(), &hPipeSrcReadDup, 0, false, DUPLICATE_SAME_ACCESS ) )
	{
		logerr.Log("复制第二个句柄失败,GetLastError=%d\n", GetLastError() );
		return;
	}
	CloseHandle(hPipeSrcRead );
	//创建子进程,
	if(!CreateTIDProcess(hPipeClientRead,hPipeClientWrite))
	{
		logerr.Log("创建子进程失败\n" );
		return;
	}
	if(WriteToTIDClient(hPipeSrcWriteDup))
	{
		logerr.Log("数据传输失败");
		return;
	}
	if(logerr.IsHasContents())
		logerr.ShowToScreen();
}