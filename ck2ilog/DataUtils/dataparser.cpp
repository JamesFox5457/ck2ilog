/******************************************************************************
 * dataparser.cpp
 * File for the classes that do the primary work of this app -- inserting
 * logging statements into CK2 decision and event files.
******************************************************************************/
#ifndef DATAPARSER_H
    #include "dataparser.h"
#endif // DATAPARSER_H
//-----------------------------------------
#include <QCoreApplication>
#include <assert.h>
//*****************************************************************************
const QString sp_exp = "[ \\t\xA0]*";
const QString nl_exp = "(\\n|\\r\\n|\\r|\\n\\r)";
const QString line_comment = "#[^\\n\\r]*(" + nl_exp + "|$)";
const QString block_start = "\\s*=\\s*\\{";
const QString break_exp = "break\\s*=\\s*yes";
const QString start_or_nl_exp = "(^|" + nl_exp + ")";
/* NOTE : I am using regular expresssion to find places to insert log statements.
 * this is not as flexible as true parsing, so I've ended up assuming that big
 * blocks are always on a new line, and that comments (damn them) are not inserted
 * in the middle of block starts (which I think is possible, but strange). */
//=======================================================
liScannerBase::liScannerBase() {
    ResetNonStrings();
    break_statement = QRegExp(break_exp);
}
//-------------------------------
bool liScannerBase::SetSource(const QString& source_in) {
    if (source_in.isEmpty()) return false;
    ResetNonStrings();
    source = source_in;
    workblock = &source;
    work_pos = 0;
    destination = "";
    return true;
}
//-------------------------------
QString liScannerBase::GetResult() const {
    return destination;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
void liScannerBase::LogString(QString msg, QString data) {
    emit SendLog(msg + " : " + data);
    QCoreApplication::processEvents();
}
//-------------------------------
void liScannerBase::LogBool(QString msg, bool data) {
    emit SendLog(msg + " : " + ((data)?("true"):("false")));
    QCoreApplication::processEvents();
}
//-------------------------------
void liScannerBase::LogNumber(QString msg, int data) {
    emit SendLog(msg + " : " + QString::number(data));
    QCoreApplication::processEvents();
}
//-------------------------------
void liScannerBase::LogBlockStart(QString msg, size_t amount) {
    if ((workblock->length()) < amount) amount = workblock->length();
    QString oo = workblock->left(amount);
    emit SendLog(msg + " : " + oo);
    QCoreApplication::processEvents();
}
//-------------------------------
void liScannerBase::ResetNonStrings() {
    block_level = 0;
    endblock_pos[0] = -1;
    endblock_pos[1] = -1;
    endblock_pos[2] = -1;
    workblock = NULL;
    work_pos = -1;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
// parser help methods
//-------------------------------
// after {, find the closing } (handles nesting)
int liScannerBase::FindEndOfBlock() const {
    assert(workblock != NULL);
    if ( work_pos >= workblock->length() ) return -1;
    // prepping for the character scanning
    int cpos = work_pos;
    QChar currchar;
    bool xvalid;
    int level = 0; // nested {} level, we reach the end when negative
    bool incomment = false;

    // scanning character by character for '{' and '}'
    while (level >= 0) {
        // premature end check
        if ( cpos >= workblock->length() ) {
            xvalid = false;
            break;
        }
        // checking the current character
        currchar = (*workblock)[cpos];
        if (currchar == '#') {
            xvalid = false;
            incomment = true;
        }
        else if (currchar == '\n') {
            xvalid = false;
            incomment = false;
        }
        else if ((currchar == '{') && (!incomment)) {
            level++;
            xvalid = true;
        }
        else if ((currchar == '}') && (!incomment)) {
            level--;
            xvalid = true;
        }
        else xvalid = false;
        // moving to the next character
        cpos++;
    }
    // at the end, have we found the end of the block?
    if (!xvalid) return -1;
    // wwe have found the end of the block!
    else return cpos-1;
}
//----------------------------------------------
/* an item has been found at pos, we scan from work_pos to pos to
 * check if there is anything that might invalidate the find. */
bool liScannerBase::ScanToPosZeroLevelValid(const int& pos, bool nobrace) const {
    // assertion checks
    assert(workblock!=NULL);
    assert(work_pos>=0);
    assert(pos>=work_pos);
    // quick okay
    if (pos == work_pos) return true;
    // setting up the variables
    size_t brace_level = 0;
    bool incomment = false;
    QChar xchar;
    // checking character by character
    for (int cpos = work_pos; cpos < (pos-1); cpos++) {
        xchar = (*workblock)[cpos];
        if (xchar == '#') incomment = true;
        else if (xchar == '\n') incomment = false;
        else if ((!incomment) && (xchar == '{')) {
            if (nobrace) return false;
            else brace_level++;
        }
        else if ((!incomment) && (xchar == '}')) {
            if (nobrace) return false;
            else brace_level--;
        }
    }
    // check for bad things after the loop
    xchar = (*workblock)[pos-1];
    if ((xchar == '\n') || (xchar == '\r')) return true;
    if (incomment) return false;
    if (xchar == '{') brace_level++;
    else if (xchar == '}') brace_level--;
    if (brace_level != 0) return false;
    // the last character must be whitespace
    return xchar.isSpace();
}
//----------------------------------------------
bool liScannerBase::ScanToPosValid(const int& pos, int &brace_change) const {
    // assertion checks
    assert(workblock!=NULL);
    assert(work_pos>=0);
    assert(pos>=work_pos);
    // quick okay
    if (pos == work_pos) return false;
    // setting up the variables
    brace_change = 0;
    bool incomment = false;
    QChar xchar;
    // checking character by character
    for (int cpos = work_pos; cpos < (pos-1); cpos++) {
        xchar = (*workblock)[cpos];
        if (xchar == '#') incomment = true;
        else if (xchar == '\n') incomment = false;
        else if (xchar == '\r') incomment = false;
        else if ((!incomment) && (xchar == '{')) brace_change++;
        else if ((!incomment) && (xchar == '}')) brace_change--;
    }
    // check for bad things after the loop
    xchar = (*workblock)[pos-1];
    if ((xchar == '\n') || (xchar == '\r')) return true;
    if (incomment) return false;
    if (xchar == '{') brace_change++;
    else if (xchar == '}') brace_change--;
    if ((xchar == '{') || (xchar == '}') || xchar.isSpace()) return true;
    else return false;
 }
//----------------------------------------------
/* searches for xp_to_find from work_pos, finds must pass checking using
ScanToPosZeroLevelValid. */
int liScannerBase::SearchForUncommentedZeroLevel(QRegExp& xp_to_find, bool nobrace) const {
    // assertion checks
    assert(xp_to_find.isValid());
    assert(workblock!=NULL);
    assert(work_pos >= 0);
    // variables
    int spos = work_pos;
    int fpos = 0;
    bool xvalid;
    // the main loop
    while(true) {
        fpos = xp_to_find.indexIn((*workblock),spos);
        if (fpos < 0) return -1;
        xvalid = ScanToPosZeroLevelValid(fpos,nobrace);
        if (xvalid) return fpos;
        spos = fpos + xp_to_find.matchedLength();
        if (spos >= (workblock->length())) return -1;
    }
    // we should never get here
    return -1;
}
//----------------------------------------------
int liScannerBase::SearchForUncommented(QRegExp& xp_to_find, int& brace_out) const {
    // assertion checks
    assert(xp_to_find.isValid());
    assert(workblock!=NULL);
    assert(work_pos >= 0);
    // variables
    int spos = work_pos;
    int fpos = 0;
    bool xvalid = false;
    int brace_change;
    // the main loop
    while(true) {
        fpos = xp_to_find.indexIn((*workblock),spos);
        if (fpos < 0) return -1;
        xvalid = ScanToPosValid(fpos,brace_change);
        brace_out = brace_change;
        if (xvalid) return fpos;
        spos = fpos + xp_to_find.matchedLength();
        if (spos >= (workblock->length())) return -1;
    }
    // we should never get here
    return -1;
}
//----------------------------------------------
// used at spots to try and skip whitespace and comments
bool liScannerBase::SkipSpaceComment(int& newpos) const {
    if (workblock==NULL) return false;
    if (work_pos < 0) return false;
    if (work_pos >= workblock->length()) return false;
    // variables for scanning
    QChar xchar;
    int xpos = work_pos;
    bool incomment = false;
    // scanning character by character...
    while (xpos < workblock->length()) {
        xchar = (*workblock)[xpos];
        if (xchar == '#') incomment = true;
        else if (incomment & (xchar == '\n')) incomment = false;
        else if ((!incomment) & (!xchar.isSpace())) break; // the end!
        xpos++;
    }
    // if we get here, return true even if at the end of the string
    newpos = xpos;
    return true;
}
//------------------------------------------------
bool liScannerBase::CopyTo(const int& newpos) {
    assert(workblock != NULL);
    assert(work_pos >= 0);
    if (newpos <= work_pos) return false;
    if (newpos > workblock->length()) return false;
    destination += workblock->mid(work_pos,newpos-work_pos);
    work_pos = newpos;
    return true;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* Finds blockstart_exp = { ... }, and extracts the contents to block<num>,
 * nesting to a new block level (and storing the end position in endblock_pos). */
bool liScannerBase::ToAndGetBlock(QRegExp& exp_to_use) {
    assert(workblock != NULL);
    assert(exp_to_use.isValid());
    /**/LogString("ToAndGetBlock 01","");
    if ( work_pos >= workblock->length() ) return false;
    /**/LogString("ToAndGetBlock 02","");
    // looking for the next match
    int rpos = workblock->indexOf(exp_to_use,work_pos);
    /**/LogNumber("BlockStart pos",rpos);
    if (rpos < 0) return false;
    // transferring to past it, appending the skipped stuff to destination (output)
    int endpos = rpos + exp_to_use.matchedLength();
    /**/LogString("BlockStart found",exp_to_use.cap(0));
    CopyTo(endpos);
    // only used for decisions
    if (exp_to_use.captureCount()>0) {
        blockname = exp_to_use.cap(exp_to_use.captureCount());
        /**/LogString("BlockName",blockname);
    }
    else blockname = "";
    // we are now inside a block, so we look for the end of it
    int epos = FindEndOfBlock();
    /**/LogNumber("BlockEnd pos",epos);
    if (epos < 0) return false;
    // getting the block contents
    QString tempblock = workblock->mid(work_pos,epos-work_pos);
    // now, we move to a new block level!
    endblock_pos[block_level] = epos + 1;
    block_level++;
    /**/LogNumber("BlockLevel",block_level);
    if (block_level==1) {
        block1 = tempblock;
        workblock = &block1;
    }
    else if (block_level==2) {
        block2 = tempblock;
        workblock = &block2;
    }
    else if (block_level==3) {
        block3 = tempblock;
        workblock = &block3;
    }
    else assert(false);
    /*LogBlockStart("BLOCK F",200);*/
    work_pos = 0;
    // done
    return true;
}

//----------------------------------------------
bool liScannerBase::ToAndGetBlock(const QString blockstart_exp) {
    assert(workblock != NULL);
    assert(blockstart_exp.length()>0);
    if ( work_pos >= workblock->length() ) return false;

    // making a regular expression to search for the start of the block
    QString rtemp = start_or_nl_exp + "[ \\t]*" + blockstart_exp + block_start;
    QRegExp startlook = QRegExp(rtemp);
    assert(startlook.isValid());
    // doing the rest
    return ToAndGetBlock(startlook);
}
//----------------------------------------------
// adds block remaining + } to destination and goes up a nested block level
bool liScannerBase::OutOfBlock() {
    if (block_level == 0) return false;
    // copying remainder of block to destination and closing
    CopyTo(workblock->length());
    destination += '}';
    // going up a block level
    block_level--;
    if (block_level==0) workblock = &source;
    else if (block_level==1) workblock = &block1;
    else if (block_level==2) workblock = &block2;
    else assert(false);
    work_pos = endblock_pos[block_level];
    // done
    return true;
}
//----------------------------------------------
/* breaks cause exits from anywhere in the primary block, so to properly
 * record an exit, we need to insert logging statements before them. */
int liScannerBase::LogBreaksinBlock(QString msg) {
    if (block_level == 0) return -1;
    QString itab;
    int spos,npos;
    int extra_bracec = 0;
    int runbrace = block_level;
    size_t bcount = 0;
    /**/LogString("LogBreaksinBlock","HERE 1");

    while(0 <= (spos = SearchForUncommented(break_statement,extra_bracec))) {
        CopyTo(spos);
        runbrace += extra_bracec;
        itab = QString(runbrace,'\t');
        destination += "log = \"" + msg + " ENDB\"\n";
        destination += itab;
        npos = spos + break_statement.matchedLength();
        CopyTo(npos);
        bcount++;
    }
    /**/LogNumber("LogBreaksinBlock",bcount);
    return bcount;
}
//----------------------------------------------
// the same as above, but with logging statements
bool liScannerBase::LogAndOutOfBlock(QString msg, bool endonly, bool inserttab) {
    if (block_level == 0) return false;
    QString itab = QString(block_level,'\t');
    // start log
    if (!endonly) {
        destination += "\n" + itab;
        destination += "log = \"" + msg + " START\"\n";
    }
    if (inserttab) destination += itab;
    /**/LogString("LogAndOutOfBlock","HERE 1");
    LogBreaksinBlock(msg);
    // copying remainder of block to destination and closing
    CopyTo(workblock->length());
    // end log
    destination += "\n" + itab;
    destination += "log = \"" + msg + " END\"\n";
    // closing the block
    if (block_level > 1) destination += "\t";
    if (block_level == 3) destination += "\t";
    destination += '}';
    // going up a block level
     /**/LogNumber("LogAndOutOfBlock BlockLevel",block_level);
    block_level--;
    if (block_level==0) workblock = &source;
    else if (block_level==1) workblock = &block1;
    else if (block_level==2) workblock = &block2;
    else assert(false);
    work_pos = endblock_pos[block_level];
    // done
    return true;
}
//----------------------------------------------
// adding the rest of the source
void liScannerBase::AddRest() {
    assert(block_level == 0);
    assert(work_pos >= 0);
    if (work_pos < source.length()) {
        destination += source.mid(work_pos);
        work_pos = source.length();
    }
}
//=====================================================
// Regular expressions for decisions
const QString decision_group_exp = "([a-zA-Z]+_([a-zA-Z]+_)?)?decisions";
const QString decision_exp = "([a-z_A-Z]+)";
//+++++++++++++++++++++++++++++++++++++++++++++++++
liDecisionsScanner::liDecisionsScanner():liScannerBase() {
    SetupFirstExp();
    do_potential_blocks = false;
    do_allow_blocks = false;
}
//----------------------------------------------
liDecisionsScanner::liDecisionsScanner(bool il_potential, bool il_allow):liScannerBase() {
    SetupFirstExp();
    do_potential_blocks = il_potential;
    do_allow_blocks = il_allow;
    if (il_potential || il_allow) {
        QString reg_temp = start_or_nl_exp + sp_exp + "potential" + block_start;
        potential_block = QRegExp(reg_temp);
        reg_temp = nl_exp + sp_exp + "allow" + block_start;
        allow_block = QRegExp(reg_temp);
    }
}
//-----------------------------------------------
bool liDecisionsScanner::Process() {
    bool effect_found;
    QString decision_name;
    /**/LogString("ProcessStart","");
    // the top level loop: blocks of multiple decisions
    while (ToAndGetBlock(decision_group)) {
        /**/LogString("Decision Group Start","");
        // inner loop: decisions within the block
        while (ToAndGetBlock(single_decision)) {
            decision_name = blockname;
             /**/LogString("DecisionStart",decision_name);
            // potential
            if (do_potential_blocks) {
                if (ToAndGetBlock(potential_block)) {
                    effect_found = LogAndOutOfBlock("LOG POTENTIAL: " + decision_name,false,false);
                }
            }
            // allow
            if (do_allow_blocks) {
                if (ToAndGetBlock(allow_block)) {
                    effect_found = LogAndOutOfBlock("LOG ALLOW: " + decision_name,false,false);
                }
            }
            // finding the effect block and producing the log version
            effect_found = ToAndGetBlock(effect_block);
            if (effect_found) {
                effect_found = LogAndOutOfBlock("LOG DECISION: " + decision_name,false,false);
            }
            /**/LogString("Decision Here","");
            // finishing the decision
            OutOfBlock();
        }
        // finishing the group of decisions
        OutOfBlock();
    }
    // done
    AddRest();
    return true;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++
void liDecisionsScanner::SetupFirstExp() {
    // I am assuming here that blocks are at the start of lines (not counting tabs or spaces)
    QString reg_temp = start_or_nl_exp + sp_exp + decision_group_exp + block_start;
    decision_group = QRegExp(reg_temp);
    reg_temp = nl_exp + sp_exp + decision_exp + block_start;
    single_decision = QRegExp(reg_temp);
    reg_temp = nl_exp + sp_exp + "effect" + block_start;
    effect_block = QRegExp(reg_temp);
}
//==========================================================================
// Regular expressions for events
const QString single_event_exp = "[a-zA-Z]+_([a-zA-Z]+_)?event";
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
liEventsScanner::liEventsScanner():liScannerBase() {
    InitRegExp();
    insert_in_triggers = false;
}
//-----------------------------------------------------------------
liEventsScanner::liEventsScanner(bool do_triggers):liScannerBase() {
    InitRegExp();
    insert_in_triggers = do_triggers;
}
//---------------------------------------------
bool liEventsScanner::Process() {
    bool fresult;
    QString event_name;
    QString logstr;
    int optioncount;
    // the top level loop: events
    while (ToAndGetBlock(single_event)) {
        /**/LogString("Event Start","");
        optioncount = 0;
       // we look for the id in the event
        if (ExtractID(event_name)) {
            // inserting logs in triggers: makes the game slow and the logfile huge
            if (insert_in_triggers) {
                if (ToAndGetBlock(trigger_block)) {
                    LogString("Event TRIGGER","");
                    logstr = "LOG TRIGGER : " + event_name;
                    fresult = LogAndOutOfBlock(logstr,false,false);
                    if (!fresult) return false;
                }
            }
            // next, the immediate block
            if (ToAndGetBlock(immediate_block)) {
                /**/LogString("Event Immediate","");
                logstr = "LOG EVENT IMMEDIATE : " + event_name;
                fresult = LogAndOutOfBlock(logstr,false,false);
                if (!fresult) return false;
            }
            // option blocks
            while (ToAndGetBlock(option_block)) {
                optioncount++;
                /**/LogNumber("Event Option",optioncount);
                logstr = "LOG EVENT OPTION " + QString::number(optioncount);
                logstr += " : " + event_name;
                // moving past the names, triggers, and ai_chance is complex
                fresult = MovePastNameTrigger();
                /**/LogBool("Event Optionskip A",fresult);
                if (fresult)  fresult = LogAndOutOfBlock(logstr,false,true);
                else fresult = LogAndOutOfBlock(logstr,true,true);
                if (!fresult) return false;
            }
        }
        OutOfBlock();
    }
    // done
    AddRest();
    return true;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void liEventsScanner::InitRegExp() {
    // setting up the regular expression...
    // I am assuming here that blocks are at the start of lines (not counting tabs or spaces)
    QString reg_temp = start_or_nl_exp + sp_exp + single_event_exp + block_start;
    single_event = QRegExp(reg_temp);
    reg_temp = nl_exp + sp_exp + "trigger" + block_start;
    trigger_block = QRegExp(reg_temp);
    reg_temp = nl_exp + sp_exp + "immediate" + block_start;
    immediate_block = QRegExp(reg_temp);
    reg_temp = nl_exp + sp_exp + "option" + block_start;
    option_block = QRegExp(reg_temp);
    // finding the id extraction expression
    id_exp = QRegExp("id\\s*=\\s*(([a-z_A-Z]+\\.)?\\d+)");
    // expressions to help skipping the stuff at the start of option blocks
    name_exp = QRegExp("name\\s*=\\s*");
    nameb_exp = QRegExp("name" + block_start);
    trigger_exp = QRegExp("trigger" + block_start);
    aido_exp = QRegExp("ai_chance" + block_start);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// helper methods
/* id is not a block. It can have several issues, since unlike major blocks,
 * the id tag can appear within commands. Also, watch out for comments! */
bool liEventsScanner::ExtractID(QString& idout) {
    // using some special methods to avoid comment issues
    int spos = SearchForUncommentedZeroLevel(id_exp,true);
    /**/LogNumber("ExtractID",spos);
    if (spos < 0) return false;
    // there should be a captured name...
    if (id_exp.captureCount() == 0) return false;
    idout = id_exp.cap(1);
    /**/LogString("ExtractID",idout);
    // moving to the end, and copying data over
    CopyTo(spos+id_exp.matchedLength());
    // done
    return true;
}
//---------------------------------------------
/* an option block can start with multiple names, a trigger block, and an ai_chance block, all of these
 * are optional. Since logging statements should go after, this is extra complicated... */
bool liEventsScanner::MovePastNameTrigger() {
    // apparently order is not strict, so I look for the last...
    int fpos1 = SearchForUncommentedZeroLevel(name_exp,false);
    int fpos2 = SearchForUncommentedZeroLevel(trigger_exp,false);
    int fpos3 = SearchForUncommentedZeroLevel(aido_exp,false);
    int largest = (fpos1>fpos2)?(fpos1):(fpos2);
    largest = (fpos3>largest)?(fpos3):(largest);
    if (largest == -1) return true; // this is actually okay
    int which,epos,npos;
    if (largest == fpos1) which = 1;
    else if (largest == fpos2) which = 2;
    else if (largest == fpos3) which = 3;
    /* I will assume that multiple name blocks occur one after another, so if
     * which == 2 or 3, that is the last block. */
    if (which != 1) {
        // finding the end of the match and moving past it
        if (which == 2) epos = fpos2 + trigger_exp.matchedLength();
        else epos = fpos3 + aido_exp.matchedLength();
        CopyTo(epos);
        // finding the closing } and moving past it
        npos = FindEndOfBlock();
        if (npos<0) return false;
        CopyTo(npos+1);
        if ((npos+1) >= workblock->length()) return true;
    }
    // the last block was a name block, dammit
    else {
        epos = fpos1 + name_exp.matchedLength();
        if (epos >= (workblock->length())) return false;
        QChar checkx = (*workblock)[epos];
        // names can be either a single item, or multiple blocks
        if (checkx != '{') {
            // single item, find the end and move past it...
            QRegExp endname = QRegExp("\\s|#|$");
            int epos2 = workblock->indexOf(endname,epos);
            if (epos2 <= epos) return false;
            CopyTo(epos2);
        }
        // multiblock names! (aaargh!)
        else {
            CopyTo(epos+1);
            if (work_pos >= (workblock->length())) return false;
            // finding the closing } and moving past it
            npos = FindEndOfBlock();
            if (npos<0) return false;
            CopyTo(npos+1);
            // after this, there may be multiple name = { blocks...
            while (0<=(npos = SearchForUncommentedZeroLevel(nameb_exp,false))) {
                epos = npos + nameb_exp.matchedLength();
                CopyTo(epos);
                if (work_pos >= (workblock->length())) return false;
                epos = FindEndOfBlock();
                if (epos<0) return false;
                CopyTo(epos+1);
            }
        }
    }
    // we also skip any follwing comments (or spaces)
    SkipSpaceComment(npos);
    if (npos != work_pos) CopyTo(npos);
    // done
    return true;
}

//*****************************************************************************

