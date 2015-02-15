#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QListWidget;
class QUdpSocket;
class QPushButton;
class QListWidgetItem;
class QCheckBox;
class QLineEdit;
class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void slotReadPendingData();
    void slotStateChanged(int);
    void slotTextChanged(const QString&);
    void slotPause();
    void slotClear();
    void slotItemDoubleClicked(QListWidgetItem*);

private:
    void filterShow(const QString &text);

private:
    QListWidget *list_;
    QUdpSocket *udp_;
    QCheckBox *cbFilter_;
    QLineEdit *leFilter_;
    QPushButton *pbPause_;
    QPushButton *pbClear_;
    int index_;
};

#endif // DIALOG_H
