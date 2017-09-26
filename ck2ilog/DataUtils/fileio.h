/******************************************************************************
 * fileio.h
 * Header file for the functions and classes that handle file i/o
******************************************************************************/
#ifndef FILEIO_H
    #define FILEIO_H
#endif // FILEIO_H
#ifndef DATAPARSER_H
    #include "dataparser.h"
#endif // DATAPARSER_H
//--------------------------
#include <QString>
#include <QObject>
#include <QStringList>
/*****************************************************************************/
bool ToANSIFile(QString indata, QString filepath);
bool FromANSIFile(QString& outdata, QString filepath);
//--------------------------------------------------------------
class liDirectoryConverter : public QObject {
    Q_OBJECT
  public:
    liDirectoryConverter();
    liDirectoryConverter(bool do_potential, bool do_allow);  // decision constructor
    liDirectoryConverter(bool insert_in_triggers); // event constructor
    int SetDirectory(const QString& in_sourcedir);
    int Filecount() const;
    int FileIndex() const;
    QString CurrentFile() const;
    bool DoCurrentFile(QString& errmsg);
  signals:
    void RelayLog(QString msg);
  protected slots:
    void ReceiveLog(QString msg);
  protected:
    // loading list of files
    int LoadFileList(bool native);
    bool is_event;
    liDecisionsScanner *decision_scanner;
    liEventsScanner    *event_scanner;
    QString sourcedir;
    QStringList filelist;
    int fileindex;

};

/*****************************************************************************/
