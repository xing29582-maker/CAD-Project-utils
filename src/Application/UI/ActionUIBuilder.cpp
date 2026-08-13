#include "ActionUIBuilder.h"
#include "ActionManager.h"
#include "IAction.h"
#include "CommandAction.h"
#include "CommandRegistry.h"

#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QFile>
#include <QXmlStreamReader>
#include <QKeySequence>
#include <QPointer>
#include <QDebug>

#include <string>
#include <unordered_map>
#include <vector>

namespace cadutils
{
    namespace
    {
        // ---- Action 定义（<Actions> 定义区）----
        struct ActionDef
        {
            std::string id;
            std::string name;
            std::string commandId;
            std::string icon;
            std::string shortcut;
            std::vector<std::string> subRefs; // 非空表示展示容器（子菜单/下拉）
        };

        // ---- 挂载项（菜单/工具栏）----
        struct MountEntry
        {
            enum class Kind
            {
                ActionRef,   // <ActionRef id="..."/>
                ItemCommand, // <Item command="..." shortcut="..."/>（兼容旧格式）
                Separator
            };
            Kind kind = Kind::Separator;
            std::string actionId;
            std::string commandId;
            std::string shortcut;
        };

        struct MenuDef
        {
            std::string name;
            std::vector<MountEntry> entries;
        };

        struct ToolBarDef
        {
            std::string name;
            std::vector<MountEntry> entries;
        };

        // ---- QActionSink：把 IAction 缓存状态同步到 QAction ----
        // 生命周期由 QObject 树（parent = window）管理。
        // 说明：MainWindow 析构时成员（ActionManager/IAction）先于 Qt children（本对象）销毁，
        // 因此析构函数不访问 m_action；OnActionStateChanged 仅在 IAction 存活期间（Refresh）被调用。
        class QActionSink : public QObject, public ActionSink
        {
        public:
            QActionSink(IAction* action, QAction* qa, QObject* parent)
                : QObject(parent), m_action(action), m_qa(qa)
            {
                if (m_action)
                    m_action->AttachSink(this);
            }

            ~QActionSink() override = default;

            void OnActionStateChanged(const std::string&) override
            {
                if (m_action && m_qa)
                {
                    m_qa->setEnabled(m_action->GetEnabled());
                    m_qa->setVisible(m_action->GetVisible());
                    m_qa->setChecked(m_action->GetChecked());
                }
            }

            void OnActionExecuted(const std::string&) override {}

        private:
            IAction* m_action;
            QPointer<QAction> m_qa;
        };

        // 为叶子 Action 创建 QAction 并绑定（triggered → ActionManager::PushAction）
        QAction* CreateActionForAction(const std::shared_ptr<IAction>& action,
                                       QMainWindow* window,
                                       ActionManager& actionMgr,
                                       const std::shared_ptr<CommandContext>& ctx)
        {
            if (!action || !window)
                return nullptr;

            auto* qa = new QAction(QString::fromStdString(action->GetName()), window);

            const std::string shortcut = action->GetShortcut();
            if (!shortcut.empty())
                qa->setShortcut(QKeySequence(QString::fromStdString(shortcut)));

            // 初始状态（后续由 Refresh → sink 同步）
            qa->setEnabled(action->GetEnabled());
            qa->setVisible(action->GetVisible());
            qa->setChecked(action->GetChecked());

            const std::string actionId = action->GetId();
            QObject::connect(qa, &QAction::triggered, window, [&actionMgr, ctx, actionId]()
            {
                actionMgr.PushAction(actionId, *ctx);
            });

            new QActionSink(action.get(), qa, window);
            return qa;
        }

        // 挂载到菜单：容器 → 子菜单；叶子 → 菜单项
        void MountToMenu(const std::string& refId,
                         QMenu* menu,
                         QMainWindow* window,
                         ActionManager& actionMgr,
                         const std::shared_ptr<CommandContext>& ctx,
                         const std::unordered_map<std::string, ActionDef>& defs)
        {
            auto it = defs.find(refId);
            if (it != defs.end() && !it->second.subRefs.empty())
            {
                QMenu* sub = menu->addMenu(QString::fromStdString(it->second.name));
                for (const auto& subRef : it->second.subRefs)
                    MountToMenu(subRef, sub, window, actionMgr, ctx, defs);
                return;
            }

            auto action = actionMgr.Find(refId);
            if (!action)
            {
                qWarning() << "Action not found:" << QString::fromStdString(refId);
                return;
            }
            if (QAction* qa = CreateActionForAction(action, window, actionMgr, ctx))
                menu->addAction(qa);
        }

