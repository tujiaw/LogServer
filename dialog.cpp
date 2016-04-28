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
#include <QTime>
#include <QTimer>
#include <QScrollBar>
#include <QComboBox>

static unsigned short PORT = 5566;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), index_(0), scrollBarOldValue_(0)
{
	Qt::WindowFlags flags = Qt::Dialog;
	flags |= Qt::WindowMinimizeButtonHint;
	flags |= Qt::WindowMaximizeButtonHint;
	flags |= Qt::WindowCloseButtonHint;
	this->setWindowFlags(flags);

	scrollTimer_ = new QTimer();
	scrollTimer_->setInterval(300);
	connect(scrollTimer_, &QTimer::timeout, this, &Dialog::slotScrollTimer);

    list_ = new QListWidget(this);
    connect(list_, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));
    connect(list_->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotVScrollBarValueChanged(int)));

	cbMaxCount_ = new QCheckBox("MaxCount", this);
	cbMaxCount_->setChecked(true);

    maxCount_ = 5000;
	leMaxCount_ = new QLineEdit(this);
	leMaxCount_->setFixedWidth(70);
    leMaxCount_->setText(QString::number(maxCount_));
	QRegExp regx("[0-9]+$");
	QValidator *validator = new QRegExpValidator(regx, leMaxCount_);
	leMaxCount_->setValidator(validator);
	connect(leMaxCount_, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));

    cbBox_ = new QComboBox(this);
    cbBox_->addItems(QStringList() << "ShowKey" << "HideKey");
    connect(cbBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));

    leFilter_ = new QLineEdit(this);
    leFilter_->setClearButtonEnabled(true);
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
	bottomLayout->addWidget(cbMaxCount_);
	bottomLayout->addWidget(leMaxCount_);
    bottomLayout->addWidget(cbBox_);
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

        if (cbMaxCount_->isChecked() && list_->count() >= maxCount_) {
            for (int i=0; i<maxCount_/10; i++) {
				QListWidgetItem *item = list_->takeItem(0);
				delete item;
			}
		}

        QListWidgetItem *newItem = new QListWidgetItem(QString::number(++index_) + " - " + QString::fromUtf8(data));
        list_->addItem(newItem);
        if (!scrollTimer_->property("stop").toInt()) {
            scrollTimer_->start();
        }

        bool isHidden = isItemHidden(newItem->text().trimmed());
        newItem->setHidden(isHidden);
    }
}

void Dialog::slotTextChanged(const QString &text)
{
    if (sender() == leFilter_) {
        filterKey();
    } else if (sender() == leMaxCount_) {
		QTimer::singleShot(3000, this, SLOT(slotSetMaxCount()));
	}
}

void Dialog::slotCurrentIndexChanged(int index)
{
    filterKey();
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
    scrollBarOldValue_ = 0;
    list_->clear();
}

void Dialog::slotItemDoubleClicked(QListWidgetItem *item)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(item->text());
}

void Dialog::slotVScrollBarValueChanged(int value)
{
    // 如果鼠标滚轮往上则暂停自动滚动到底部，鼠标滚轮滚动到底部则开启自动滚动到底部
    if (value < scrollBarOldValue_) {
        scrollTimer_->setProperty("stop", 1);
    } else {
        if (scrollTimer_->property("stop").toInt()) {
            if (value == list_->verticalScrollBar()->maximum()) {
                scrollTimer_->setProperty("stop", 0);
            }
        }
    }
    scrollBarOldValue_ = value;
}

void Dialog::slotScrollTimer()
{
	list_->scrollToBottom();
	scrollTimer_->stop();
}

void Dialog::slotSetMaxCount()
{
    maxCount_ = leMaxCount_->text().toInt();
    maxCount_ = qMax(10, maxCount_);
}

bool Dialog::isItemHidden(const QString &text)
{
    bool isHidden = false;
    QString filterText = leFilter_->text().trimmed();
    if (cbBox_->currentIndex() == 0) {
        if (!filterText.isEmpty() && !text.contains(filterText)) {
            isHidden = true;
        }
    } else if (cbBox_->currentIndex() == 1) {
        if (!filterText.isEmpty() && text.contains(filterText)) {
            isHidden = true;
        }
    }
    return isHidden;
}

void Dialog::filterKey()
{
    for (int i=0; i<list_->count(); i++) {
        QListWidgetItem *item = list_->item(i);
        bool isHidden = isItemHidden(item->text().trimmed());
        item->setHidden(isHidden);
    }
}


