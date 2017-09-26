/******************************************************************************
 * dataparser.h
 * Header file for the classes that do the primary work of this app -- inserting
 * logging statements into CK2 decision and event files.
******************************************************************************/
#ifndef DATAPARSER_H
    #define DATAPARSER_H
#endif // DATAPARSER_H
//--------------------------------
#include <QObject>
#include <QString>
#include <QRegExp>
/*****************************************************************************/
// base class for the decision and event scanners, handles some common stuff
class liScannerBase : public QObject {
    Q_OBJECT
  public:
    liScannerBase();
    bool SetSource(const QString& source_in);
    QString GetResult() const;
    virtual bool Process() = 0;
  signals:
    void SendLog(QString msg);
  protected:
    void LogString(QString msg, QString data);
    void LogBool(QString msg, bool data);
    void LogNumber(QString msg, int data);
    void LogBlockStart(QString msg, size_t amount);
    void ResetNonStrings();
    // parser help methods
    int FindEndOfBlock() const;
    bool ScanToPosZeroLevelValid(const int& pos, bool nobrace) const;
    bool ScanToPosValid(const int& pos, int &brace_change) const;
    int SearchForUncommentedZeroLevel(QRegExp& xp_to_find, bool nobrace) const;
    int SearchForUncommented(QRegExp& xp_to_find, int& brace_out) const;
    bool SkipSpaceComment(int& newpos) const;
    bool CopyTo(const int& newpos);
    /* finds blockstart_exp = { ... }, and extracts the contents to block<num>,
     * nesting to a new block level (and storing the end position in endblock_pos). */
    bool ToAndGetBlock(QRegExp& exp_to_use);
    bool ToAndGetBlock(const QString blockstart_exp);
    // adds block remaining + }\n to destination and goes up a nested block level
    bool OutOfBlock();
    // the same as above, but with logging statements
    int LogBreaksinBlock(QString msg);
    bool LogAndOutOfBlock(QString msg, bool endonly, bool inserttab);
    // adding the rest of the source
    void AddRest();
    // source
    QString source;   // file level
    // working data
    QString block1,block2,block3;  // blocks of text within {} that are looked at
    size_t block_level;  // 0 to 3
    int endblock_pos[3]; // stores end of block position in the enclosing block/file
    QString *workblock;  // pointer to currently used source/block
    QString blockname;
    int work_pos;
    // result data
    QString destination;
    QRegExp break_statement;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// handles inserting logs for a file of decisions
class liDecisionsScanner : public virtual liScannerBase {
    Q_OBJECT
  public:
    liDecisionsScanner();
    liDecisionsScanner(bool il_potential, bool il_allow);
    virtual bool Process();
  protected:
    void SetupFirstExp();
    QRegExp decision_group,single_decision,effect_block;
    QRegExp potential_block,allow_block;
    bool do_potential_blocks,do_allow_blocks;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// handles inserting logs for a file of events
class liEventsScanner : public virtual liScannerBase {
    Q_OBJECT
  public:
    liEventsScanner();
    liEventsScanner(bool do_triggers);
    virtual bool Process();
  protected:
    void InitRegExp();
    // helper methods
    bool ExtractID(QString& idout);
    bool MovePastNameTrigger();
    // expressions
    QRegExp single_event,trigger_block,immediate_block,option_block;
    QRegExp id_exp;
    QRegExp name_exp,nameb_exp,trigger_exp,aido_exp;
    // also
    bool insert_in_triggers;
};

/*****************************************************************************/
