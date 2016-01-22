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
	s_udp->writeDatagram(myArray.data(), qMin(myArray.size(), 1024), QHostAddress(HOST), PORT);
#endif
}

void UdpLog::setUsername(const QString &username)
{
	s_username = username;
}
