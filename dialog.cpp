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

static unsigned short PORT = 5566;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), index_(0)
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

	m_maxCount = 5000;
	leMaxCount_ = new QLineEdit(this);
	leMaxCount_->setFixedWidth(70);
	leMaxCount_->setText(QString::number(m_maxCount));
	QRegExp regx("[0-9]+$");
	QValidator *validator = new QRegExpValidator(regx, leMaxCount_);
	leMaxCount_->setValidator(validator);
	connect(leMaxCount_, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));

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
	bottomLayout->addWidget(cbMaxCount_);
	bottomLayout->addWidget(leMaxCount_);
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

		if (cbMaxCount_->isChecked() && list_->count() >= m_maxCount) {
			for (int i=0; i<m_maxCount/10; i++) {
				QListWidgetItem *item = list_->takeItem(0);
				delete item;
			}
		}

        QListWidgetItem *newItem = new QListWidgetItem(QString::number(++index_) + " - " + QString::fromUtf8(data));
        list_->addItem(newItem);
        if (!scrollTimer_->property("stop").toInt()) {
            scrollTimer_->start();
        }
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
	if (sender() == leFilter_) {
		if (cbFilter_->isChecked()) {
			filterShow(text);
		}
	} else if (sender() == leMaxCount_) {
		QTimer::singleShot(3000, this, SLOT(slotSetMaxCount()));
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

void Dialog::slotVScrollBarValueChanged(int value)
{
    // 如果鼠标滚轮往上则暂停自动滚动到底部，鼠标滚轮滚动到底部则开启自动滚动到底部
    static int s_oldValue = 0;
    if (value < s_oldValue) {
        scrollTimer_->setProperty("stop", 1);
    } else {
        if (scrollTimer_->property("stop").toInt()) {
            if (value == list_->verticalScrollBar()->maximum()) {
                scrollTimer_->setProperty("stop", 0);
            }
        }
    }
    s_oldValue = value;
}

void Dialog::slotScrollTimer()
{
	list_->scrollToBottom();
	scrollTimer_->stop();
}

void Dialog::slotSetMaxCount()
{
	m_maxCount = leMaxCount_->text().toInt();
	m_maxCount = qMax(10, m_maxCount);
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

