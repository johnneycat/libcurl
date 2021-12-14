#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <thread>
#include <curl.h>
#include <functional>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



struct MemoryStruct
{
    char *memory;
    size_t size;
};

using Callback = std::function<void(const MemoryStruct &m)>;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	enum DOWNLOAD_STATE {
		NONE,
		REQUEST_HEADER,
		HEADER_FINISHED,
		REQUEST_DATA,
	};


    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

	DOWNLOAD_STATE getDownloadState() {
		return m_state;
	}

	void onHeaderFinish();
	void onWritable(void *contents, size_t size, size_t nmemb);


	void onResponseCode(int code);
	void onGetContentLength(int length);


private slots:
    void on_pushButton_clicked();
    void init();
    void Download(const std::string& url, const std::string& localfilepath);
    void Finish();

private:
    Ui::MainWindow *ui;

    CURL *curl_handle;
    Callback callback;

	

	DOWNLOAD_STATE	m_state;
	std::string		m_destfilepath;
	int64_t			m_file_size = 0;

	std::thread     m_download_thread;

};
#endif // MAINWINDOW_H
