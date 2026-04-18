#pragma once

#include "NameDefine.h"
#include "ParameterItem.h"

#include <QMainWindow>

class QTreeView;
class QStandardItemModel;
class QStandardItem;
class QAction;

namespace cadutils
{
    class Document;
    class IObject;
    class RenderSystem;
    class TransactionManager;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);

    private:
        void buildUi();
        void buildDocument();
        void buildTreeModel();
        void rebuildTreeModel();
        void buildPropertyModel(const IObject* obj);
        const IObject* objectFromIndex(const QModelIndex& idx) const;
        void UpdateProperties(ObjectId id);
        void SetPropRow(int row, const QString& name, const QString& value
        , ObjectId objId, ParamKey key, bool editable);
        void SyncAndRefresh(bool isAll);

    private slots:
        void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void onTreeDoubleClicked(const QModelIndex& idx);
        void OnPropItemChanged(QStandardItem* item);
        void OnUndo();
        void OnRedo();
        void OnAddSphere();
        void OnDeleteSelected();

    private:
        QWidget* m_viewport = nullptr;

        // Left: Document Tree
        QTreeView* m_docTreeView = nullptr;
        QStandardItemModel* m_docTreeModel = nullptr;

        // Right: Property Tree
        QTreeView* m_propView = nullptr;
        QStandardItemModel* m_propModel = nullptr;

        // Data
        std::shared_ptr<Document> m_doc;
        std::shared_ptr<RenderSystem> m_renderSystem;
        std::shared_ptr<TransactionManager> m_txMgr;

        // Actions
        QAction* m_undoAction = nullptr;
        QAction* m_redoAction = nullptr;
        QAction* m_addSphereAction = nullptr;
        QAction* m_deleteAction = nullptr;

        bool m_updatingProps;
        int m_sphereCounter = 1;
    };

} // namespace cadutils
