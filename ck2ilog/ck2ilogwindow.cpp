#include "ck2ilogwindow.h"
#include "ui_ck2ilogwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <assert.h>
#ifndef SCANRES_H
   #include "DataUtils/scanres.h"
#endif // SCANRES_H

CK2iLogWindow::CK2iLogWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CK2iLogWindow)
{
    ui->setupUi(this);

    decision_converter = NULL;
    decisionf_count = 0;
    event_converter = NULL;
    eventf_count = 0;

}

CK2iLogWindow::~CK2iLogWindow()
{
    delete ui;
}

void CK2iLogWindow::displayLog(QString log_msg) {
    ui->log_display->appendPlainText(log_msg);
}


void CK2iLogWindow::on_PickDecisionSourceBtn_clicked() {
    int loadcount;
    QString msgtext;
    decision_dir = QFileDialog::getExistingDirectory(this,"Pick the directory of CK2 decision files",
                         QString(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // empty result
    if (decision_dir.isEmpty()) {
        ui->DecisionDirInfoLabel->setText("No Directory Picked!");
    }
    // non-empty result
    else {
        if (decision_converter != NULL) {
            disconnect(decision_converter,SIGNAL(RelayLog(QString)),this,SLOT(displayLog(QString)));
            delete decision_converter;
        }
        // creating the converter and setting it's director
        decision_converter = new liDirectoryConverter(false,false);
        connect(decision_converter,SIGNAL(RelayLog(QString)),this,SLOT(displayLog(QString)));
        loadcount = decision_converter->SetDirectory(decision_dir);
        if (loadcount > 0) {
            msgtext = "Converting " + QString::number(loadcount) + " files in ";
            msgtext += decision_dir;
            ui->DecisionDirInfoLabel->setText(msgtext);
            // setting the widgets for the loop
            ui->DecisionLogProgressBar->setMaximum(loadcount);
            ui->CurrentDecisionLabel->setText("Not Started");
            ui->DecisionLogStartBtn->setEnabled(true);
        }
        else {
            msgtext = "No text files to convert in ";
            msgtext += decision_dir;
            ui->DecisionDirInfoLabel->setText(msgtext);
        }
    }
}

void CK2iLogWindow::on_DecisionLogStartBtn_clicked() {
    assert(decision_converter != NULL);
    ui->PickDecisionSourceBtn->setEnabled(false);
    ui->DecisionLogStartBtn->setEnabled(false);
    // variables
    QString cfile,xmsg,outerr;
    bool cresult;
    // looping over the files
    for (size_t fildex = 0; fildex < (decision_converter->Filecount()); fildex++) {
        cfile = decision_converter->CurrentFile();
        xmsg = "Checking file: " + cfile;
        ui->CurrentDecisionLabel->setText(xmsg);
        ui->log_display->clear();
        cresult = decision_converter->DoCurrentFile(outerr);
        if (!cresult) {
            xmsg = "File failed: " + cfile + " : " + outerr;
            break;
        }
        else {
            ui->DecisionLogProgressBar->setValue(fildex+1);
        }
    }
}

void CK2iLogWindow::on_PickEventSourceBtn_clicked() {
    int loadcount;
    QString msgtext;
    event_dir = QFileDialog::getExistingDirectory(this,"Pick the directory of CK2 event files",
                         QString(),QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // empty result
    if (event_dir.isEmpty()) {
        ui->EventDirInfoLabel->setText("No Directory Picked!");
    }
    // non-empty result
    else {
        if (event_converter != NULL) {
            disconnect(event_converter,SIGNAL(RelayLog(QString)),this,SLOT(displayLog(QString)));
            delete event_converter;
        }
        // creating the converter and setting it's director
        event_converter = new liDirectoryConverter(ui->InsertInTriggersCB->isChecked());
        connect(event_converter,SIGNAL(RelayLog(QString)),this,SLOT(displayLog(QString)));
        loadcount = event_converter->SetDirectory(event_dir);
        if (loadcount > 0) {
            msgtext = "Converting " + QString::number(loadcount) + " files in ";
            msgtext += event_dir;
            ui->EventDirInfoLabel->setText(msgtext);
            // setting the widgets for the loop
            ui->EventLogProgressBar->setMaximum(loadcount);
            ui->CurrentEventLabel->setText("Not Started");
            ui->EventLogStartBtn->setEnabled(true);
        }
        else {
            msgtext = "No text files to convert in ";
            msgtext += decision_dir;
            ui->EventDirInfoLabel->setText(msgtext);
        }
    }
}

void CK2iLogWindow::on_EventLogStartBtn_clicked() {
    assert(event_converter != NULL);
    ui->PickEventSourceBtn->setEnabled(false);
    ui->EventLogStartBtn->setEnabled(false);
    // variables
    QString cfile,xmsg,outerr;
    bool cresult;
    // looping over the files
    for (size_t fildex = 0; fildex < (event_converter->Filecount()); fildex++) {
        cfile = event_converter->CurrentFile();
        xmsg = "Checking file: " + cfile;
        ui->CurrentEventLabel->setText(xmsg);
        ui->log_display->clear();
        cresult = event_converter->DoCurrentFile(outerr);
        if (!cresult) {
            xmsg = "File failed: " + cfile + " : " + outerr;
            break;
        }
        else {
            ui->EventLogProgressBar->setValue(fildex+1);
        }
    }
}

void CK2iLogWindow::on_ScanLogfileBtn_clicked() {
    QString msgtext;
    QString logfilepath = QFileDialog::getOpenFileName (this,"Pick the log file to scan for mismatches",
               QString(),tr("Log Files (*.log)"));
    if (!logfilepath.isEmpty()) {
        ui->ScanLogfileBtn->setEnabled(false);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->log_display->clear();
        liScanLogs* logscan = new liScanLogs();
        connect(logscan,SIGNAL(SendLog(QString)),this,SLOT(displayLog(QString)));
        if (logscan->LoadFromSource(logfilepath)) {
            logscan->ScanLines();
            msgtext = logscan->GetUmatched();
            ui->log_display->appendPlainText(msgtext);
        }
        else {
            ui->log_display->appendPlainText("Unable To Load File");
        }
        disconnect(logscan,SIGNAL(SendLog(QString)),this,SLOT(displayLog(QString)));
        delete logscan;
        ui->ScanLogfileBtn->setEnabled(true);
        QApplication::restoreOverrideCursor();
    }
}
