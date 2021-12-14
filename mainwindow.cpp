#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <fstream>
#include <iostream>


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "./curl/libs/x86/Debug/libcrypto.lib")
#pragma comment(lib, "./curl/libs/x86/Debug/libcurl_a.lib")
#pragma comment(lib, "./curl/libs/x86/Debug/libssl.lib")


  




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();
	m_state = NONE;
}

MainWindow::~MainWindow()
{
    delete ui;
}


#include <thread>
#include <string>
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

typedef void(*DownLoadCallback)(int ContentSize, int CUR_LEN);

typedef struct _URL_INFO
{
    WCHAR szScheme[512];
    WCHAR szHostName[512];
    WCHAR szUserName[512];
    WCHAR szPassword[512];
    WCHAR szUrlPath[512];
    WCHAR szExtraInfo[512];
}URL_INFO, *PURL_INFO;


void dcallback(int ContentSize, int file_size)
{
    qDebug() << "count: " << ContentSize << "  file_size size:" << file_size;
}

//
//void winhttpCallback(HINTERNET hInternet,DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation OPTIONAL,DWORD dwStatusInformationLength)
//{
//	qDebug() << "1111111";
//}

void download(std::string fileUrl, std::string localfile/*,QByteArray &byt*/, DownLoadCallback Func)
{
    //url
    int urlLen = (int)(fileUrl.length()+1);
    wchar_t* Url = new wchar_t[urlLen];
    MultiByteToWideChar( CP_ACP, 0, fileUrl.c_str(), -1, Url, urlLen);

    //filename
    urlLen = (int)(localfile.length()+1);
    wchar_t* FileName = new wchar_t[urlLen];
    MultiByteToWideChar( CP_ACP, 0, localfile.c_str(), -1, FileName, urlLen);


    URL_INFO url_info = { 0 };
    URL_COMPONENTSW lpUrlComponents = { 0 };
    lpUrlComponents.dwStructSize = sizeof(lpUrlComponents);
    lpUrlComponents.lpszExtraInfo = url_info.szExtraInfo;
    lpUrlComponents.lpszHostName = url_info.szHostName;
    lpUrlComponents.lpszPassword = url_info.szPassword;
    lpUrlComponents.lpszScheme = url_info.szScheme;
    lpUrlComponents.lpszUrlPath = url_info.szUrlPath;
    lpUrlComponents.lpszUserName = url_info.szUserName;

    lpUrlComponents.dwExtraInfoLength =
        lpUrlComponents.dwHostNameLength =
        lpUrlComponents.dwPasswordLength =
        lpUrlComponents.dwSchemeLength =
        lpUrlComponents.dwUrlPathLength =
        lpUrlComponents.dwUserNameLength = 512;

    WinHttpCrackUrl(Url, 0, ICU_ESCAPE, &lpUrlComponents);

    // 创建一个会话
    HINTERNET hSession = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
    DWORD dwReadBytes, dwSizeDW = sizeof(dwSizeDW), dwContentSize, dwIndex = 0;
#if 0
	WINHTTP_STATUS_CALLBACK isCallback = WinHttpSetStatusCallback(hSession,
		(WINHTTP_STATUS_CALLBACK)winhttpCallback,
		WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
		NULL);
#endif

#if 0
    // 创建一个连接
    HINTERNET hConnect = WinHttpConnect(hSession, lpUrlComponents.lpszHostName, lpUrlComponents.nPort, 0);
    // 创建一个请求，先查询内容的大小
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"HEAD", lpUrlComponents.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    WinHttpReceiveResponse(hRequest, 0);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwContentSize, &dwSizeDW, &dwIndex);
	WinHttpCloseHandle(hRequest);

    // 创建一个请求，获取数据
    hRequest = WinHttpOpenRequest(hConnect, L"GET", lpUrlComponents.lpszUrlPath, L"HTTP/1.1", WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest, 0);

	DWORD dwSize;
	WCHAR* lpOutBuffer = NULL;
	BOOL  bResults = FALSE;
	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
		WINHTTP_HEADER_NAME_BY_INDEX, NULL,
		&dwSize, WINHTTP_NO_HEADER_INDEX);


	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		lpOutBuffer = new WCHAR[dwSize / sizeof(WCHAR)];

		// Now, use WinHttpQueryHeaders to retrieve the header.
		bResults = WinHttpQueryHeaders(hRequest,
			WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX,
			lpOutBuffer, &dwSize,
			WINHTTP_NO_HEADER_INDEX);
		
		wchar_t * wText = lpOutBuffer;
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用
		char *psText; // psText为char*的临时数组，作为赋值给std::string的中间变量
		psText = new char[dwNum];
		WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用
		
		//获取http错误码
		int code = -1;
		char *p = strstr(psText, "HTTP/");
		if (p)
			sscanf(p, "%*s %d", &code);
		{
			if (code != 200)
			{
				return;
			}
		}
		delete[]psText;// psText的清除
	}

	//开始下载
    // 分段回调显示进度
    DWORD BUF_LEN = 1024, ReadedLen = 0;
    BYTE *pBuffer = NULL;
    pBuffer = new BYTE[BUF_LEN];

    HANDLE hFile = CreateFileW(FileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    while (dwContentSize > ReadedLen)
    {
        ZeroMemory(pBuffer, BUF_LEN);
        WinHttpReadData(hRequest, pBuffer, BUF_LEN, &dwReadBytes);
        ReadedLen += dwReadBytes;

        //byt.append((const char*)pBuffer,dwReadBytes);
        // 写入文件
        WriteFile(hFile, pBuffer, dwReadBytes, &dwReadBytes, NULL);
        // 进度回调
        Func(dwContentSize, ReadedLen);
    }

    CloseHandle(hFile);
    delete []pBuffer;

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    delete [] Url;
    delete [] FileName;
#endif
}

