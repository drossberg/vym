#ifndef EXPORT_BASE_H
#define EXPORT_BASE_H

#include <qdir.h>
#include <qstring.h>
#include <iostream>

#include "settings.h"
#include "vymmodel.h"


/*! \brief Base class for all exports
*/

///////////////////////////////////////////////////////////////////////

class ExportBase
{
public:
    ExportBase();
    ExportBase(VymModel *m);
    virtual ~ExportBase();
    virtual void init();
    virtual void setDirPath (const QString&);
    virtual QString getDirPath();
    virtual void setFilePath (const QString&);
    virtual QString getFilePath ();
    virtual QString getMapName ();
    virtual void setModel (VymModel *m);
    virtual void setWindowTitle (const QString &);
    virtual void setName( const QString &);
    virtual QString getName();
    virtual void addFilter (const QString &);
    virtual void setListTasks( bool b);
    virtual bool execDialog();
    virtual bool canceled();
    void setLastCommand( const QString& );
    void completeExport(QString args="");  //! set lastExport and send status message

protected:  
    VymModel *model;
    QString exportName;
    QString lastCommand;
    virtual QString getSectionString (TreeItem*);

    QString indent (const int &n, bool useBullet);
    QDir tmpDir;
    QString dirPath;        // Path to dir  e.g. /tmp/vym-export/
    QString defaultDirPath; // Default path
    QString filePath;       // Path to file e.g. /tmp/vym-export/export.html
    QString extension;      // Extension, e.g. .html
    QString indentPerDepth;
    int indentPerDepth2;
    QStringList bulletPoints;
    QString caption;
    QString filter;
    bool listTasks;         // Append task list
    bool cancelFlag;
};


#endif