        // 挂载到工具栏：容器 → 下拉按钮；叶子 → 工具按钮
        void MountToToolBar(const std::string& refId,
                            QToolBar* toolbar,
                            QMainWindow* window,
                            ActionManager& actionMgr,
                            const std::shared_ptr<CommandContext>& ctx,
                            const std::unordered_map<std::string, ActionDef>& defs)
        {
            auto it = defs.find(refId);
            if (it != defs.end() && !it->second.subRefs.empty())
            {
                auto* sub = new QMenu(QString::fromStdString(it->second.name), window);
                for (const auto& subRef : it->second.subRefs)
                    MountToMenu(subRef, sub, window, actionMgr, ctx, defs);
                QAction* qa = toolbar->addAction(QString::fromStdString(it->second.name));
                qa->setMenu(sub);
                return;
            }

            auto action = actionMgr.Find(refId);
            if (!action)
            {
                qWarning() << "Action not found:" << QString::fromStdString(refId);
                return;
            }
            if (QAction* qa = CreateActionForAction(action, window, actionMgr, ctx))
                toolbar->addAction(qa);
        }

        // 兼容旧格式 <Item command="..."/>：隐式 Action（id = command id），已注册则复用
        void MountItemCommand(const MountEntry& e,
                              QMenu* menu,
                              QMainWindow* window,
                              ActionManager& actionMgr,
                              const std::shared_ptr<CommandContext>& ctx)
        {
            auto action = actionMgr.Find(e.commandId);
            if (!action)
            {
                auto cmd = CommandRegistry::Instance().Create(e.commandId);
                if (!cmd)
                {
                    qWarning() << "Command not found:" << QString::fromStdString(e.commandId);
                    return;
                }
                action = std::make_shared<CommandAction>(
                    e.commandId, cmd->GetName(), std::move(cmd), "", e.shortcut);
                actionMgr.Register(action);
            }
            if (QAction* qa = CreateActionForAction(action, window, actionMgr, ctx))
                menu->addAction(qa);
        }

        void MountItemCommandToToolBar(const MountEntry& e,
                                       QToolBar* toolbar,
                                       QMainWindow* window,
                                       ActionManager& actionMgr,
                                       const std::shared_ptr<CommandContext>& ctx)
        {
            auto action = actionMgr.Find(e.commandId);
            if (!action)
            {
                auto cmd = CommandRegistry::Instance().Create(e.commandId);
                if (!cmd)
                {
                    qWarning() << "Command not found:" << QString::fromStdString(e.commandId);
                    return;
                }
                action = std::make_shared<CommandAction>(
                    e.commandId, cmd->GetName(), std::move(cmd), "", e.shortcut);
                actionMgr.Register(action);
            }
            if (QAction* qa = CreateActionForAction(action, window, actionMgr, ctx))
                toolbar->addAction(qa);
        }

