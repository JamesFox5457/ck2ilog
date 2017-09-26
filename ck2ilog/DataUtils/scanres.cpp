/******************************************************************************
 * scranres.cpp
 * Definition file for the class used to scan logfiles for unmatched logs
******************************************************************************/
#ifndef SCANRES_H
   #include "scanres.h"
#endif // SCANRES_H
//------------------------------------
#ifndef FILEIO_H
    #include "fileio.h"
#endif // FILEIO_H
#include <assert.h>
#include <QRegExp>
/*****************************************************************************/
liScanLogs::liScanLogs() {
    checked = false;
}
//------------------------------------
bool liScanLogs::LoadFromSource(QString filepath){
    QString rawfile;
    if (!FromANSIFile(rawfile,filepath)) return false;
    QStringList rawlines = rawfile.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    emit SendLog(QString::number(rawlines.length()));
    for (size_t lindex = 0; lindex < rawlines.length(); lindex++) {
        HandleInputLine(rawlines[lindex]);
    }
    return (!lines.empty());
}
//------------------------------------
size_t liScanLogs::ScanLines() {
    emit SendLog(QString::number(lines.length()));
    for (size_t lindex = 0; lindex < lines.length(); lindex++) {
        LineToRunTotal(lindex);
    }
    checked = true;
    return runtotal.size();
}
//------------------------------------
QString liScanLogs::GetUmatched() const {
    assert(checked);
    if (!checked) return "Scan Lines has not been run yet!";
    // variables
    QString output,current;
    std::map<QString,int>::const_iterator itemptr;
    itemptr = runtotal.begin();
    // checking all the items
    while (itemptr != runtotal.end()) {
        if ((itemptr->second) != 0) {
            current = (itemptr->first) + "  :  " + QString::number(itemptr->second);
            output += current + "\n";
        }
        itemptr++;
    }
    // done
    if (output.isEmpty()) return "No Unmatched items found!";
    else return output;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// internal helper methods
//------------------------------------
bool liScanLogs::HandleInputLine(QString in_line){
    QString xline;
    if (in_line.endsWith("START")) {
        xline = in_line.left(in_line.length()-5).trimmed();
        lines.append(xline);
        is_end.push_back(false);
    }
    else if (in_line.endsWith("END")) {
        xline = in_line.left(in_line.length()-3).trimmed();
        lines.append(xline);
        is_end.push_back(true);
    }
    else if (in_line.endsWith("ENDB")) {
        xline = in_line.left(in_line.length()-4).trimmed();
        lines.append(xline);
        is_end.push_back(true);
    }
    else return false;
    return true;
}
//------------------------------------
bool liScanLogs::LineToRunTotal(size_t index) {
    assert(index<lines.length());
    QString pline;
    int lastcolon;
    // parsing the line
    lastcolon = lines[index].indexOf(":LOG");
    if (lastcolon<0) return false;
    pline = lines[index].mid(lastcolon+4).trimmed();
    if (pline.isEmpty()) return false;
    // we now look for this item
    std::map<QString,int>::iterator itemptr;
    itemptr = runtotal.find(pline);
    // if not found, add it
    if (itemptr == runtotal.end()) {
        if (is_end[index]) runtotal[pline] = -1;
        else runtotal[pline] = 1;
    }
    // otherwise just adjust the running count
    else {
        if (is_end[index]) --runtotal[pline];
        else ++runtotal[pline];
    }
    return true;
}

/*****************************************************************************/
