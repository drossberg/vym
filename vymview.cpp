#include "vymview.h"

#include "branchitem.h"
#include "dockeditor.h"
#include "mainwindow.h"
#include "mapeditor.h"
#include "treedelegate.h"
#include "treeeditor.h"

extern Main *mainWindow;


VymView::VymView(VymModel *m)
{
    model=m;

    // Create TreeView
    treeEditor=new TreeEditor (model);
    //treeEditor->setModel ((QAbstractItemModel*)model);
    //treeEditor->setMinimumWidth (50);

    treeEditor->setColumnWidth (0,150);
    treeEditor->setAnimated (true);

    selModel=new QItemSelectionModel (model);

    model->setSelectionModel (selModel);
    treeEditor->setSelectionModel (selModel);

    TreeDelegate *delegate=new TreeDelegate (this);
    treeEditor->setItemDelegate (delegate);

    // Create good old MapEditor
    mapEditor=model->getMapEditor();
    if (!mapEditor) mapEditor=new MapEditor (model);

    // Create Layout 

    /*
    QVBoxLayout* mainLayout = new QVBoxLayout (this); 
    QSplitter *splitter= new QSplitter (this);

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    splitter->setSizePolicy(sizePolicy);
    mainLayout->addWidget (splitter);
    */

    // Connect selections

	// Selection in Model changed	
	connect (
	    selModel, SIGNAL (selectionChanged(const QItemSelection &, const QItemSelection &)), 
	    this,SLOT (changeSelection(const QItemSelection &,const QItemSelection &)));

	// Needed to update selbox during animation
	connect (
	    model, SIGNAL (selectionChanged(const QItemSelection &, const QItemSelection &)), 
	    mapEditor,SLOT (updateSelection(const QItemSelection &,const QItemSelection &)));

    // Connect data changed signals 
    connect (
	model, SIGNAL (dataChanged(const QModelIndex &, const QModelIndex &)), 
	mapEditor,SLOT (updateData(const QModelIndex &) ) );

    connect (
	model, SIGNAL (updateQueries (VymModel*)), 
	mainWindow,SLOT (updateQueries(VymModel*) ) );

    connect (
	model, SIGNAL (noteHasChanged(QModelIndex) ),
	mainWindow, SLOT (updateNoteEditor (QModelIndex) ) );
	
    connect (
	model, SIGNAL (expandAll() ),
	this, SLOT (expandAll () ) );
	
    connect (
	model, SIGNAL (expandOneLevel() ),
	this, SLOT (expandOneLevel() ) );
	
    connect (
	model, SIGNAL (collapseOneLevel() ),
	this, SLOT (collapseOneLevel() ) );
	
    connect (
	model, SIGNAL (collapseUnselected() ),
	this, SLOT (collapseUnselected() ) );
	
    connect (
	model, SIGNAL (showSelection() ),
	this, SLOT (showSelection() ) );
	
    connect (
	model, SIGNAL (updateLayout() ),
	mapEditor, SLOT (autoLayout() ) );
	
    mapEditor->setAntiAlias (mainWindow->isAliased());
    mapEditor->setSmoothPixmap(mainWindow->hasSmoothPixmapTransform());

    setCentralWidget (mapEditor);
    DockEditor *de;
    de = new DockEditor (tr("TreeEditor","Title of dockable editor widget"), this, model);
    de->setWidget (treeEditor);
    addDockWidget(Qt::LeftDockWidgetArea, de);
    treeEditorDE=de;
    connect (
	treeEditorDE, SIGNAL (visibilityChanged(bool) ), 
	mainWindow,SLOT (updateActions() ) );

    de = new DockEditor (tr("MapEditor","Title of dockable editor widget"), this, model);
    de->setWidget (mapEditor);
    addDockWidget(Qt::RightDockWidgetArea, de);
    mapEditorDE=de;
    connect (
	mapEditorDE, SIGNAL (visibilityChanged(bool) ), 
	mainWindow,SLOT (updateActions() ) );
}

VymView::~VymView() {}

VymModel* VymView::getModel()
{
    return model;
}

MapEditor* VymView::getMapEditor()
{
    return mapEditor;
}

bool VymView::treeEditorIsVisible()
{
    return treeEditorDE->isVisible();
}

void VymView::initFocus()
{
    mapEditor->setFocus();
}

void VymView::changeSelection (const QItemSelection &newsel, const QItemSelection &oldsel)  
{
    //qDebug() << "VV::changeSelection   newsel.count="<<newsel.indexes().count();

    mainWindow->changeSelection (model,newsel,oldsel);	
    mapEditor->updateSelection (newsel,oldsel);

    if (newsel.indexes().count()>0)
    {

	
	QModelIndex ix=newsel.indexes().first();
	selModel->setCurrentIndex (ix,QItemSelectionModel::ClearAndSelect  );
	treeEditor->setCurrentIndex (ix);
	showSelection();
	
    }
}

void VymView::expandAll()
{
    treeEditor->expandAll();
}

void VymView::expandOneLevel()
{
    int level=999999;
    int d;
    BranchItem *cur=NULL;
    BranchItem *prev=NULL;
    QModelIndex pix;

    // Find level to expand
    model->nextBranch(cur,prev);
    while (cur) 
    {
	pix=model->index (cur);
	d=cur->depth();
	if (!treeEditor->isExpanded(pix) && d < level)
	    level=d;
	model->nextBranch(cur,prev);	
    }

    // Expand all to level
    cur=NULL;
    prev=NULL;
    model->nextBranch(cur,prev);
    while (cur) 
    {
	pix=model->index (cur);
	d=cur->depth();
	if (!treeEditor->isExpanded(pix) && d <= level && cur->branchCount()>0)
	    treeEditor->setExpanded(pix,true);
	model->nextBranch(cur,prev);	
    }
}

void VymView::collapseOneLevel()
{
    int level=-1;
    int d;
    BranchItem *cur=NULL;
    BranchItem *prev=NULL;
    QModelIndex pix;

    // Find level to collapse
    model->nextBranch(cur,prev);
    while (cur) 
    {
	pix=model->index (cur);
	d=cur->depth();
	if (treeEditor->isExpanded(pix) && d > level)
	    level=d;
	model->nextBranch(cur,prev);	
    }

    // collapse all to level
    cur=NULL;
    prev=NULL;
    model->nextBranch(cur,prev);
    while (cur) 
    {
	pix=model->index (cur);
	d=cur->depth();
	if (treeEditor->isExpanded(pix) && d >= level)
	    treeEditor->setExpanded(pix,false);
	model->nextBranch(cur,prev);	
    }
}

void VymView::collapseUnselected()
{
    BranchItem *cur=NULL;
    BranchItem *prev=NULL;
    QModelIndex pix;

    // Find level to collapse
    TreeItem *selti=model->getSelectedItem();
    if (!selti) return;

    int level=selti->depth();

    // collapse all to level
    model->nextBranch(cur,prev);
    while (cur) 
    {
	pix=model->index (cur);
	if (treeEditor->isExpanded(pix) &&  level <= cur->depth())
	    treeEditor->setExpanded(pix,false);
	model->nextBranch(cur,prev);	
    }
}

void VymView::showSelection()
{
    QModelIndex ix=model->getSelectedIndex();
    treeEditor->scrollTo( ix, QAbstractItemView::EnsureVisible);
    mapEditor->scrollTo ( ix);	
}

void VymView::toggleTreeEditor()
{
    if (treeEditorDE->isVisible() )
	treeEditorDE->hide();
    else
	treeEditorDE->show();
}


