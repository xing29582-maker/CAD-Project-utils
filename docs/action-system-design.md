# Action 系统设计文档

> 版本：0.2（设计提案，尚未实现）
> 日期：2026-08-09
> 模块：Platform / Application
> 关联文档：[命令系统设计](command-system-design.md)、[后续开发建议](next-development-recommendations.md)

---

## 目录

1. [概述](#1-概述)
2. [背景与动机](#2-背景与动机)
3. [目标与非目标](#3-目标与非目标)
4. [核心概念](#4-核心概念)
5. [总体架构](#5-总体架构)
6. [核心接口契约](#6-核心接口契约)
7. [Command 状态模型与刷新机制](#7-command-状态模型与刷新机制)
8. [父子 Action 栈设计](#8-父子-action-栈设计)
9. [Action 复用机制](#9-action-复用机制)
10. [XML 配置扩展](#10-xml-配置扩展)
11. [交互流程](#11-交互流程)
12. [与现有系统的迁移路径](#12-与现有系统的迁移路径)
13. [边界情况与注意事项](#13-边界情况与注意事项)
14. [验收标准](#14-验收标准)
15. [文件清单](#15-文件清单)
16. [待决策问题](#16-待决策问题)

---

## 1. 概述

Action 是应用层 UI 交互的**可复用动作单元**。与常见的"Action 树 / 分组"模型不同，本设计采用**父子 Action 栈（Action Stack）**：

- 所有 Action 都是**普通 Action**，没有"组"的概念；
- 父 Action 通过栈结构挂起自身、压入子 Action，**框架自动调度最上层（栈顶）的 Action**；
- 子 Action 结束时，由**框架**把子 Action 产生的结果数据回传给父 Action，父 Action 继续执行。

Action 本身**不持有业务状态**：`enabled` / `visible` / `checked` 状态由被包装的 **Command** 提供，Action 只负责 UI 呈现与触发调度。

本设计在现有命令系统（`ICommand` / `CommandRegistry` / `CommandUIBuilder`）之上新增一层 Action 抽象，解决当前 UI 状态不刷新、动作无法复用、无法表达"多步交互流程"的问题。

---

## 2. 背景与动机

### 2.1 当前实现方式（现状）

- UI 布局由 [`config/ui_layout.xml`](../config/ui_layout.xml) 驱动，[`CommandUIBuilder`](../src/Application/UI/CommandUIBuilder.h) 解析 XML，为每个 `<Item command="...">` 调用 [`CommandRegistry::Create()`](../src/Platform/Public/CommandRegistry.h) 创建命令实例并生成 `QAction`。
- QAction 的 `triggered` 信号绑定 lambda，点击时先检查 `ICommand::CanExecute()` 再执行。

### 2.2 现有问题

| # | 问题 | 影响 |
|---|------|------|
| 1 | **QAction 的 enabled 状态从不刷新** | `cmd.undo` / `cmd.redo` / `cmd.delete_selected` 永远可点，点击后才拦截，用户无视觉反馈 |
| 2 | **每次构建 UI 都新建命令实例** | 菜单与工具栏中同一命令是两个独立实例，状态无法共享 |
| 3 | **只能表达"单步动作"** | 无法表达"选择图元 → 输入参数 → 创建对象"这类多步交互流程 |
| 4 | **状态与业务分离不足** | `enabled` 判断散落在 UI 层，命令无法独立回答"我现在可见吗 / 勾选吗" |
| 5 | **同类命令重复实现** | 添加球体/盒子/圆柱体逻辑相似，只能各写一个命令类，无法参数化复用 |

### 2.3 设计取向（三条原则）

1. **用栈取代静态分组**：父子关系是"执行时"的流程关系（谁挂起、谁在上层、数据怎么回流），而不是静态的菜单树；因此用执行栈表达，而非 Action 树/组。
2. **状态归属 Command**：Action 是无状态的 UI 壳，命令是状态的权威来源（`enabled` / `visible` / `checked`）。
3. **子 Action 普通化**：子 Action 与普通 Action 是同一个抽象，区别仅在"它在栈中的位置"以及"结束时结果回传给谁"。

---

## 3. 目标与非目标

### 3.1 目标

- 提供可复用的交互动作单元，支持菜单 / 工具栏等多挂载点共享同一 Action 实例。
- 提供**父子 Action 栈**：框架自动调度栈顶 Action，子 Action 结束自动回传数据给父 Action。
- `enabled` / `visible` / `checked` 状态由 **Command** 提供，Action 层负责读取与同步到 UI。
- 保持与现有命令系统兼容：现有 8 个命令**无需修改**即可接入（新增状态接口提供默认实现）。
- UI 布局继续由 XML 驱动，扩展而不破坏现有格式。

### 3.2 非目标（第一版不做）

- 不做动作录制 / 回放（宏）。
- 不做基于权限的角色隐藏（`visible` 预留语义，第一版默认恒可见）。
- 不做 Action 与文档的自动双向绑定，保持"刷新触发点"显式驱动。
- 不把 UI 元数据塞进命令类（命令保持无 Qt 依赖）。

---

## 4. 核心概念

### 4.1 Action（普通动作，唯一动作形态）

一次可触发的交互动作，**所有 Action（包括父子场景中的子 Action）都是同一个抽象**。包含：

- **身份**：全局唯一 `actionId`（如 `act.delete_selected`）。
- **元数据**：显示名称、tooltip、图标、默认快捷键。
- **命令引用**：持有被包装的 `ICommand` 实例（注册时创建一次，供状态读取与执行）。
- **触发**：`Trigger(ctx)` —— 压入执行栈并调用命令 `Execute`（或独立逻辑）。
- **状态读取**：`enabled` / `visible` / `checked` 均**转发给命令**（见第 7 节）。
- **生命周期**：由 ActionManager 持有单例，所有挂载点引用同一实例。

### 4.2 Command（业务逻辑 + 状态权威）

现有 `ICommand` 的扩展：除 `CanExecute` 外，新增 `IsVisible` / `IsChecked` 两个**可选虚方法（默认实现）**，作为 Action 状态的唯一来源。

```
ICommand（现有）
  ├─ GetId() / GetName()
  ├─ CanExecute(ctx)   → Action.enabled
  ├─ IsVisible(ctx)    → Action.visible   （新增，默认 true）
  └─ IsChecked(ctx)    → Action.checked   （新增，默认 false）
```

### 4.3 ActionStack（父子 Action 执行栈）

框架维护的**执行栈**，是"父子 Action"的载体：

- `Push(action)`：父 Action 挂起，子 Action 入栈，**框架自动调用新的栈顶**。
- `Pop()`：栈顶 Action 结束，出栈；其**结果数据由框架回传给新的栈顶（父 Action）**。
- `Peek()`：当前最上层 Action。
- 栈空 = 整个交互流程结束。

### 4.4 ActionManager（注册表 + 栈调度 + 状态刷新中枢）

- 注册 / 查找 Action（按 id）。
- 持有 `ActionStack`，提供 `Push` / `Pop` / `Peek` 及"自动调用栈顶"逻辑。
- 提供 `RefreshAll(ctx)` 统一刷新入口（求值各 Action 绑定的命令状态）。
- 由 MainWindow 在关键时机（选中变化、对象增删、事务栈变化、命令执行后、Load/New）触发刷新。

### 4.5 ActionSink（状态监听）

Action 状态变化 → 通知挂载点（QAction）同步。解耦 Action 模型与 Qt UI。

---

## 5. 总体架构

### 5.1 分层

```
┌──────────────────────────────────────────────────────────┐
│                     Application 层（Qt）                   │
│  ActionUIBuilder        Action 具体实现（包装 ICommand）    │
│  （XML → QAction 绑定）  （AddPrimitiveAction 等）          │
├──────────────────────────────────────────────────────────┤
│                     Platform 层（纯 C++，无 Qt）           │
│  IAction / ActionStack / ActionManager / ActionSink       │
│  （状态转发 + 栈调度 + 注册表 + 刷新中枢）                   │
├──────────────────────────────────────────────────────────┤
│  ICommand（扩展 IsVisible/IsChecked）/ CommandRegistry     │
│  （业务逻辑 + 状态权威）                                    │
└──────────────────────────────────────────────────────────┘
```

### 5.2 依赖方向

- Platform 层的 Action 模型**不依赖 Qt**，依赖 `CommandContext` / `ICommand`。
- Application 层的 `ActionUIBuilder` 负责把 Action 模型映射到 `QAction`。
- `CommandRegistry` 保持现状；`ICommand` 仅**新增两个默认实现的可选虚方法**，现有命令类零改动。

### 5.3 与现有系统的关系

| 现有概念 | Action 系统中的角色 |
|----------|---------------------|
| `ICommand` | Action 的执行载体 + 状态来源（`CanExecute` / `IsVisible` / `IsChecked`） |
| `CommandContext` | Action 执行、栈调度、状态求值时共享的上下文 |
| `CommandRegistry` | 命令工厂（Action 注册时通过它创建命令实例） |
| `CommandUIBuilder` | 职责收窄为"XML 解析 + QAction 挂载"，或拆分为 `ActionUIBuilder` |
| `config/ui_layout.xml` | 扩展 `<Actions>` 定义区 + 挂载区引用 ActionRef |

---

## 6. 核心接口契约

> 本节只定义接口契约（类、方法、语义），不涉及实现。

### 6.1 IAction（普通动作抽象）

| 成员 | 语义 |
|------|------|
| `GetId()` | 全局唯一 actionId，如 `act.undo` |
| `GetName()` / `GetTooltip()` / `GetIcon()` / `GetShortcut()` | UI 元数据 |
| `GetCommand()` | 被包装的命令引用（可空，Action 也可独立实现逻辑） |
| `Trigger(ctx)` | 触发：由 ActionManager 压栈后调用，内部委托命令 `Execute`（或独立逻辑） |
| `OnSubActionFinished(ctx, subActionId, result)` | **父 Action 回调**：子 Action 出栈时框架调用，传入子 Action 的结果数据 |
| `IsEnabled(ctx)` / `IsVisible(ctx)` / `IsChecked(ctx)` | 状态读取，**转发给命令**（无命令时默认 true / true / false） |
| `AttachSink(sink)` / `DetachSink(sink)` | 状态变化监听 |

### 6.2 ICommand 扩展（状态权威）

| 成员 | 语义 | 默认 |
|------|------|------|
| `CanExecute(ctx)` | `enabled` 来源 | `true`（现有） |
| `IsVisible(ctx)` | `visible` 来源（预留权限/模式） | `true`（新增，默认实现） |
| `IsChecked(ctx)` | `checked` 来源（模式切换类命令，如"显示网格"） | `false`（新增，默认实现） |

> 兼容性：新增方法均为带默认实现的虚方法，现有 8 个命令**无需任何修改**。

### 6.3 ActionStack（父子执行栈）

| 成员 | 语义 |
|------|------|
| `Push(shared_ptr<IAction>)` | 子 Action 入栈；框架随即调用新栈顶的 `Trigger` |
| `Pop()` | 栈顶 Action 结束出栈；把其结果数据经框架回传给新的栈顶父 Action |
| `Peek()` | 当前最上层 Action（可为空 = 栈空） |
| `IsEmpty()` | 栈是否为空 |
| `Clear()` | 清空栈（取消整个交互流程） |

### 6.4 结果数据（子 Action → 父 Action）

| 成员 | 语义 |
|------|------|
| `subActionId` | 产生结果的子 Action 的 id |
| `data` | 结果数据，载体复用现有 `AnyValue`（字符串/数值/Point3d），复杂结构可序列化为字符串 |

### 6.5 ActionManager（注册表 + 调度 + 刷新）

| 成员 | 语义 |
|------|------|
| `Register(shared_ptr<IAction>)` | 注册 Action（同时创建其命令实例） |
| `Find(id)` | 按 id 查 Action |
| `PushAction(id, ctx)` | 按 id 压栈并自动调度栈顶 |
| `PopAction(ctx)` | 出栈并回传结果（内部调用父的 `OnSubActionFinished`） |
| `RefreshAll(ctx)` | 遍历所有 Action，向命令求值状态并通知 sink |
| `GetStack()` | 访问执行栈（调试/测试） |

---

## 7. Command 状态模型与刷新机制

### 7.1 状态归属

**`enabled` / `visible` / `checked` 属于 Command，不属于 Action。**

| 状态 | 含义 | 求值输入 |
|------|------|----------|
| `enabled` | 可触发 | 命令 `CanExecute(ctx)` |
| `visible` | 可见 | 命令 `IsVisible(ctx)`（第一版默认 true） |
| `checked` | 勾选态 | 命令 `IsChecked(ctx)`（第一版预留，默认 false） |

### 7.2 典型映射（第一版即实现）

| Action | enabled 条件（命令 `CanExecute`） |
|--------|----------------------------------|
| `act.undo` | `TransactionManager::CanUndo()` |
| `act.redo` | `TransactionManager::CanRedo()` |
| `act.delete_selected` | `Document::GetSelected() != 0` |
| `act.file_save` / `act.file_load` | `doc != nullptr`（恒可用） |
| `act.add_*` | 恒可用 |

> 说明：这些条件目前就在各命令的 `CanExecute` 中实现（如 [`UndoCommand`](../src/Application/Commands/UndoCommand.cpp)），Action 层只需在刷新时读取。

### 7.3 刷新触发点

在以下时机调用 `ActionManager::RefreshAll(ctx)`：

1. 选中对象变化（3D 拾取、树视图选中）
2. 对象增删（命令执行后）
3. Undo / Redo 后（事务栈变化）
4. 文档 Load / New 后
5. 任意 Action 执行完成 / 出栈后（栈顶变化触发一次统一刷新）

> 说明：采用"事件驱动 + 全量求值"的简单策略；动作数量在百级以内，全量求值成本可忽略。

### 7.4 刷新流程

```
事件（选中变化/事务栈变化/命令执行后/栈顶变化）
  → MainWindow 调用 ActionManager::RefreshAll(ctx)
    → 对每个 Action 读取命令状态：
        enabled = cmd.CanExecute(ctx)
        visible = cmd.IsVisible(ctx)
        checked = cmd.IsChecked(ctx)
      → 状态变化则调用 sink.OnActionStateChanged(actionId)
        → ActionUIBuilder 更新对应 QAction 的 enabled/visible/checked
```

---

## 8. 父子 Action 栈设计

### 8.1 栈模型（取代"组"概念）

```
ActionStack（框架维护）
┌─────────────┐  栈顶 = 当前最上层 Action，框架自动调用
│ act.input   │ ← 子 Action（普通 Action，正在执行）
├─────────────┤
│ act.create  │ ← 父 Action（挂起，等待子结果）
├─────────────┤
│ (栈底)       │
└─────────────┘
```

- **父**：需要子流程时调用 `PushAction(subActionId)`，自身挂起。
- **框架**：自动调用新栈顶（子）Action 的 `Trigger`。
- **子**：执行完毕，设置结果数据；框架 `Pop()` 并把结果经 `OnSubActionFinished` 回传给父。
- **父**：收到结果后继续（可能再压子 Action，或完成自己后出栈）。

### 8.2 子 Action 的语义

- 子 Action **就是普通 Action**，与菜单/工具栏里的 Action 是同一个抽象、同一套接口。
- 它唯一的"特殊性"来自**栈中的位置**：
  - 在栈顶时：被框架自动调用；
  - 出栈时：结果数据由**框架**负责传给父 Action（父无需自己查找子 Action）。
- 子 Action 与父 Action 之间**不直接互相引用**，只通过框架栈与结果数据通信，从而最大化复用（同一 Action 可同时作为菜单项和某流程的子步骤）。

### 8.3 数据回传时序

```
父 Action 执行中需要用户输入
  → 父调用 ActionManager.PushAction(act.input)
    → 框架压栈 act.input，自动调用其 Trigger
      → act.input 执行，用户完成输入，设置结果 data="sphere"
        → 框架 Pop(act.input)，取结果
          → 框架调用父的 OnSubActionFinished(ctx, "act.input", data)
            → 父收到 data，继续后续逻辑（再压子 Action 或完成）
```

### 8.4 栈的生命周期

| 事件 | 行为 |
|------|------|
| 用户点击普通 Action | `Push` 自己 → 执行 → 若无子流程则立即 `Pop` 结束 |
| 父请求子 Action | 父挂起，子入栈，框架调度子 |
| 子完成 | 出栈，结果回传父，父继续 |
| 用户取消（ESC） | `Pop` 栈顶（丢弃结果），父收到空结果视为取消 |
| 整个流程结束 | 栈空，刷新所有 Action 状态 |

### 8.5 多级嵌套

栈天然支持多级嵌套（向导式流程）：

```
act.create_primitive（父）
  └─ act.pick_type      （选类型：sphere）
       └─ act.input_param（输入半径：50）→ 结果回传 pick_type → 再回传 create_primitive → 创建对象
```

建议交互层级 ≤ 3 级（可读性），框架不限制。

---

## 9. Action 复用机制

### 9.1 多挂载点共享（同一实例）

同一 Action 可同时挂载到菜单与工具栏：

- 菜单 `<ActionRef id="act.undo">` 与工具栏 `<ActionRef id="act.undo">` 引用**同一个** `IAction` 实例。
- 两处 QAction 的 enabled/checked 由同一个 sink 同步，**永远一致**。

### 9.2 状态与执行逻辑复用（Action 包装 Command）

- Action 不复制业务逻辑与状态判断，全部委托给命令。
- 同一命令实例被 Action 长期持有（注册时创建一次），`checked` 等有状态信息不会丢失。

### 9.3 参数化 Action（同类动作复用同一类）

以"添加图元"为例：

- 定义**一个** Action 类（如 `AddPrimitiveAction`），构造参数为图元类型（sphere / box / cylinder）。
- 注册三个实例：`act.add_sphere`、`act.add_box`、`act.add_cylinder`，仅参数不同。
- 新增图元类型时只需注册一行参数，无需新增类。

### 9.4 子流程复用（栈）

- 同一个"输入对话框 / 选择器"类子 Action 可被**多个父 Action 复用**：父 Action 只关心收到的结果数据，不关心子 Action 内部实现。
- 子 Action 不感知"我是谁的子步骤"，因此可同时作为独立菜单项使用。

### 9.5 复用约束

- 同一 Action 实例被多个挂载点共享时，快捷键只允许定义一次（XML 校验告警，见 13.3）。
- 参数化 Action 的参数在注册时固化，运行期不允许修改（保证 id → 行为一一对应）。

---

## 10. XML 配置扩展

### 10.1 设计原则

- **无 Group 概念**：XML 中只有普通 `<Action>` 定义，父子关系由运行时栈建立。
- 定义区（`<Actions>`，一次定义）与挂载区（`<Menu>/<ToolBar>`，多次引用）分离。
- 现有 `<Item command="cmd.xxx">` 保持兼容（作为隐式 Action 的快捷写法，见 12.2）。

### 10.2 配置结构草案

```xml
<?xml version="1.0" encoding="UTF-8"?>
<UILayout>
    <!-- 定义区：普通 Action（无父子层级概念） -->
    <Actions>
        <Action id="act.file_save" name="Save" command="cmd.file_save" icon="save.png" shortcut="Ctrl+S" />
        <Action id="act.file_load" name="Load" command="cmd.file_load" icon="open.png" shortcut="Ctrl+O" />

        <Action id="act.undo" name="Undo" command="cmd.undo" icon="undo.png" shortcut="Ctrl+Z" />
        <Action id="act.redo" name="Redo" command="cmd.redo" icon="redo.png" shortcut="Ctrl+Y" />

        <!-- 参数化叶子：同一类，不同参数 -->
        <Action id="act.add_sphere"   name="Sphere"   type="add_primitive" param="sphere" />
        <Action id="act.add_box"      name="Box"      type="add_primitive" param="box" />
        <Action id="act.add_cylinder" name="Cylinder" type="add_primitive" param="cylinder" />

        <!-- 独立逻辑 Action（无命令包装） -->
        <Action id="act.delete_selected" name="Delete" type="delete_selected" icon="del.png" shortcut="Delete" />

        <!-- 可选：静态子菜单展示（仅展示，不承担执行层级；执行父子关系由栈动态建立） -->
        <Action id="act.add_primitive" name="Add Primitive">
            <ActionRef id="act.add_sphere" />
            <ActionRef id="act.add_box" />
            <ActionRef id="act.add_cylinder" />
        </Action>
    </Actions>

    <!-- 挂载区：引用定义区（同一 Action 可被多次引用） -->
    <MenuBar>
        <Menu name="File">
            <ActionRef id="act.file_save" />
            <ActionRef id="act.file_load" />
        </Menu>
        <Menu name="Edit">
            <ActionRef id="act.undo" />
            <ActionRef id="act.redo" />
            <Separator/>
            <ActionRef id="act.add_primitive" />   <!-- 展示为子菜单，点击子项 = 直接执行对应 Action -->
            <Separator/>
            <ActionRef id="act.delete_selected" />
        </Menu>
    </MenuBar>

    <ToolBars>
        <ToolBar name="Main">
            <ActionRef id="act.file_save" />
            <ActionRef id="act.file_load" />
            <Separator/>
            <ActionRef id="act.undo" />
            <ActionRef id="act.redo" />
            <Separator/>
            <ActionRef id="act.add_primitive" />   <!-- 工具栏下拉 -->
            <Separator/>
            <ActionRef id="act.delete_selected" />
        </ToolBar>
    </ToolBars>
</UILayout>
```

### 10.3 关键语义

- **静态子菜单（10.2 中 `act.add_primitive` 的子项列表）只负责"展示"**：点击某个子项 = 直接触发该子 Action（走普通触发路径）。
- **执行时的父子关系与静态列表无关**：由 Action 内部在运行期通过 `PushAction` 动态建立（如向导流程）。两者是两种独立能力，可同时存在、互不影响。
- 若某 Action 不需要子菜单展示，省略其子项列表即可，它仍可作为流程中的父 Action。

### 10.4 挂载语义

| 挂载对象 | 菜单 | 工具栏 |
|----------|------|--------|
| 普通 Action（无子项列表） | 菜单项 | 工具按钮 |
| 带子项列表的 Action | 子菜单（QMenu） | 下拉按钮（QToolButton + menu） |
| `Separator` | 分隔线 | 分隔线 |

### 10.5 校验规则

- Action id 全局唯一，重复定义报错。
- `ActionRef` 必须引用已定义的 Action，未找到报错。
- `type="add_primitive"` 等参数化类型必须提供 `param`。
- 子菜单展示层数 ≤ 3 级（警告级别）。

---

## 11. 交互流程

### 11.1 普通触发流程

```
用户点击 QAction
  → ActionUIBuilder 的 lambda 调用 Action.Trigger(ctx)
    → ActionManager.PushAction(actionId)   （入栈）
      → 框架调用栈顶 Action 的命令 Execute
        → 执行完成，无子流程 → 框架 Pop 出栈
          → OnActionExecuted → RefreshAll(ctx) → 挂载点状态同步
```

### 11.2 父子流程（栈调度 + 数据回传）

```
用户点击"创建参数化图元"（act.create_primitive）
  → Push(act.create_primitive) → 框架调用其命令 Execute
    → 命令需要用户选择类型 → 调用 PushAction(act.pick_type)
      → Push(act.pick_type) → 框架自动调用栈顶 act.pick_type
        → 用户选择"sphere"，命令完成，结果 data="sphere"
          → 框架 Pop(act.pick_type)，调用父 OnSubActionFinished(ctx, "act.pick_type", "sphere")
            → 父命令继续：需要输入半径 → 再 PushAction(act.input_param)
              → ... 半径 50 回传 ...
                → 父创建 SphereObject，执行完成
                  → 框架 Pop(act.create_primitive)，栈空，流程结束
                    → RefreshAll(ctx)
```

### 11.3 状态刷新流程（非触发场景）

```
用户切换选中 / Undo / Redo / Load
  → MainWindow 调用 ActionManager::RefreshAll(ctx)
    → 读取每个 Action 绑定命令的状态（CanExecute/IsVisible/IsChecked）
      → 状态变化 → sink 通知 → QAction 更新
```

---

## 12. 与现有系统的迁移路径

### 12.1 迁移策略（渐进式，不推倒重来）

1. **阶段 A：扩展 ICommand 状态接口** —— 新增 `IsVisible` / `IsChecked` 默认实现虚方法（现有命令零改动），并新增 Platform 层 `IAction` / `ActionStack` / `ActionManager` 纯 C++ 模型。
2. **阶段 B：ActionUIBuilder 解析新格式** —— 支持 `<Actions>` 定义区 + `<ActionRef>` 挂载；保留对旧 `<Item command=>` 的兼容解析。
3. **阶段 C：状态刷新接入** —— MainWindow 在选中/事务/命令执行后调用 `RefreshAll()`，QAction 状态实时化。
4. **阶段 D：命令接入 Action** —— 现有命令按 7.2 映射注册为 Action（undo/redo/delete/add_*/save/load），XML 切换新格式，旧 `Item command` 写法下线。

### 12.2 兼容模式（旧 XML 写法）

旧写法 `<Item command="cmd.undo" shortcut="Ctrl+Z" />` 在阶段 B/C 期间**继续有效**：

- 解析器为其生成"隐式 Action"（id 沿用 command id，元数据取 XML 属性）。
- 隐式 Action 的 enabled = 命令 `CanExecute`，visible/checked 取命令默认值。
- 迁移完成后删除兼容分支。

### 12.3 CommandUIBuilder 的演变

- 现状：XML 解析 + QAction 创建 + lambda 绑定。
- 目标：职责拆为——`ActionUIBuilder`（XML → Action + QAction 挂载）与 `ActionManager`（栈调度 + 状态维护）。
- `CommandUIBuilder` 保留兼容壳或在阶段 D 移除。

---

## 13. 边界情况与注意事项

### 13.1 快捷键冲突

- 同一 Action 被多处挂载时，快捷键只认定义区声明的一次。
- 两个不同 Action 声明相同快捷键：加载时报错或按挂载顺序覆盖（建议报错）。

### 13.2 生命周期

- Action 实例由 ActionManager 持有（`shared_ptr`），QAction 只持有弱引用/观察者，避免循环引用。
- 栈内挂起的 Action 由 ActionManager 持有；`Clear()` 或窗口销毁时安全释放。

### 13.3 刷新代价

- 全量求值只遍历 Action 列表并调用命令状态方法，纯内存操作；第一版不做增量依赖分析。
- 若未来 Action 数量激增或含重计算条件，再引入"按需刷新"。

### 13.4 线程

- 状态刷新、QAction 更新、栈调度均在 UI 线程（Qt 主线程）完成。
- 命令执行保持现状同步执行；长任务留待后续异步化（栈调度需保证同步语义）。

### 13.5 栈的正确性

- 只有栈顶 Action 能触发新的 `PushAction` / `Pop`；非栈顶 Action 收到结果时视为异常（断言/日志）。
- `Pop` 时若父 Action 已被销毁（异常场景），丢弃结果并继续出栈，保证栈一致性。
- 结果数据为空字符串视为"用户取消"，父 Action 应据此终止流程。

### 13.6 与现有命令的关系约束

- Action 包装命令时**不得修改命令类**（组合优先；命令只需提供状态接口默认实现）。
- 若某动作无对应命令（如模式切换），用独立 Action 类型承载，不强行造命令。

---

## 14. 验收标准

### 14.1 功能验收

| # | 场景 | 预期 |
|---|------|------|
| 1 | 文档为空 / 无可撤销 | `act.undo` / `act.redo` 禁用（灰显） |
| 2 | 执行一次操作后 | `act.undo` 启用；执行 Undo 后 `act.redo` 启用 |
| 3 | 无选中对象 | `act.delete_selected` 禁用；选中后启用 |
| 4 | 菜单与工具栏同一 Action | 两处状态**始终一致**（enabled 同步） |
| 5 | 多步流程（选择类型 → 输入参数 → 创建） | 栈自动调度：子 Action 完成后数据正确回传父 Action，对象创建成功 |
| 6 | 流程中途取消（ESC） | 栈顶出栈，父 Action 收到空结果并终止流程，无副作用 |
| 7 | Load/New 后 | 所有 Action 状态按新文档重新求值 |

### 14.2 非功能验收

- 新增一个图元类型：仅 XML 加一行 + 参数化 Action 工厂支持，无新增 C++ 类。
- 旧 `Item command` XML 在兼容期内可正常构建 UI。
- 全量刷新在 100 个 Action 下耗时 < 1ms（可忽略）。
- 现有 8 个命令零修改即可接入（仅靠默认接口实现）。

---

## 15. 文件清单

> 仅为预期变更，具体以实施为准。

| 文件 | 类型 | 说明 |
|------|------|------|
| `src/Platform/Public/ICommand.h` | 修改 | 新增 `IsVisible` / `IsChecked` 默认实现虚方法 |
| `src/Platform/Public/IAction.h` | 新增 | Action 接口（状态转发 + 子流程回调） |
| `src/Platform/Public/ActionStack.h` | 新增 | 父子执行栈（Push/Pop/Peek/数据回传） |
| `src/Platform/Public/ActionManager.h` | 新增 | 注册表 + 栈调度 + 刷新中枢 |
| `src/Platform/Private/ActionStack.cpp` / `ActionManager.cpp` | 新增 | 实现 |
| `src/Platform/CMakeLists.txt` | 修改 | 增加新源文件 |
| `src/Application/UI/ActionUIBuilder.h/.cpp` | 新增 | XML 解析 + QAction 挂载 + 状态同步 |
| `src/Application/Actions/AddPrimitiveAction.h/.cpp` | 新增 | 参数化 Action 示例 |
| `src/Application/Actions/DeleteSelectedAction.h/.cpp` | 新增 | 独立逻辑 Action 示例 |
| `src/Application/Actions/CreatePrimitiveFlowAction.h/.cpp` | 新增 | 多步流程（栈父子）示例 |
| `src/Application/UI/MainWindow.h/.cpp` | 修改 | 持有 ActionManager，接入刷新触发点与栈操作 |
| `src/Application/UI/CommandUIBuilder.h/.cpp` | 修改 | 兼容壳或拆分 |
| `config/ui_layout.xml` | 修改 | 切换为新格式（定义区 + 挂载区） |
| `docs/action-system-design.md` | 本文档 | 设计基线 |

---

## 16. 待决策问题

以下问题需要实现前确认：

1. **Action 模型放 Platform 还是 Application？** 本文推荐 Platform（纯 C++，便于测试与复用）；备选：直接放 Application（少一层抽象，但状态模型与 Qt 耦合）。
2. **父 Action 如何声明子 Action？** 方案 A：命令内直接调用 `PushAction(subActionId)`（推荐，父子关系完全运行期动态）；方案 B：XML 中静态声明子项列表由框架自动推进（减少命令内联逻辑，但灵活性低）。
3. **结果数据载体**：直接复用 `AnyValue`（简单，第一版推荐）；还是引入结构化结果对象（可携带多字段，稍重）。
4. **checked 状态第一版是否启用**：默认预留不实现，还是为"显示网格 / 线框"类模式命令启用？
5. **兼容期长度**：旧 `Item command` 写法何时下线（建议新格式稳定两个迭代后）。
