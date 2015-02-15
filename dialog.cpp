#include "dialog.h"
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtNetwork/QUdpSocket>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
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

    cbFilter_ = new QCheckBox("Filter", this);
    cbFilter_->setChecked(true);
    connect(cbFilter_, SIGNAL(stateChanged(int)), this, SLOT(slotStateChanged(int)));

    leFilter_ = new QLineEdit(this);
    connect(leFilter_, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));

    pbPause_ = new QPushButton("Stop", this);
    pbPause_->setCheckable(true);
    pbPause_->setChecked(false);
    connect(pbPause_, SIGNAL(clicked()), this, SLOT(slotPause()));

    pbClear_ = new QPushButton("Clear", this);
    connect(pbClear_, SIGNAL(clicked()), this, SLOT(slotClear()));

    udp_ = new QUdpSocket(this);
    udp_->bind(QHostAddress::Any, PORT);
    connect(udp_, SIGNAL(readyRead()), this, SLOT(slotReadPendingData()));

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(cbFilter_);
    bottomLayout->addWidget(leFilter_);
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
        udp_->readDatagram(data.data(), udp_->pendingDatagramSize(), &senderHost, &senderPort);
        if (pbPause_->isChecked()) {
            continue;
		}
		
        QListWidgetItem *newItem = new QListWidgetItem(QString::number(++index_) + " - " + QString::fromUtf8(data));
        list_->addItem(newItem);
        list_->setCurrentRow(list_->count() - 1);
        if (cbFilter_->isChecked() && !leFilter_->text().isEmpty() && !newItem->text().contains(leFilter_->text())) {
            newItem->setHidden(true);
        }
    }
}

void Dialog::slotStateChanged(int state)
{
    leFilter_->setEnabled(state == Qt::Checked);
    filterShow(state == Qt::Checked ? leFilter_->text() : "");
}

void Dialog::slotTextChanged(const QString &text)
{
    if (cbFilter_->isChecked()) {
        filterShow(text);
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

void Dialog::filterShow(const QString &text)
{
    for (int i=0; i<list_->count(); i++) {

        QListWidgetItem *item = list_->item(i);
        if (text.trimmed().isEmpty())  {
            item->setHidden(false);
        } else {
            item->setHidden(!item->text().contains(text));
        }
    }
}