///////////////////////////////////////

int dataCount = 0;
int realdata = 0;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
	MainWindow *mem = (MainWindow *)userp;

	dataCount += realsize;

	char* tryread = (char*)contents;

	if (mem->getDownloadState() == MainWindow::REQUEST_HEADER)
	{
		char *p = strstr((char*)contents, "HTTP/");
		if (p)
		{
			int code = -1;
			sscanf(p, "%*s %d", &code);
			if (code != -1)
			{
				mem->onResponseCode(code);
			}
		}
		p = strstr((char*)contents, "Content-Length");
		if (p)
		{
			int length = -1;
			sscanf(p, "%*s %d", &length);
			if (length != -1)
			{
				mem->onGetContentLength(length);
			}
		}
		tryread;
		if (std::string("\r\n").compare((char*)contents) == 0)
		{
			mem->onHeaderFinish();
		}
		
	}
	else {
		qDebug() << "bbbbb";
		mem->onWritable(contents, size, nmemb);
	}
	qDebug() << "dataCount: " << dataCount;

	

    return realsize;
}

static size_t ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	MainWindow *holeder = (MainWindow *)clientp;
	qDebug() << "ProgressCallback";
	//return holeder->getDownloadState() != HttpDownloader::STOPED;
	return 0;
}

static size_t WriteHeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{

#if 0
	MainWindow *mem = (MainWindow *)userp;
	
	char* readable = (char*)contents;

	char *p = strstr((char*)contents, "HTTP/");
	if (p)
	{
		int code = -1;
		sscanf(p, "%*s %d", &code);
		if (code != -1)
		{
			mem->onResponseCode(code);
		}
	}
	p = strstr((char*)contents, "Content-Length");
	if (p)
	{
		int length = -1;
		sscanf(p, "%*s %d", &length);
		if (length != -1)
		{
			mem->onGetContentLength(length);
		}
	}
	
	if (std::string("/r/n").compare((char*)contents) == 0)
	{
		mem->onHeaderFinish();
	}
		
#endif

	return nmemb* size;

}


////////////////////////////////////////


void MainWindow::init()
{
    CURLcode error;

    curl_global_init(CURL_GLOBAL_ALL);
    /* init the curl session */
    curl_handle = curl_easy_init();
    ///* specify URL to get */
    //curl_easy_setopt(curl_handle, CURLOPT_URL, const_cast<char*>(strUrl.c_str()));
    ////curl_easy_setopt(curl_handle, CURLOPT_URL, "http://10.66.91.15:7777/ld/smog/2612_src.jpg");


	curl_easy_setopt(curl_handle, CURLOPT_HEADER, this);
	curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);


    /* send all data to this function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)this);


	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
	

    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);


    /* some servers don't like requests that are made without a user-agent
    field, so we provide one */
    //curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // https, skip the verification of the server's certificate.
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    /* 设置连接超时,单位:毫秒 */
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 1000L);

    // add by yexiaoyogn 10 second time out
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 2000);

    //add yexiaoyong set time out
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 3);
}



