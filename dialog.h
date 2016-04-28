#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QListWidget;
class QUdpSocket;
class QPushButton;
class QListWidgetItem;
class QCheckBox;
class QLineEdit;
class QComboBox;
class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void slotReadPendingData();
    void slotTextChanged(const QString&);
    void slotCurrentIndexChanged(int index);
    void slotPause();
    void slotClear();
    void slotItemDoubleClicked(QListWidgetItem*);
    void slotVScrollBarValueChanged(int value);
	void slotScrollTimer();
	void slotSetMaxCount();

private:
    bool isItemHidden(const QString &text);
    void filterKey();

private:
    QListWidget *list_;
    QUdpSocket *udp_;
	QCheckBox *cbMaxCount_;
	QLineEdit *leMaxCount_;
    QComboBox *cbBox_;
    QLineEdit *leFilter_;
    QPushButton *pbPause_;
    QPushButton *pbClear_;

    int index_;
	QTimer *scrollTimer_;
    int maxCount_;
    int scrollBarOldValue_;
};

#endif // DIALOG_H
