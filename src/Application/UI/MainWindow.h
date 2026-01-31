#pragma once

#include "NameDefine.h"
#include "ParameterItem.h"

#include <QMainWindow>

class QTreeView;
class QStandardItemModel;
class QStandardItem;

namespace cadutils 
{
    class Document;
    class IObject;
    class RenderSystem;

    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);

    private:
        void buildUi();
        void buildDocument();   // 造一些数据
        void buildTreeModel();  // 左侧树
        void buildPropertyModel(const IObject* obj); // 右侧属性
        const IObject* objectFromIndex(const QModelIndex& idx) const;
        void UpdateProperties(ObjectId id);
        void SetPropRow(int row, const QString& name, const QString& value
        , ObjectId objId, ParamKey key, bool editable);
    private slots:
        void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void onTreeDoubleClicked(const QModelIndex& idx);
        void OnPropItemChanged(QStandardItem* item);
    private:
        // 中间视图（后面换 OSG）
        QWidget* m_viewport = nullptr;

        // 左：Document Tree
        QTreeView* m_docTreeView = nullptr;
        QStandardItemModel* m_docTreeModel = nullptr;

        // 右：Property Tree
        QTreeView* m_propView = nullptr;
        QStandardItemModel* m_propModel = nullptr;

        // 数据
         std::shared_ptr<Document> m_doc;

         std::shared_ptr<RenderSystem> m_renderSystem;

         bool m_updatingProps;
    };

} // namespace cadutils
