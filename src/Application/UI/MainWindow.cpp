#include "MainWindow.h"
#include "Document.h"
#include "IObject.h"
#include "Point3d.h"
#include "IRenderView.h"
#include "GraphicsScene.h"
#include "MeshGenerator.h"
#include "RenderSystem.h"
#include "TessellationOptions.h"

#include <QItemSelectionModel>
#include <QHeaderView>
#include <QDebug>
#include <QDockWidget>
#include <QTreeView>
#include <QStandardItemModel>

using namespace cadutils;
using namespace std;
namespace
{
    static constexpr int Role_ObjectPtr = Qt::UserRole + 100; // 存 Object* 指针

    static constexpr int Role_ObjectId = Qt::UserRole + 200;
    static constexpr int Role_PropKey = Qt::UserRole + 201;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_updatingProps(false)
{
    buildUi();
    buildDocument();
    buildTreeModel();
    std::shared_ptr<IRenderView> renderView = IRenderView::createRenderView();
    setCentralWidget(renderView->widget());
    std::shared_ptr<GraphicsScene> grepScene = std::make_shared<GraphicsScene>();
    MeshGenerator meshGenerator;
    m_renderSystem = std::make_shared<RenderSystem>(grepScene , meshGenerator , renderView);
    renderView->SetOnPicked([&](ObjectId id)
        {
            m_renderSystem->GetRenderView()->SetSelected(id);     // 视口高亮       
            m_doc->SetSelected(id);
            UpdateProperties(id);             // 右侧属性刷新
            //SelectTreeById(id);                  // 左侧树选中（可选）

        });
    TessellationOptions tessellationOptions;
    m_renderSystem->SyncFromDocument(m_doc , tessellationOptions);
    m_renderSystem->Refresh();
}

void MainWindow::buildUi()
{
    setWindowTitle("CAD Demo");
    resize(1200, 800);

    // 中央占位
    m_viewport = new QWidget(this);
    m_viewport->setMinimumWidth(600);
    setCentralWidget(m_viewport);

    // 左侧 dock：Document Tree
    QDockWidget* leftDock = new QDockWidget("Document", this);
    leftDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_docTreeView = new QTreeView(leftDock);
    m_docTreeView->setHeaderHidden(true);
    m_docTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftDock->setWidget(m_docTreeView);
    addDockWidget(Qt::LeftDockWidgetArea, leftDock);

    // 右侧 dock：Properties
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

    // 属性表两列：Name / Value
    m_propModel = new QStandardItemModel(this);
    m_propModel->setColumnCount(2);
    m_propModel->setHeaderData(0, Qt::Horizontal, "Name");
    m_propModel->setHeaderData(1, Qt::Horizontal, "Value");
    m_propView->setModel(m_propModel);
    m_propView->header()->setStretchLastSection(true);

    connect(m_propModel, &QStandardItemModel::itemChanged,
        this, &MainWindow::OnPropItemChanged);
}

void MainWindow::buildDocument()
{
    m_doc = std::make_shared<Document>("Document");
    m_doc->add(std::dynamic_pointer_cast<IObject>(IObject::CreateSphereObject("Sphere", Point3d(0, 0, 0) ,50.0)));
}


void MainWindow::buildTreeModel()
{
    m_docTreeModel = new QStandardItemModel(this);
    QStandardItem* root = m_docTreeModel->invisibleRootItem();

    // 顶层：Document
    QStandardItem* docItem = new QStandardItem(QString::fromStdString(m_doc->name()));
    docItem->setEditable(false);
    root->appendRow(docItem);

    // 子节点：Objects
    QStandardItem* objsItem = new QStandardItem("Objects");
    objsItem->setEditable(false);
    docItem->appendRow(objsItem);

    for (const auto& obj : m_doc->GetObjects())
    {
        QStandardItem* item = new QStandardItem(QString::fromStdString(obj->GetObjectName()));
        item->setEditable(false);

        // 保存 Object*，注意：Object 生命周期由 m_doc 管理，这里存裸指针没问题
        item->setData(QVariant::fromValue(reinterpret_cast<quintptr>(obj.get())), Role_ObjectPtr);
        objsItem->appendRow(item);
    }

    m_docTreeView->setModel(m_docTreeModel);
    m_docTreeView->expandAll();

    // 连接 selection changed
    auto* sel = m_docTreeView->selectionModel();
    connect(sel, &QItemSelectionModel::currentChanged,
        this, &MainWindow::onTreeSelectionChanged);

    connect(m_docTreeView, &QTreeView::doubleClicked,
        this, &MainWindow::onTreeDoubleClicked);
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
        SetPropRow(row++, param.Name.c_str(),param.Value.c_str(), id, param.Key, param.Editable);
    }
}

void cadutils::MainWindow::SetPropRow(int row, const QString& name, const QString& value 
    ,ObjectId objId, ParamKey key, bool editable)
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
    // 关键：把“这格对应哪个对象、哪个属性”存起来
    valueItem->setData(QVariant::fromValue<qulonglong>(static_cast<qulonglong>(objId)), Role_ObjectId);
    valueItem->setData(static_cast<int>(key), Role_PropKey);
    m_propModel->setItem(row, 1, valueItem);
}

void MainWindow::buildPropertyModel(const IObject* obj)
{
    m_propModel->removeRows(0, m_propModel->rowCount());

    if (!obj) return;

    //for (const auto& [k, v] : obj->properties())
    //{
    //    QList<QStandardItem*> row;
    //    row << new QStandardItem(QString::fromStdString(k));
    //    row << new QStandardItem(QString::fromStdString(v));
    //    m_propModel->appendRow(row);
    //}
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

    // 只处理 Value 列（第 1 列）
    if (item->column() != 1) return;

    auto objIdVar = item->data(Role_ObjectId);
    auto keyVar = item->data(Role_PropKey);
    if (!objIdVar.isValid() || !keyVar.isValid()) return;

    ObjectId objId = static_cast<ObjectId>(objIdVar.toULongLong());
    ParamKey key = static_cast<ParamKey>(keyVar.toInt());

    // 找对象
    const std::shared_ptr<IObject> obj = m_doc->GetobjectById(objId);
    if (!obj) return;

    obj->SetParameters(key, item->text().toStdString());

    TessellationOptions tessellationOptions;
    m_doc->MarkDirty(objId);
    m_renderSystem->SyncFromDocument(m_doc, tessellationOptions, false);
    m_renderSystem->Refresh(false);
}
