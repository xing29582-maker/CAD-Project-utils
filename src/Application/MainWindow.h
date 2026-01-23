#pragma once

#include "AppExport.h"
#include "NameDefine.h"

#include <QMainWindow>

class QTreeView;
class QStandardItemModel;

namespace cadutils 
{
    class Document;
    class Object;
    class RenderSystem;

    class CADUTILS_APP_API MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget* parent = nullptr);

    private:
        void buildUi();
        void buildDocument();   // 造一些数据
        void buildTreeModel();  // 左侧树
        void buildPropertyModel(const Object* obj); // 右侧属性
        const Object* objectFromIndex(const QModelIndex& idx) const;
    private slots:
        void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
        void onTreeDoubleClicked(const QModelIndex& idx);

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
    };

} // namespace cadutils