void MainWindow::Download(const std::string& url, const std::string& localfilepath)
{
    CURLcode res;

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, const_cast<char*>(url.c_str()));
    //curl_easy_setopt(curl_handle, CURLOPT_URL, "http://10.66.91.15:7777/ld/smog/2612_src.jpg");


    /* get it! */
    res = curl_easy_perform(curl_handle);
    /* check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else {
        /*
        * Now, our chunk.memory points to a memory block that is chunk.size
        * bytes big and contains the remote file.
        *
        * Do something nice with it!
        */
    }

    //调用回调
}

void MainWindow::Finish()
{
    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
}



void MainWindow::on_pushButton_clicked()
{
	if (m_download_thread.joinable())
		m_download_thread.join();


#if 0

	std::string src = "http://zego-ecommerce.oss-cn-shanghai.aliyuncs.com/room/sdGjMhAmwG6pQlSZMlmzdsL0byu3111.jpeg";
	auto it = src.end() - 1;
	while (*it != '/')
	{
		it--;
	}
	std::string filename = src.assign(it + 1, src.end());


	download(src, "1.jpg", dcallback);

#endif

	//https://ss0.bdstatic.com/5aV1bjqh_Q23odCf/static/superman/img/logo/bd_logo1_31bdc765.png
	std::string src = "http://zego-ecommerce.oss-cn-shanghai.aliyuncs.com/room/sdGjMhAmwG6pQlSZMlmzdsL0byu3.jpeg";


	//std::string src = "https://ss0.bdstatic.com/5aV1bjqh_Q23odCf/static/superman/img/logo/bd_logo1_31bdc7651112.png";

	if (src.substr(0, 4).compare("http") == 0 || src.substr(0, 5).compare("https"))
	{
		qDebug() << "1111";
	}
	TCHAR szPathCopy[_MAX_PATH];
	//获取临时目录路径
	GetTempPath(_MAX_PATH, szPathCopy);
	int iLen = WideCharToMultiByte(CP_ACP, 0, szPathCopy, -1, NULL, 0, NULL, NULL);   //首先计算TCHAR 长度。
	char* chRtn = new char[iLen * sizeof(char)];  //定义一个 TCHAR 长度大小的 CHAR 类型。
	WideCharToMultiByte(CP_ACP, 0, szPathCopy, -1, chRtn, iLen, NULL, NULL);  //将TCHAR 类型的数据转换为 CHAR 类型。
	//文件路径
	std::string filepath(chRtn); //最后将CHAR 类型数据 转换为 STRING 类型数据
	//文件名字
	auto it = src.end() - 1;
	while (*it != '/')
	{
		it--;
	}
	auto filename = src;
	filename.assign(it + 1, src.end());
	filepath += filename;
	m_destfilepath = filepath;

	{
		std::ofstream outfile;
		outfile.open(m_destfilepath, std::ios::trunc);
		outfile.close();
	}



	m_state = REQUEST_HEADER;
	qDebug() << "filepath: " << m_destfilepath.c_str();


	dataCount = 0;
	realdata = 0;

	//获取temp目录


	m_download_thread = std::thread(&MainWindow::Download, this, src, m_destfilepath);
    //Download(src, m_destfilepath);
}




void MainWindow::onResponseCode(int code)
{
	//m_state = REQUEST_DATA;
}

void MainWindow::onHeaderFinish()
{
	m_state = REQUEST_DATA;
}


void MainWindow::onGetContentLength(int length)
{
	m_file_size = length;
}




//写文件
void MainWindow::onWritable(void *contents, size_t size, size_t nmemb)
{
	realdata += size * nmemb;
	qDebug() << "realdata write data: " << realdata;
	qDebug() << "m_file_size: " << m_file_size;
	std::ofstream outfile;
	outfile.open(m_destfilepath, std::ios::out | std::ios::binary | std::ios::app);
	outfile.write((char*)contents, size*nmemb);
	outfile.close();
}
