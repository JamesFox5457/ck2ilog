/******************************************************************************
 * scranres.h
 * Header file for the class used to scan logfiles for unmatched logs
******************************************************************************/
#ifndef SCANRES_H
   #define SCANRES_H
#endif // SCANRES_H
//-------------------------------------
#include <QObject>
#include <QStringList>
#include <QString>
#include <vector>
#include <map>
/*****************************************************************************/
class liScanLogs : public QObject {
    Q_OBJECT
  public:
    liScanLogs();
    bool LoadFromSource(QString filepath);
    size_t ScanLines();
    QString GetUmatched() const;
signals:
    void SendLog(QString msg);
  protected:
    // internal helper methods
    bool HandleInputLine(QString in_line);
    bool LineToRunTotal(size_t index);
    // lines after being read in
    QStringList lines;
    std::vector<bool> is_end;
    // running total
    bool checked;
    std::map<QString,int> runtotal;

};

/*****************************************************************************/
