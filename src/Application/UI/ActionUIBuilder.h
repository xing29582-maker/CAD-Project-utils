#pragma once

#include "ICommand.h"

#include <QString>
#include <memory>

class QMainWindow;

namespace cadutils
{
    class ActionManager;

    // 从 XML 构建 Action 定义区（<Actions>）与挂载区（<Menu>/<ToolBar>）。
    // - 新格式：<Action id command/> + <ActionRef id/>（同一 Action 可被菜单/工具栏多次引用，共享实例与状态）
    // - 兼容格式：<Item command="..."/>（隐式 Action，id 沿用 command id）
    // - 带子 ActionRef 列表的 Action 表现为子菜单/下拉（仅展示，执行父子关系由 ActionStack 运行时建立）
    class ActionUIBuilder
    {
    public:
        static bool BuildFromXml(const QString& xmlPath,
                                 QMainWindow* window,
                                 ActionManager& actionMgr,
                                 std::shared_ptr<CommandContext> ctx);
    };

} // namespace cadutils
