# 命令系统设计文档

## 概述

命令系统将 UI 操作（菜单、工具栏按钮）与具体业务逻辑解耦。每个操作封装为一个 `ICommand` 实现类，通过 `REGISTER_COMMAND` 宏自动注册到全局 `CommandRegistry`。UI 布局由 XML 配置文件 (`config/ui_layout.xml`) 定义，运行时由 `CommandUIBuilder` 解析并构建菜单和工具栏。

## 架构

```
┌─────────────────────────────────────────────────────┐
│                    Application 层                     │
│                                                       │
│  ┌──────────────┐  ┌──────────────────────────────┐  │
│  │ MainWindow   │  │ Commands/                     │  │
│  │  构造时调用   │  │  UndoCommand                  │  │
│  │  UIBuilder   │  │  RedoCommand                  │  │
│  └──────┬───────┘  │  AddSphereCommand             │  │
│         │          │  DeleteSelectedCommand         │  │
│         │          │  (REGISTER_COMMAND 宏注册)      │  │
│         │          └──────────────────────────────┘  │
│         │                                             │
│  ┌──────▼──────────────────┐                         │
│  │ CommandUIBuilder        │                         │
│  │  读取 XML → 创建 QAction │                         │
│  │  绑定 ICommand::Execute │                         │
│  └──────┬──────────────────┘                         │
│         │                                             │
├─────────┼─────────────────────────────────────────────┤
│         │          Platform 层                         │
│         │                                             │
│  ┌──────▼──────────┐  ┌────────────────────┐         │
│  │ CommandRegistry │  │ ICommand 接口       │         │
│  │  单例注册表     │  │ CommandContext      │         │
│  │  Create(id)     │  │ CommandRegistrar    │         │
│  └─────────────────┘  │ REGISTER_COMMAND 宏 │         │
│                        └────────────────────┘         │
└───────────────────────────────────────────────────────┘
```

## 核心类

### ICommand (Platform/Public/ICommand.h)

命令抽象接口：

```cpp
class ICommand {
    virtual std::string GetId() const = 0;      // 唯一标识，如 "cmd.undo"
    virtual std::string GetName() const = 0;     // 显示名称
    virtual bool CanExecute(const CommandContext& ctx) const;  // 是否可执行
    virtual void Execute(CommandContext& ctx) = 0;             // 执行命令
};
```

### CommandContext (Platform/Public/ICommand.h)

命令执行上下文，包含命令执行所需的所有共享资源：

```cpp
struct CommandContext {
    std::shared_ptr<Document>           doc;
    std::shared_ptr<TransactionManager> txMgr;
    void* renderSystem;  // RenderSystem*，避免 Platform 层依赖 Qt
    void* mainWindow;    // MainWindow*，同上
};
```

### CommandRegistry (Platform/Public/CommandRegistry.h)

全局命令工厂注册表（单例）：

```cpp
class CommandRegistry {
    using FactoryFn = std::unique_ptr<ICommand>(*)();
    static CommandRegistry& Instance();
    void Register(const std::string& id, FactoryFn factory);
    std::unique_ptr<ICommand> Create(const std::string& id) const;
};
```

### REGISTER_COMMAND 宏

```cpp
REGISTER_COMMAND(cadutils::UndoCommand, "cmd.undo", UndoCommand)
```

展开为一个静态 `CommandRegistrar` 变量，在程序启动时自动将命令工厂注册到 `CommandRegistry`。

参数说明：
- 参数1：完整类名（含命名空间）
- 参数2：命令 ID 字符串
- 参数3：用于生成唯一变量名的标签（不含命名空间）

### CommandUIBuilder (Application/UI/CommandUIBuilder.h)

读取 XML 配置文件，为每个命令创建 `QAction`，构建菜单和工具栏：

```cpp
class CommandUIBuilder {
    static bool BuildFromXml(const QString& xmlPath,
                             QMainWindow* window,
                             CommandRegistry& registry,
                             std::shared_ptr<CommandContext> ctx);
};
```

