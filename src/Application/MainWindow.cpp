#include "MainWindow.h"
#include "Document.h"
#include "Object.h"

#include <QItemSelectionModel>
#include <QHeaderView>
#include <QDebug>
#include <QDockWidget>
#include <QTreeView>
#include <QStandardItemModel>


using namespace cadutils;

static constexpr int Role_ObjectPtr = Qt::UserRole + 100; // 存 Object* 指针

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    buildUi();
    buildDocument();
    buildTreeModel();
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
    m_propView->setEditTriggers(QAbstractItemView::NoEditTriggers);
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
}

void MainWindow::buildDocument()
{
    m_doc = std::make_shared<Document>("Document");
    m_doc->add(std::make_shared<Box>());
    m_doc->add(std::make_shared<Sphere>());
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

    for (const std::shared_ptr<Object>& obj : m_doc->objects())
    {
        QStandardItem* item = new QStandardItem(QString::fromStdString(obj->name()));
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

const Object* MainWindow::objectFromIndex(const QModelIndex& idx) const
{
    if (!idx.isValid()) 
        return nullptr;
    auto v = idx.data(Role_ObjectPtr);
    if (!v.isValid()) 
        return nullptr;
    auto ptr = reinterpret_cast<const Object*>(v.value<quintptr>());
    return ptr;
}

void MainWindow::buildPropertyModel(const Object* obj)
{
    m_propModel->removeRows(0, m_propModel->rowCount());

    if (!obj) return;

    for (const auto& [k, v] : obj->properties())
    {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::fromStdString(k));
        row << new QStandardItem(QString::fromStdString(v));
        m_propModel->appendRow(row);
    }
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
        qDebug() << "Double clicked object:" << QString::fromStdString(obj->name());
    }
}
