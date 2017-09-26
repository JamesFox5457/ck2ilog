/******************************************************************************
 * fileio.cpp
 * Header file for the classes that do the primary work of this app -- inserting
 * logging statements into CK2 decision and event files.
******************************************************************************/
#ifndef FILEIO_H
    #include "fileio.h"
#endif // FILEIO_H
//--------------------------
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <assert.h>
/*****************************************************************************/
//===========================================================================
bool ToANSIFile(QString indata, QString filepath) {
  // variables
  QFile* outfile = NULL;
  QTextStream* outstream = NULL;
  // creating or opening the file
  outfile = new QFile(filepath);
  if (!outfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
    delete outfile;
    return false;
  }
  // creating the stream and writing
  outstream = new QTextStream(outfile);
  outstream->setCodec("Windows-1252");
  (*outstream) << indata;
  // closing
  outstream->flush();
  outfile->flush();
  outfile->close();
  // clearing
  delete outstream;
  delete outfile;
  return true;
}
//--------------------------------------------------------------------
// reads data from a file
bool FromANSIFile(QString& outdata, QString filepath) {
  // variables
  QFile* infile = NULL;
  QTextStream* instream = NULL;
  // opening the file
  infile = new QFile(filepath);
  if (!infile->open(QIODevice::ReadOnly | QIODevice::Text)) {
    delete infile;
    return false;
  }
  // creating the stream and reading
  instream = new QTextStream(infile);
  instream->setCodec("Windows-1252");
  outdata = instream->readAll();
  // done
  infile->close();
  delete instream;
  delete infile;
  return true;
}
//=======================================================================
liDirectoryConverter::liDirectoryConverter():QObject() {} // not used
//-----------------------------------------------
// decision constructor
liDirectoryConverter::liDirectoryConverter(bool do_potential,bool do_allow):QObject() {
    is_event = false;
    decision_scanner = new liDecisionsScanner(do_potential,do_allow);
    event_scanner = NULL;
    connect(decision_scanner,SIGNAL(SendLog(QString)),this,SLOT(ReceiveLog(QString)));
    fileindex = -1;
}
//-----------------------------------------------
// event constructor
liDirectoryConverter::liDirectoryConverter(bool insert_in_triggers):QObject() {
    is_event = true;
    decision_scanner = NULL;
    event_scanner = new liEventsScanner(insert_in_triggers);
    connect(event_scanner,SIGNAL(SendLog(QString)),this,SLOT(ReceiveLog(QString)));
    fileindex = -1;
}
//-----------------------------------------------
int liDirectoryConverter::SetDirectory(const QString& in_sourcedir) {
    QDir testx;
    if (!testx.exists(in_sourcedir)) return 0;
    sourcedir = QDir::fromNativeSeparators(in_sourcedir.trimmed());
    if (!sourcedir.endsWith('/')) sourcedir += '/';
    return LoadFileList(false);
}
//-----------------------------------------------
int liDirectoryConverter::Filecount() const {
    return filelist.length();
}
//-----------------------------------------------
int liDirectoryConverter::FileIndex() const {
    return fileindex;
}
//-----------------------------------------------
QString liDirectoryConverter::CurrentFile() const {
    if (fileindex < 0) return "";
    if (fileindex >= filelist.length()) return "";
    QString fullpath = filelist[fileindex];
    int spos = fullpath.lastIndexOf('/');
    assert(spos>=0);
    return fullpath.mid(spos+1);
}
//-----------------------------------------------
bool liDirectoryConverter::DoCurrentFile(QString& errmsg) {
    // basic bad cases
    if (fileindex < 0) {
        errmsg = "No files found!";
        return false;
    }
    if (fileindex >= filelist.length()) {
        errmsg = "Index past the end!";
        return false;
    }
    // going ahead
    QString filedata,filepath;
    // loading the event/decision file contents
    filepath = filelist[fileindex];
    bool opresult = FromANSIFile(filedata,filepath);
    if (!opresult) {
        errmsg = "Could not load : " + filepath;
        return false;
    }
    // setting the data and processing
    if (is_event) {
       event_scanner->SetSource(filedata);
       opresult = event_scanner->Process();
    }
    else {
        decision_scanner->SetSource(filedata);
        opresult = decision_scanner->Process();
    }
    if (!opresult) {
        errmsg = "Error when trying to insert log : " + filepath;
        fileindex++;
        return false;
    }
    // getting the changed data
    if (is_event) filedata = event_scanner->GetResult();
    else filedata = decision_scanner->GetResult();
    // writing the data
    opresult = ToANSIFile(filedata,filepath);
    if (!opresult) {
        errmsg = "Could not write changed file : " + filepath;
        fileindex++;
        return false;
    }
    // done
    fileindex++;
    return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++
void liDirectoryConverter::ReceiveLog(QString msg) {
    emit RelayLog(msg);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
// loading list of files
int liDirectoryConverter::LoadFileList(bool native) {
    QDir workdir(sourcedir);
    QStringList name_filters;
    QFileInfo curf;
    QString result;
    // setting the filters before getting the name list
    name_filters << "*.txt";
    workdir.setFilter(QDir::Files|QDir::Readable|QDir::Writable|QDir::NoDotAndDotDot|QDir::NoSymLinks);
    workdir.setNameFilters(name_filters);
    // getting the fill list
    QFileInfoList dlist = workdir.entryInfoList();
    filelist.clear();
    emit RelayLog(QString::number(dlist.size()));
    for (size_t dindex = 0 ; dindex < dlist.size() ; dindex++) {
      curf = dlist.at(dindex);
      result = curf.canonicalFilePath();
      if (native) filelist.append(QDir::toNativeSeparators(result));
      else filelist.append(result);

    }
    fileindex = -1;
    if (filelist.length()>0) fileindex = 0;
    emit RelayLog(QString::number(filelist.length()));
    return filelist.length();
}

/*****************************************************************************/
