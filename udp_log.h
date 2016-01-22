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


