#include "dialog.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtNetwork/QUdpSocket>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>

static unsigned short PORT = 5566;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), index_(0)
{
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::WindowMaximizeButtonHint;
	flags |= Qt::WindowCloseButtonHint;
	this->setWindowFlags(flags);

    list_ = new QListWidget(this);
    list_->setAutoScroll(true);
    connect(list_, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));

    pbPause_ = new QPushButton("Stop", this);
    pbPause_->setCheckable(true);
    pbPause_->setChecked(false);
    connect(pbPause_, SIGNAL(clicked()), this, SLOT(slotPause()));

    pbClear_ = new QPushButton("Clear", this);
    connect(pbClear_, SIGNAL(clicked()), this, SLOT(slotClear()));

    udp_ = new QUdpSocket(this);
    udp_->bind(QHostAddress::LocalHost, PORT);
    connect(udp_, SIGNAL(readyRead()), this, SLOT(slotReadPendingData()));

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(pbPause_);
    bottomLayout->addWidget(pbClear_);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(list_);
    mainLayout->addLayout(bottomLayout);

    this->setLayout(mainLayout);
    this->setMinimumSize(800, 600);
}

Dialog::~Dialog()
{
}


void Dialog::slotReadPendingData()
{
    while (udp_->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(udp_->pendingDatagramSize());
        QHostAddress senderHost;
        quint16 senderPort;
        udp_->readDatagram(data.data(), data.size(), &senderHost, &senderPort);
        if (pbPause_->isChecked()) {
            continue;
		}
		
		list_->addItem(QString::number(++index_) + QString::fromUtf8(data));
        list_->setCurrentRow(list_->count() - 1);
    }
}

void Dialog::slotPause()
{
    if (pbPause_->isChecked()) {
        pbPause_->setText("Start");
    } else {
        pbPause_->setText("Stop");
    }
}

void Dialog::slotClear()
{
    index_ = 0;
    list_->clear();
}

void Dialog::slotItemDoubleClicked(QListWidgetItem *item)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(item->text());
}