        // 解析 <Menu> 或 <ToolBar> 内的挂载项（endTag 区分）
        void ParseMountEntries(QXmlStreamReader& xml, const QString& endTag, std::vector<MountEntry>& out)
        {
            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == endTag))
            {
                xml.readNext();
                if (xml.tokenType() != QXmlStreamReader::StartElement)
                    continue;

                MountEntry e;
                if (xml.name() == "ActionRef")
                {
                    e.kind = MountEntry::Kind::ActionRef;
                    e.actionId = xml.attributes().value("id").toString().toStdString();
                    out.push_back(std::move(e));
                    xml.skipCurrentElement();
                }
                else if (xml.name() == "Item")
                {
                    e.kind = MountEntry::Kind::ItemCommand;
                    e.commandId = xml.attributes().value("command").toString().toStdString();
                    e.shortcut = xml.attributes().value("shortcut").toString().toStdString();
                    out.push_back(std::move(e));
                    xml.skipCurrentElement();
                }
                else if (xml.name() == "Separator")
                {
                    e.kind = MountEntry::Kind::Separator;
                    out.push_back(std::move(e));
                    xml.skipCurrentElement();
                }
            }
        }

    } // namespace

    bool ActionUIBuilder::BuildFromXml(const QString& xmlPath,
                                       QMainWindow* window,
                                       ActionManager& actionMgr,
                                       std::shared_ptr<CommandContext> ctx)
    {
        if (!window || !ctx)
            return false;

        QFile file(xmlPath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << "Failed to open UI layout XML:" << xmlPath;
            return false;
        }

        QXmlStreamReader xml(&file);

        std::unordered_map<std::string, ActionDef> defs;
        std::vector<MenuDef> menus;
        std::vector<ToolBarDef> toolbars;

        while (!xml.atEnd())
        {
            xml.readNext();
            if (xml.tokenType() != QXmlStreamReader::StartElement)
                continue;

            if (xml.name() == "Actions")
            {
                // 定义区：<Action id name command icon shortcut>（可含子 <ActionRef>）
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Actions"))
                {
                    xml.readNext();
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Action")
                    {
                        ActionDef def;
                        def.id = xml.attributes().value("id").toString().toStdString();
                        def.name = xml.attributes().value("name").toString().toStdString();
                        def.commandId = xml.attributes().value("command").toString().toStdString();
                        def.icon = xml.attributes().value("icon").toString().toStdString();
                        def.shortcut = xml.attributes().value("shortcut").toString().toStdString();

                        while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Action"))
                        {
                            xml.readNext();
                            if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "ActionRef")
                            {
                                def.subRefs.push_back(xml.attributes().value("id").toString().toStdString());
                                xml.skipCurrentElement();
                            }
                        }

                        if (!def.id.empty())
                            defs[def.id] = std::move(def);
                    }
                }
            }
            else if (xml.name() == "MenuBar")
            {
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "MenuBar"))
                {
                    xml.readNext();
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Menu")
                    {
                        MenuDef mdef;
                        mdef.name = xml.attributes().value("name").toString().toStdString();
                        ParseMountEntries(xml, "Menu", mdef.entries);
                        menus.push_back(std::move(mdef));
                    }
                }
            }
            else if (xml.name() == "ToolBars")
            {
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "ToolBars"))
                {
                    xml.readNext();
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "ToolBar")
                    {
                        ToolBarDef tdef;
                        tdef.name = xml.attributes().value("name").toString().toStdString();
                        ParseMountEntries(xml, "ToolBar", tdef.entries);
                        toolbars.push_back(std::move(tdef));
                    }
                }
            }
        }

        if (xml.hasError())
        {
            qWarning() << "XML parse error:" << xml.errorString();
            return false;
        }

        // 注册叶子 Action（有 commandId 的）
        for (const auto& [id, def] : defs)
        {
            if (def.commandId.empty())
                continue; // 纯展示容器

            auto cmd = CommandRegistry::Instance().Create(def.commandId);
            if (!cmd)
            {
                qWarning() << "Command not found for action:" << QString::fromStdString(id)
                           << "->" << QString::fromStdString(def.commandId);
                continue;
            }
            auto action = std::make_shared<CommandAction>(
                def.id, def.name, std::move(cmd), def.icon, def.shortcut);
            if (!actionMgr.Register(action))
                qWarning() << "Duplicate action id:" << QString::fromStdString(id);
        }

        // 挂载菜单
        for (const auto& mdef : menus)
        {
            QMenu* menu = window->menuBar()->addMenu(QString::fromStdString(mdef.name));
            for (const auto& e : mdef.entries)
            {
                switch (e.kind)
                {
                case MountEntry::Kind::Separator:
                    menu->addSeparator();
                    break;
                case MountEntry::Kind::ActionRef:
                    MountToMenu(e.actionId, menu, window, actionMgr, ctx, defs);
                    break;
                case MountEntry::Kind::ItemCommand:
                    MountItemCommand(e, menu, window, actionMgr, ctx);
                    break;
                }
            }
        }

        // 挂载工具栏
        for (const auto& tdef : toolbars)
        {
            QToolBar* toolbar = window->addToolBar(QString::fromStdString(tdef.name));
            for (const auto& e : tdef.entries)
            {
                switch (e.kind)
                {
                case MountEntry::Kind::Separator:
                    toolbar->addSeparator();
                    break;
                case MountEntry::Kind::ActionRef:
                    MountToToolBar(e.actionId, toolbar, window, actionMgr, ctx, defs);
                    break;
                case MountEntry::Kind::ItemCommand:
                    MountItemCommandToToolBar(e, toolbar, window, actionMgr, ctx);
                    break;
                }
            }
        }

        return true;
    }

} // namespace cadutils