## XML 配置文件格式

```xml
<?xml version="1.0" encoding="UTF-8"?>
<UILayout>
    <MenuBar>
        <Menu name="Edit">
            <Item command="cmd.undo" shortcut="Ctrl+Z" />
            <Item command="cmd.redo" shortcut="Ctrl+Y" />
            <Separator/>
            <Item command="cmd.add_sphere" />
            <Item command="cmd.delete_selected" shortcut="Delete" />
        </Menu>
    </MenuBar>
    <ToolBars>
        <ToolBar name="Main">
            <Item command="cmd.undo" />
            <Item command="cmd.redo" />
            <Separator/>
            <Item command="cmd.add_sphere" />
            <Item command="cmd.delete_selected" />
        </ToolBar>
    </ToolBars>
</UILayout>
```

- `command` 属性：对应 `ICommand::GetId()` 返回的命令 ID
- `shortcut` 属性（可选）：快捷键，格式同 `QKeySequence`
- `Separator`：分隔线

## 已注册命令

| 命令 ID | 类名 | 功能 |
|---------|------|------|
| cmd.undo | UndoCommand | 撤销上一步操作 |
| cmd.redo | RedoCommand | 重做上一步操作 |
| cmd.add_sphere | AddSphereCommand | 添加球体对象 |
| cmd.delete_selected | DeleteSelectedCommand | 删除选中对象 |

## 执行流程

1. 程序启动 → `REGISTER_COMMAND` 宏触发静态初始化，所有命令工厂注册到 `CommandRegistry`
2. `main()` 调用 `RegisterAllCommands()` 确保链接
3. `MainWindow` 构造函数创建 `CommandContext`，调用 `CommandUIBuilder::BuildFromXml()`
4. `CommandUIBuilder` 解析 XML，对每个 `<Item>` 调用 `CommandRegistry::Create()` 创建命令实例
5. 为每个命令创建 `QAction`，设置快捷键，connect `triggered` 信号到 lambda
6. Lambda 中调用 `ICommand::CanExecute()` 检查，通过后调用 `ICommand::Execute()`
7. 命令内部通过 `CommandContext` 访问 Document、TransactionManager、RenderSystem 等

## 扩展新命令

1. 在 `Application/Commands/` 下创建 `XxxCommand.h` 和 `XxxCommand.cpp`
2. 继承 `ICommand`，实现 `GetId()`、`GetName()`、`Execute()`
3. 在 `.cpp` 文件中添加 `REGISTER_COMMAND(cadutils::XxxCommand, "cmd.xxx", XxxCommand)`
4. 在 `config/ui_layout.xml` 中添加对应的 `<Item command="cmd.xxx" />`
5. 编译即可，无需修改其他文件

## 文件清单

| 文件 | 层 | 说明 |
|------|-----|------|
| Platform/Public/ICommand.h | Platform | ICommand 接口 + CommandContext |
| Platform/Public/CommandRegistry.h | Platform | 注册表 + 宏定义 |
| Platform/Private/CommandRegistry.cpp | Platform | 注册表实现 |
| Application/Commands/CommandHelper.h | Application | 辅助函数（类型转换、刷新） |
| Application/Commands/CommandHelper.cpp | Application | RefreshAfterCommand 实现 |
| Application/Commands/UndoCommand.h/.cpp | Application | 撤销命令 |
| Application/Commands/RedoCommand.h/.cpp | Application | 重做命令 |
| Application/Commands/AddSphereCommand.h/.cpp | Application | 添加球体命令 |
| Application/Commands/DeleteSelectedCommand.h/.cpp | Application | 删除选中命令 |
| Application/Commands/RegisterAllCommands.h/.cpp | Application | 链接保障 |
| Application/UI/CommandUIBuilder.h/.cpp | Application | XML 解析 + UI 构建 |
| config/ui_layout.xml | 配置 | UI 布局配置 |