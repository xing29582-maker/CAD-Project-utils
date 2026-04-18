#include "MainWindow.h"
#include "Document.h"
#include "IObject.h"
#include "Point3d.h"
#include "IRenderView.h"
#include "GraphicsScene.h"
#include "MeshGenerator.h"
#include "RenderSystem.h"
#include "TessellationOptions.h"
#include "ObjectFactory.h"
#include "TransactionManager.h"

#include <QItemSelectionModel>
#include <QHeaderView>
#include <QDebug>
#include <QDockWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QKeySequence>

using namespace cadutils;
using namespace std;
namespace
{
    static constexpr int Role_ObjectPtr = Qt::UserRole + 100;
    static constexpr int Role_ObjectId = Qt::UserRole + 200;
    static constexpr int Role_PropKey = Qt::UserRole + 201;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_updatingProps(false)
    , m_sphereCounter(1)
{
    buildUi();
    buildDocument();
    buildTreeModel();

    m_txMgr = std::make_shared<TransactionManager>(m_doc);

    std::shared_ptr<IRenderView> renderView = IRenderView::createRenderView();
    setCentralWidget(renderView->widget());
    std::shared_ptr<GraphicsScene> grepScene = std::make_shared<GraphicsScene>();
    MeshGenerator meshGenerator;
    m_renderSystem = std::make_shared<RenderSystem>(grepScene, meshGenerator, renderView);
    renderView->SetOnPicked([&](ObjectId id)
        {
            m_renderSystem->GetRenderView()->SetSelected(id);
            m_doc->SetSelected(id);
            UpdateProperties(id);
        });
    TessellationOptions tessellationOptions;
    m_renderSystem->SyncFromDocument(m_doc, tessellationOptions);
    m_renderSystem->Refresh();
}

void MainWindow::buildUi()
{
    setWindowTitle("CAD Demo");
    resize(1200, 800);

    m_viewport = new QWidget(this);
    m_viewport->setMinimumWidth(600);
    setCentralWidget(m_viewport);

    // Left dock: Document Tree
    QDockWidget* leftDock = new QDockWidget("Document", this);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_docTreeView = new QTreeView(leftDock);
    m_docTreeView->setHeaderHidden(true);
    m_docTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftDock->setWidget(m_docTreeView);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // Right dock: Properties
    QDockWidget* rightDock = new QDockWidget("Properties", this);
    rightDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_propView = new QTreeView(rightDock);
    m_propView->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::SelectedClicked
    );
    m_propView->setRootIsDecorated(false);
    rightDock->setWidget(m_propView);
    addDockWidget(Qt::RightDockWidgetArea, rightDock);

    // Property model
    m_propModel = new QStandardItemModel(this);
    m_propModel->setColumnCount(2);
    m_propModel->setHeaderData(0, Qt::Horizontal, "Name");
    m_propModel->setHeaderData(1, Qt::Horizontal, "Value");
    m_propView->setModel(m_propModel);
    m_propView->header()->setStretchLastSection(true);

    connect(m_propModel, &QStandardItemModel::itemChanged,
        this, &MainWindow::OnPropItemChanged);

    // --- Actions ---

    // Undo / Redo
    m_undoAction = new QAction("Undo", this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, this, &MainWindow::OnUndo);

    m_redoAction = new QAction("Redo", this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, this, &MainWindow::OnRedo);

    // Add / Delete
    m_addSphereAction = new QAction("Add Sphere", this);
    connect(m_addSphereAction, &QAction::triggered, this, &MainWindow::OnAddSphere);

    m_deleteAction = new QAction("Delete", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::OnDeleteSelected);

    // Menu bar
    QMenu* editMenu = menuBar()->addMenu("Edit");
    editMenu->addAction(m_undoAction);
    editMenu->addAction(m_redoAction);
    editMenu->addSeparator();
    editMenu->addAction(m_addSphereAction);
    editMenu->addAction(m_deleteAction);

    // Toolbar
    QToolBar* toolbar = addToolBar("Edit");
    toolbar->addAction(m_undoAction);
    toolbar->addAction(m_redoAction);
    toolbar->addSeparator();
    toolbar->addAction(m_addSphereAction);
    toolbar->addAction(m_deleteAction);
}

void MainWindow::buildDocument()
{
    m_doc = std::make_shared<Document>("Document");
    m_doc->add(ObjectFactory::CreateSphereObject("Sphere", Point3d(0, 0, 0), 50.0));
}

void MainWindow::buildTreeModel()
{
    m_docTreeModel = new QStandardItemModel(this);
    QStandardItem* root = m_docTreeModel->invisibleRootItem();

    QStandardItem* docItem = new QStandardItem(QString::fromStdString(m_doc->name()));
    docItem->setEditable(false);
    root->appendRow(docItem);

    QStandardItem* objsItem = new QStandardItem("Objects");
    objsItem->setEditable(false);
    docItem->appendRow(objsItem);

    for (const auto& obj : m_doc->GetObjects())
    {
        QStandardItem* item = new QStandardItem(QString::fromStdString(obj->GetObjectName()));
        item->setEditable(false);
        item->setData(QVariant::fromValue(reinterpret_cast<quintptr>(obj.get())), Role_ObjectPtr);
        objsItem->appendRow(item);
    }

    m_docTreeView->setModel(m_docTreeModel);
    m_docTreeView->expandAll();

    auto* sel = m_docTreeView->selectionModel();
    connect(sel, &QItemSelectionModel::currentChanged,
        this, &MainWindow::onTreeSelectionChanged);

    connect(m_docTreeView, &QTreeView::doubleClicked,
        this, &MainWindow::onTreeDoubleClicked);
}

