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
    class ActionManager;
    struct CommandContext; // 注意：CommandContext 在 ICommand.h 中以 struct 定义，前置声明必须一致（否则 MSVC 修饰名 U/V 不匹配）

    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);

        // Public methods for command system
        void RebuildAfterCommand();
        void UpdatePropertiesById(ObjectId id);
        void SyncAndRefresh(bool isAll);

        // 刷新所有 Action 状态（enabled/visible/checked 来自命令，经 ActionManager 求值）
        void RefreshActions();

        // Accessors for command context setup
        std::shared_ptr<Document> GetDocument() const { return m_doc; }
        std::shared_ptr<TransactionManager> GetTransactionManager() const { return m_txMgr; }
        RenderSystem* GetRenderSystem() const { return m_renderSystem.get(); }
        int& SphereCounter() { return m_sphereCounter; }
        int& BoxCounter() { return m_boxCounter; }
        int& CylinderCounter() { return m_cylinderCounter; }

    private:
        void buildUi();
        void buildDocument();
        void buildTreeModel();
        void rebuildTreeModel();
        void buildPropertyModel(const IObject* obj);
        const IObject* objectFromIndex(const QModelIndex& idx) const;
        void UpdateProperties(ObjectId id);
        void SelectInTree(ObjectId id);
        void SetPropRow(int row, const QString& name, const QString& value
        , ObjectId objId, ParamKey key, bool editable);

    private slots:
        void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void onTreeDoubleClicked(const QModelIndex& idx);
        void OnPropItemChanged(QStandardItem* item);

    private:
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
        std::shared_ptr<ActionManager> m_actionMgr;
        std::shared_ptr<CommandContext> m_ctx;

        bool m_updatingProps = false;
        bool m_updatingTree = false;
        int m_sphereCounter = 1;
        int m_boxCounter = 1;
        int m_cylinderCounter = 1;
    };

} // namespace cadutils
