#ifndef CK2ILOGWINDOW_H
#define CK2ILOGWINDOW_H

#include <QMainWindow>
#include <QString>
#include "DataUtils/dataparser.h"
#include "DataUtils/fileio.h"

namespace Ui {
class CK2iLogWindow;
}

class CK2iLogWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CK2iLogWindow(QWidget *parent = 0);
    ~CK2iLogWindow();
public slots:
    void displayLog(QString log_msg);
private slots:

    void on_PickDecisionSourceBtn_clicked();

    void on_DecisionLogStartBtn_clicked();

    void on_PickEventSourceBtn_clicked();

    void on_EventLogStartBtn_clicked();

    void on_ScanLogfileBtn_clicked();

private:
    Ui::CK2iLogWindow *ui;

    // decisions
    QString decision_dir;
    liDirectoryConverter* decision_converter;
    int decisionf_count;
    // events
    QString event_dir;
    liDirectoryConverter* event_converter;
    int eventf_count;
};

#endif // CK2ILOGWINDOW_H