void MainWindow::rebuildTreeModel()
{
    // Disconnect old selection model signals
    if (m_docTreeView->selectionModel())
    {
        disconnect(m_docTreeView->selectionModel(), nullptr, this, nullptr);
    }

    delete m_docTreeModel;
    m_docTreeModel = nullptr;

    buildTreeModel();
}

const IObject* MainWindow::objectFromIndex(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return nullptr;
    auto v = idx.data(Role_ObjectPtr);
    if (!v.isValid())
        return nullptr;
    auto ptr = reinterpret_cast<const IObject*>(v.value<quintptr>());
    return ptr;
}

void cadutils::MainWindow::UpdateProperties(ObjectId id)
{
    m_propModel->setRowCount(0);

    if (id == 0) return;

    std::weak_ptr<IObject> wobj = m_doc->GetobjectById(id);
    std::shared_ptr<IObject> obj = wobj.lock();
    if (!obj) return;

    std::vector<ParameterItem> params;
    obj->GetParameters(params);
    int row = 0;
    for (const ParameterItem& param : params)
    {
        SetPropRow(row++, param.Name.c_str(), param.Value.c_str(), id, param.Key, param.Editable);
    }
}

void cadutils::MainWindow::SetPropRow(int row, const QString& name, const QString& value
    , ObjectId objId, ParamKey key, bool editable)
{
    m_propModel->insertRow(row);
    QStandardItem* nameItem = new QStandardItem(name);
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    m_propModel->setItem(row, 0, nameItem);

    QStandardItem* valueItem = new QStandardItem(value);
    if (!editable)
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
    else
        valueItem->setFlags(valueItem->flags() | Qt::ItemIsEditable);
    valueItem->setData(QVariant::fromValue<qulonglong>(static_cast<qulonglong>(objId)), Role_ObjectId);
    valueItem->setData(static_cast<int>(key), Role_PropKey);
    m_propModel->setItem(row, 1, valueItem);
}

void MainWindow::buildPropertyModel(const IObject* obj)
{
    m_propModel->removeRows(0, m_propModel->rowCount());
    if (!obj) return;
    m_propView->resizeColumnToContents(0);
}

void MainWindow::onTreeSelectionChanged(const QModelIndex& current, const QModelIndex&)
{
    const auto* obj = objectFromIndex(current);
    buildPropertyModel(obj);
}

void MainWindow::onTreeDoubleClicked(const QModelIndex& idx)
{
    const auto* obj = objectFromIndex(idx);
    if (obj) {
        qDebug() << "Double clicked object:" << QString::fromStdString(obj->GetObjectName());
    }
}

void cadutils::MainWindow::OnPropItemChanged(QStandardItem* item)
{
    if (m_updatingProps) return;
    if (!item) return;
    if (item->column() != 1) return;

    auto objIdVar = item->data(Role_ObjectId);
    auto keyVar = item->data(Role_PropKey);
    if (!objIdVar.isValid() || !keyVar.isValid()) return;

    ObjectId objId = static_cast<ObjectId>(objIdVar.toULongLong());
    ParamKey key = static_cast<ParamKey>(keyVar.toInt());

    const std::shared_ptr<IObject> obj = m_doc->GetobjectById(objId);
    if (!obj) return;

    m_txMgr->BeginTransaction();
    obj->SetParameters(key, item->text().toStdString());
    m_txMgr->Commit();

    SyncAndRefresh(false);
}

void cadutils::MainWindow::OnUndo()
{
    if (!m_txMgr->CanUndo())
        return;

    m_txMgr->Undo();

    // Full sync needed because undo may add/remove objects
    TessellationOptions opt;
    m_renderSystem->FullSyncFromDocument(m_doc, opt);
    m_renderSystem->Refresh(true);

    rebuildTreeModel();

    ObjectId selId = m_doc->GetSelected();
    if (selId != 0)
        UpdateProperties(selId);
    else
        m_propModel->setRowCount(0);
}

void cadutils::MainWindow::OnRedo()
{
    if (!m_txMgr->CanRedo())
        return;

    m_txMgr->Redo();

    TessellationOptions opt;
    m_renderSystem->FullSyncFromDocument(m_doc, opt);
    m_renderSystem->Refresh(true);

    rebuildTreeModel();

    ObjectId selId = m_doc->GetSelected();
    if (selId != 0)
        UpdateProperties(selId);
    else
        m_propModel->setRowCount(0);
}

void cadutils::MainWindow::OnAddSphere()
{
    QString name = QString("Sphere_%1").arg(m_sphereCounter++);
    auto newObj = ObjectFactory::CreateSphereObject(
        name.toStdString(), Point3d(0, 0, 0), 50.0);

    m_txMgr->BeginTransaction();
    m_doc->add(newObj);
    m_txMgr->Commit();

    TessellationOptions opt;
    m_renderSystem->FullSyncFromDocument(m_doc, opt);
    m_renderSystem->Refresh(true);

    rebuildTreeModel();
}

void cadutils::MainWindow::OnDeleteSelected()
{
    ObjectId selId = m_doc->GetSelected();
    if (selId == 0)
        return;

    m_txMgr->BeginTransaction();
    m_doc->remove(selId);
    m_txMgr->Commit();

    m_doc->SetSelected(0);
    m_propModel->setRowCount(0);

    TessellationOptions opt;
    m_renderSystem->FullSyncFromDocument(m_doc, opt);
    m_renderSystem->Refresh(true);

    rebuildTreeModel();
}

void cadutils::MainWindow::SyncAndRefresh(bool isAll)
{
    TessellationOptions tessellationOptions;
    m_renderSystem->SyncFromDocument(m_doc, tessellationOptions, isAll);
    m_renderSystem->Refresh(isAll);
}
