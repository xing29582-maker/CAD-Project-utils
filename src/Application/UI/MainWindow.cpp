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
#include "CommandRegistry.h"
#include "CommandUIBuilder.h"
#include "ActionManager.h"
#include "ActionUIBuilder.h"

#include <QItemSelectionModel>
#include <QHeaderView>
#include <QDebug>
#include <QDockWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QFile>
#include <QCoreApplication>

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
    , m_updatingTree(false)
    , m_sphereCounter(1)
{
    buildUi();
    buildDocument();
    buildTreeModel();

    m_txMgr = std::make_shared<TransactionManager>(m_doc);
    m_actionMgr = std::make_shared<ActionManager>();
    m_ctx = std::make_shared<CommandContext>();
    m_ctx->doc = m_doc;
    m_ctx->txMgr = m_txMgr;
    m_ctx->renderSystem = nullptr; // 稍后在 RenderSystem 创建后赋值
    m_ctx->mainWindow = this;

    std::shared_ptr<IRenderView> renderView = IRenderView::createRenderView();
    setCentralWidget(renderView->widget());
    std::shared_ptr<GraphicsScene> grepScene = std::make_shared<GraphicsScene>();
    MeshGenerator meshGenerator;
    m_renderSystem = std::make_shared<RenderSystem>(grepScene, meshGenerator, renderView);
    m_ctx->renderSystem = m_renderSystem.get();

    renderView->SetOnPicked([&](ObjectId id)
        {
            m_doc->SetSelected(id);
            m_renderSystem->GetRenderView()->SetSelected(id);
            SelectInTree(id);
            UpdateProperties(id);
            RefreshActions();
        });
    TessellationOptions tessellationOptions;
    m_renderSystem->SyncFromDocument(m_doc, tessellationOptions);
    m_renderSystem->Refresh();

    // Build Action-driven UI from XML config
    // Try multiple paths to locate ui_layout.xml
    QStringList searchPaths;
    searchPaths << QCoreApplication::applicationDirPath() + "/config/ui_layout.xml";
    searchPaths << QCoreApplication::applicationDirPath() + "/../config/ui_layout.xml";
    searchPaths << QCoreApplication::applicationDirPath() + "/../../config/ui_layout.xml";
    searchPaths << QCoreApplication::applicationDirPath() + "/../../../config/ui_layout.xml";

    bool loaded = false;
    for (const auto& path : searchPaths)
    {
        if (QFile::exists(path))
        {
            loaded = ActionUIBuilder::BuildFromXml(path, this, *m_actionMgr, m_ctx);
            if (loaded)
            {
                qDebug() << "Loaded UI layout from:" << path;
                break;
            }
        }
    }

    if (!loaded)
    {
        qWarning() << "Could not find ui_layout.xml in any search path, building fallback UI";
        // Fallback: build minimal menu manually
        CommandUIBuilder::BuildFallback(this, CommandRegistry::Instance(), m_ctx);
    }

    // 初始状态同步（undo/redo/delete 等按钮按当前文档状态置灰/可用）
    RefreshActions();
}

void MainWindow::buildUi()
{
    setWindowTitle("CAD Demo");
    resize(1200, 800);

    // Central widget will be set later by the render view
    // (setCentralWidget is called in the constructor after buildUi)

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

    // Note: Menus and toolbars are now built by CommandUIBuilder
}

void MainWindow::buildDocument()
{
    m_doc = std::make_shared<Document>("Document");
    // Start with an empty document — user creates objects via commands
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
        item->setData(QVariant::fromValue<qulonglong>(static_cast<qulonglong>(obj->GetObjectId())), Role_ObjectId);
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

void MainWindow::UpdateProperties(ObjectId id)
{
    m_updatingProps = true;
    m_propModel->setRowCount(0);

    if (id == 0) { m_updatingProps = false; return; }

    std::weak_ptr<IObject> wobj = m_doc->GetobjectById(id);
    std::shared_ptr<IObject> obj = wobj.lock();
    if (!obj) { m_updatingProps = false; return; }

    std::vector<ParameterItem> params;
    obj->GetParameters(params);
    int row = 0;
    for (const ParameterItem& param : params)
    {
        SetPropRow(row++, param.Name.c_str(), param.Value.c_str(), id, param.Key, param.Editable);
    }
    m_updatingProps = false;
}

void MainWindow::UpdatePropertiesById(ObjectId id)
{
    if (id == 0)
        m_propModel->setRowCount(0);
    else
        UpdateProperties(id);
}

void MainWindow::RebuildAfterCommand()
{
    rebuildTreeModel();
    RefreshActions();
}

void MainWindow::RefreshActions()
{
    if (m_actionMgr && m_ctx)
        m_actionMgr->RefreshAll(*m_ctx);
}

void MainWindow::SetPropRow(int row, const QString& name, const QString& value
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
    if (m_updatingTree) return;

    if (!current.isValid())
    {
        m_doc->SetSelected(0);
        m_renderSystem->GetRenderView()->SetSelected(0);
        UpdateProperties(0);
        RefreshActions();
        return;
    }

    auto idVar = current.data(Role_ObjectId);
    if (!idVar.isValid())
    {
        // Clicked on a non-object node (e.g. "Document" or "Objects")
        m_doc->SetSelected(0);
        m_renderSystem->GetRenderView()->SetSelected(0);
        UpdateProperties(0);
        RefreshActions();
        return;
    }

    ObjectId id = static_cast<ObjectId>(idVar.toULongLong());
    m_doc->SetSelected(id);
    m_renderSystem->GetRenderView()->SetSelected(id);
    UpdateProperties(id);
    RefreshActions();
}

void MainWindow::onTreeDoubleClicked(const QModelIndex& idx)
{
    const auto* obj = objectFromIndex(idx);
    if (obj) {
        qDebug() << "Double clicked object:" << QString::fromStdString(obj->GetObjectName());
    }
}

void MainWindow::OnPropItemChanged(QStandardItem* item)
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
    RefreshActions();
}

void MainWindow::SelectInTree(ObjectId id)
{
    if (!m_docTreeModel || !m_docTreeView) return;

    m_updatingTree = true;

    if (id == 0)
    {
        m_docTreeView->clearSelection();
        m_updatingTree = false;
        return;
    }

    // Search through the "Objects" node children for matching ObjectId
    QStandardItem* root = m_docTreeModel->invisibleRootItem();
    if (root->rowCount() == 0) { m_updatingTree = false; return; }

    QStandardItem* docItem = root->child(0);
    if (!docItem || docItem->rowCount() == 0) { m_updatingTree = false; return; }

    QStandardItem* objsItem = docItem->child(0);
    if (!objsItem) { m_updatingTree = false; return; }

    for (int i = 0; i < objsItem->rowCount(); ++i)
    {
        QStandardItem* item = objsItem->child(i);
        auto idVar = item->data(Role_ObjectId);
        if (idVar.isValid() && static_cast<ObjectId>(idVar.toULongLong()) == id)
        {
            m_docTreeView->setCurrentIndex(item->index());
            m_updatingTree = false;
            return;
        }
    }

    // Not found — clear selection
    m_docTreeView->clearSelection();
    m_updatingTree = false;
}

void MainWindow::SyncAndRefresh(bool isAll)
{
    TessellationOptions tessellationOptions;
    m_renderSystem->SyncFromDocument(m_doc, tessellationOptions, isAll);
    m_renderSystem->Refresh(isAll);
}
