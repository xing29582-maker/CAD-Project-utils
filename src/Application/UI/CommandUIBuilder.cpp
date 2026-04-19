#include "CommandUIBuilder.h"
#include "CommandRegistry.h"

#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QFile>
#include <QXmlStreamReader>
#include <QKeySequence>
#include <QDebug>
#include <QStringList>

namespace cadutils
{
    namespace
    {
        QAction* CreateActionForCommand(QMainWindow* window,
                                        std::unique_ptr<ICommand> command,
                                        std::shared_ptr<CommandContext> ctx,
                                        const QString& shortcutText = QString())
        {
            if (!window || !command || !ctx)
                return nullptr;

            QAction* action = new QAction(QString::fromStdString(command->GetName()), window);

            if (!shortcutText.isEmpty())
                action->setShortcut(QKeySequence(shortcutText));

            // Transfer command ownership to lambda capture so command lives with QAction
            auto liveCommand = std::shared_ptr<ICommand>(std::move(command));

            QObject::connect(action, &QAction::triggered, window, [liveCommand, ctx]()
            {
                if (!liveCommand->CanExecute(*ctx))
                    return;
                liveCommand->Execute(*ctx);
            });

            return action;
        }

        bool ParseMenu(QXmlStreamReader& xml,
                       QMainWindow* window,
                       CommandRegistry& registry,
                       std::shared_ptr<CommandContext> ctx)
        {
            const auto attrs = xml.attributes();
            QString menuName = attrs.value("name").toString();
            QMenu* menu = window->menuBar()->addMenu(menuName);

            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Menu"))
            {
                xml.readNext();

                if (xml.tokenType() == QXmlStreamReader::StartElement)
                {
                    if (xml.name() == "Item")
                    {
                        const auto itemAttrs = xml.attributes();
                        QString commandId = itemAttrs.value("command").toString();
                        QString shortcut = itemAttrs.value("shortcut").toString();

                        auto command = registry.Create(commandId.toStdString());
                        if (!command)
                        {
                            qWarning() << "Command not found:" << commandId;
                            xml.skipCurrentElement();
                            continue;
                        }

                        QAction* action = CreateActionForCommand(window, std::move(command), ctx, shortcut);
                        if (action)
                            menu->addAction(action);

                        xml.skipCurrentElement();
                    }
                    else if (xml.name() == "Separator")
                    {
                        menu->addSeparator();
                        xml.skipCurrentElement();
                    }
                }
            }
            return true;
        }

        bool ParseToolBar(QXmlStreamReader& xml,
                          QMainWindow* window,
                          CommandRegistry& registry,
                          std::shared_ptr<CommandContext> ctx)
        {
            const auto attrs = xml.attributes();
            QString toolbarName = attrs.value("name").toString();
            QToolBar* toolbar = window->addToolBar(toolbarName);

            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "ToolBar"))
            {
                xml.readNext();

                if (xml.tokenType() == QXmlStreamReader::StartElement)
                {
                    if (xml.name() == "Item")
                    {
                        const auto itemAttrs = xml.attributes();
                        QString commandId = itemAttrs.value("command").toString();
                        QString shortcut = itemAttrs.value("shortcut").toString();

                        auto command = registry.Create(commandId.toStdString());
                        if (!command)
                        {
                            qWarning() << "Command not found:" << commandId;
                            xml.skipCurrentElement();
                            continue;
                        }

                        QAction* action = CreateActionForCommand(window, std::move(command), ctx, shortcut);
                        if (action)
                            toolbar->addAction(action);

                        xml.skipCurrentElement();
                    }
                    else if (xml.name() == "Separator")
                    {
                        toolbar->addSeparator();
                        xml.skipCurrentElement();
                    }
                }
            }
            return true;
        }
    }

    bool CommandUIBuilder::BuildFromXml(const QString& xmlPath,
                                        QMainWindow* window,
                                        CommandRegistry& registry,
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

        while (!xml.atEnd())
        {
            xml.readNext();

            if (xml.tokenType() == QXmlStreamReader::StartElement)
            {
                if (xml.name() == "Menu")
                {
                    ParseMenu(xml, window, registry, ctx);
                }
                else if (xml.name() == "ToolBar")
                {
                    ParseToolBar(xml, window, registry, ctx);
                }
            }
        }

        if (xml.hasError())
        {
            qWarning() << "XML parse error:" << xml.errorString();
            return false;
        }

        return true;
    }

    void CommandUIBuilder::BuildFallback(QMainWindow* window,
                                         CommandRegistry& registry,
                                         std::shared_ptr<CommandContext> ctx)
    {
        if (!window || !ctx)
            return;

        QMenu* editMenu = window->menuBar()->addMenu("Edit");
        QToolBar* toolbar = window->addToolBar("Edit");

        const QStringList preferredOrder = {
            "cmd.undo",
            "cmd.redo",
            "cmd.add_sphere",
            "cmd.delete_selected"
        };

        for (const auto& id : preferredOrder)
        {
            auto command = registry.Create(id.toStdString());
            if (!command)
                continue;

            QAction* action = CreateActionForCommand(window, std::move(command), ctx);
            if (!action)
                continue;

            editMenu->addAction(action);
            toolbar->addAction(action);

            if (id == "cmd.redo")
            {
                editMenu->addSeparator();
                toolbar->addSeparator();
            }
        }
    }

} // namespace cadutils