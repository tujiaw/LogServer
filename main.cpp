#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w;
    w.show();

    return a.exec();
}

/////////////////////////////////////////////////////////
//LogClient
#pragma once

#include <QTextStream>

class UdpLog
{
public:
	UdpLog(const QString &file, quint32 line);
	~UdpLog();

	QTextStream& stream() { return stream_; }
	static void setUsername(const QString &username);

private:
	QString buffer_;
	QTextStream stream_;
};

#define ULOG UdpLog(__FILE__, __LINE__).stream()

//////////
#include "udp_log.h"
#include <QtNetwork/QUdpSocket>
#include <QHostInfo>
#include <QDateTime>

static const QString HOST = "127.0.0.1";
static const unsigned short PORT = 5566;
static QUdpSocket *s_udp = NULL;
static QString s_username;

UdpLog::UdpLog(const QString &file, quint32 line)
	: stream_(&buffer_, QIODevice::Append)
{
#ifdef NDEBUG
#else
	if (NULL == s_udp) {
		s_udp = new QUdpSocket();
	}
	stream_ << s_username << "[" << QDateTime::currentDateTime().toString("HH:mm:ss.zzz") << "," << file.section("\\", -1) << "," << line << "] ";
#endif
}

UdpLog::~UdpLog()
{
#ifdef NDEBUG
#else
	QByteArray myArray = buffer_.toUtf8();
	s_udp->writeDatagram(myArray.data(), myArray.size(), QHostAddress(HOST), PORT);
#endif
}

void UdpLog::setUsername(const QString &username)
{
	s_username = username;
}




