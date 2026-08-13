# 草图（Sketch）功能技术方案

> 版本：0.1（技术方案，尚未实现）
> 日期：2026-08-13
> 模块：Data / Geometry / Render / Platform / Application
> 关联文档：[Action 系统设计](action-system-design.md)、[事务系统设计](transaction-system-design.md)、[持久化系统设计](persistence-system-design.md)

---

## 目录

1. [概述与目标](#1-概述与目标)
2. [现状能力盘点](#2-现状能力盘点)
3. [总体设计](#3-总体设计)
4. [草图数据模型](#4-草图数据模型)
5. [显示与拾取](#5-显示与拾取)
6. [草绘交互流程（ActionStack 落地）](#6-草绘交互流程actionstack-落地)
7. [捕捉系统](#7-捕捉系统)
8. [约束系统（分阶段）](#8-约束系统分阶段)
9. [持久化](#9-持久化)
10. [实施路线](#10-实施路线)
11. [风险与注意事项](#11-风险与注意事项)
12. [待决策问题](#12-待决策问题)

---

## 1. 概述与目标

### 1.1 草图在 CAD 中的定位

草图（Sketch）是参数化建模的基础环节：用户在**草图平面**上绘制 2D 图元（直线、圆弧、圆、矩形等），通过约束控制形状，最终**挤出/旋转**生成 3D 实体。

本方案的目标是让项目具备完整的"草绘闭环"：

```
进入草图 → 绘制 2D 图元 → 捕捉/编辑 → 约束（后续） → 退出草图 → 挤出建模（后续）
```

### 1.2 与项目现状的契合点

本项目最近刚完成 **Action 系统（父子 Action 栈）**，而草绘天然是"多步交互流程"（进入草图模式 → 选择绘制工具 → 多次点击输入 → 退出），**草图是本项目第一个真正落地的 ActionStack 父子交互场景**，两者互为验证。

### 1.3 目标范围（MVP）

第一版（MVP）聚焦"能画、能存、能撤销"：

- 一个 `SketchObject` 对象类型（挂入 Document / 树视图 / 属性面板）
- 草图平面：世界坐标平面（XY/YZ/XZ 三选一，第一版简化）
- 2D 图元：直线、圆、圆弧、矩形
- 草绘模式交互：进入/绘制/退出（ActionStack 父子 Action）
- 基础捕捉：网格 + 端点/中点/圆心
- 线框显示 + 对象拾取
- Undo/Redo（每笔绘制一个事务）
- 持久化（保存/加载 .cad）

### 1.4 非目标（第一版不做）

- 约束系统（几何/尺寸约束）→ 阶段四
- 草图平面附着到已有实体面 → 后续
- 样条曲线（B-Spline）→ 后续
- 剪裁/延伸等高级编辑 → 后续
- 挤出/旋转建模 → 阶段五
- 通用数值约束求解器 → 后续

---

## 2. 现状能力盘点

| 现有能力 | 草图需求 | 差距与方案 |
|----------|----------|------------|
| `Document` / `IObject` / `Object` 体系 | `SketchObject` 对象类型 | 新增子类，复用宏注册（`CAD_OBJECT_BEGIN` + `CAD_PROP` + `MetaRegistry`） |
| `Property<T>` + `TypeMeta` 属性系统 | 2D 图元**集合**存储 | **差距**：属性系统只支持标量/`Point3d`，不支持集合 → 图元集合走对象内部成员 + 序列化扩展接口（见 §4.3/§9） |
| `Transaction` / `TransactionManager` | 每笔绘制独立 Undo/Redo | ✅ 直接复用；图元增删需要事务记录"对象内数据变更"（见 §6.3） |
| `ICommand` + `CommandRegistry` | 绘制/删除/退出命令 | ✅ 复用；新增 `SketchCommand` 族 |
| `IAction` + `ActionStack` + `ActionManager` | 草绘模式父子交互 | ✅ **本项目首个真实父子流程场景**（见 §6） |
| `ActionUIBuilder` + XML | 草图工具栏/菜单 | ✅ 扩展 `config/ui_layout.xml` |
| `OsgBackend::BuildNode` | 线框显示 | **差距**：当前只渲染 `GL_TRIANGLES` 三角面 → `GeometryData` 增加边/线数据 + 后端增加线框绘制（见 §5） |
| `OsgQtWidget::Pick` | 点选/捕捉 | **差距**：当前是对象级拾取（`ObjectId`）→ 草绘模式需要"屏幕射线 × 草图平面求交"得到 2D 点（见 §5.3） |
| `RenderSystem` + `GraphicsScene` | 草图节点同步 | ✅ 复用（`FullSyncFromDocument`） |
| `JsonSerializer` | 草图序列化 | **差距**：遍历 `TypeMeta` 属性，图元集合不在其中 → `IObject` 增加序列化扩展接口（见 §9） |

**结论**：基础框架（对象/事务/命令/Action 栈/渲染同步/持久化）全部就绪，草图的增量工作集中在四块——**数据模型、线框渲染、草绘交互、序列化扩展**。

---

## 3. 总体设计

### 3.1 分层职责

```
┌────────────────────────────────────────────────────────────┐
│ Application 层：草绘交互                                     │
│  SketchEditAction（父）/ SketchToolAction（子，栈回传）      │
│  SketchMouseHandler（捕捉 + 点击输入）                       │
│  SketchCommand（画线/画圆/画矩形/删除/退出）                 │
│  XML：Sketch 工具栏 + Edit 菜单                              │
├────────────────────────────────────────────────────────────┤
│ Render 层：线框显示 + 平面网格 + 图元拾取                     │
│  OsgBackend（线/点绘制路径）、Sketch 平面节点                 │
├────────────────────────────────────────────────────────────┤
│ Geometry 层：2D 图元几何运算                                  │
│  SketchGeometryUtils（求交/距离/投影/捕捉候选）              │
├────────────────────────────────────────────────────────────┤
│ Data 层：草图数据模型                                        │
│  SketchObject（IObject 子类）                               │
│  SketchPlane / SketchEntity（2D 图元 + id）                  │
│  JsonSerializer 扩展接口                                      │
└────────────────────────────────────────────────────────────┘
```

### 3.2 关键设计原则

1. **图元集合不进入 `Property<T>` 宏系统**：属性系统是"标量属性 + 变更通知"模型，不适合承载图元集合；图元数据作为 `SketchObject` 内部成员，通过**序列化扩展接口**接入持久化，通过**对象级脏标记**驱动刷新（复用 `DirtyFlags::Geometry`）。
2. **草绘交互用 ActionStack 表达**：`SketchEditAction`（父，进入草绘模式）→ 压栈 `SketchToolAction`（子，画线/画圆…）→ 子完成把图元数据回传父 → 父写入草图；退出即出栈。框架自动调度栈顶，天然支持"模式切换"。
3. **坐标统一**：绘制时把鼠标点投影到草图平面得到 **草图局部坐标**；图元在内部以草图局部坐标存储；显示时通过平面变换矩阵映射到世界坐标。保证"数据不依赖视角"。
4. **每笔绘制一个事务**：进入草图本身是 `SketchObject` 创建事务；每画一个图元是独立事务，Undo/Redo 粒度 = 单图元。

---

## 4. 草图数据模型

### 4.1 SketchObject（IObject 子类）

```
SketchObject
  ├─ objName / objId（继承自 Object，CAD_PROP_TRANSIENT 语义沿用）
  ├─ plane：草图平面定义（属性：planeKind + origin + normal，或简化为中心点 + 法向）
  ├─ entities：2D 图元集合（对象内部成员，不进 Property 宏系统）
  └─ constraints：约束列表（阶段四，同理内部成员）
```

宏声明（仅示意）：

```
CAD_OBJECT_BEGIN(SketchObject)
    CAD_PROP(Point3d, origin,  DirtyFlags::Geometry)   // 平面原点（世界坐标）
    CAD_PROP(Point3d, normal,  DirtyFlags::Geometry)   // 平面法向（世界坐标）
CAD_OBJECT_END
```

`buildShape()`：MVP 阶段返回空（草图本身不产出实体）；显示走线框路径（§5）。

### 4.2 2D 图元模型（SketchEntity）

| 类型 | 字段（草图局部坐标） |
|------|---------------------|
| `SketchPoint` | `pos` |
| `SketchLine` | `startPos` / `endPos` |
| `SketchCircle` | `center` / `radius` |
| `SketchArc` | `center` / `radius` / `startAngle` / `endAngle` |
| `SketchRect` | `minCorner` / `maxCorner`（或对角两点） |

结构设计：

```
SketchEntity
  ├─ entityId（ObjectId，图元级 id，供约束/删除/拾取引用）
  ├─ kind（枚举：Point/Line/Circle/Arc/Rect）
  └─ data（按 kind 的几何参数；用简单 struct 或变体存储）
```

- `entityId` 自增分配（类似 `Document` 的 `m_nextId` 机制，草图内独立计数）。
- 删除图元后 id 不复用（保证约束/引用稳定）。

### 4.3 为何不放进 Property 系统

- `Property<T>` 依赖 `AnyValueSupported<T>`，集合类型（`vector<SketchEntity>`）无法类型擦除，无法参与事务的 old/new 记录。
- 若强行包装会破坏属性宏系统的简洁性。
- **方案**：图元变更通过 `SketchObject` 的专用方法（`AddEntity` / `RemoveEntity` / `ModifyEntity`）修改内部集合，并：
  - 手动触发 `Document::OnObjectDirty(id, DirtyFlags::Geometry)`（通过 owner doc 指针）；
  - 事务记录由**草绘命令**负责：每笔绘制包一个 `BeginTransaction/Commit`，Undo 时整体撤销该图元的增删（沿用现有"对象内数据变更"的通用事务即可，无需为图元单独建事务机制——因为图元是 SketchObject 内部数据，撤销整笔绘制 = 恢复对象数据）。

> 说明：这里存在一个**事务粒度边界**——现有事务系统记录的是"对象级"操作（属性变更/对象增删）。图元是对象内部数据，MVP 方案采用"整笔绘制操作作为一次事务"，Undo 时重放对象快照或反向操作。详见 §6.3。

---

## 5. 显示与拾取

### 5.1 线框渲染（Render 层增量）

现状 `GeometryData`：

```
positions / normals / indices（三角面）
// optional: edges, bbox, version（注释预留）
```

方案：

- `GeometryData` 增加 `edges`（`vector<Point3d>`，按两两成对表示线段）与可选的 `points`。
- `OsgBackend::BuildNode` 扩展：当 `edges` 非空时，额外构建 `osg::Geometry` + `GL_LINES` 图元（或 `GL_LINE_STRIP` 分段），使用自定义颜色（草图线为区别于实体 mesh 的颜色）。
- `SketchObject` 的图形节点数据来自：把图元集合网格化为边（`SketchGeometryUtils::ToEdges`），而非三角面。

### 5.2 草图平面辅助显示（可选，MVP 可省）

- 平面网格：半透明 `osg::Geometry`（GridLines），辅助对齐。
- MVP 可先省略，用"捕捉网格"逻辑替代视觉网格。

### 5.3 拾取（草绘模式点选）

- 现有 `OsgQtWidget::Pick` 是对象级拾取（找 `ObjectId` userValue）。
- 草绘模式需要**平面点拾取**：鼠标屏幕坐标 → `LineSegmentIntersector` 得到射线 → 与草图平面求交 → 草图局部坐标。
- 实现位置：`OsgQtWidget` 增加平面求交回调（或在 Application 层用现有相机 + 手动射线-平面求交），**不侵入对象拾取逻辑**。
- 结果经捕捉系统修正后作为图元输入点。

---

## 6. 草绘交互流程（ActionStack 落地）

### 6.1 交互状态机

```
（普通模式）
  点击 "Sketch" 按钮（act.new_sketch）
    → PushAction(act.new_sketch)
      → 创建 SketchObject + 进入草绘模式（压栈 SketchEditAction）
        → 工具栏切换为草绘工具（画线/画圆/矩形/删除/退出）
        → 用户点击"画线" → PushAction(act.sketch_line)
          → 鼠标点击 2 次（起点/终点，捕捉生效）
          → 子 Action 完成，结果回传父（SketchLine 数据）
            → 父写入 SketchObject（一个事务）
        → 用户点击"退出" → 出栈回普通模式
```

### 6.2 Action 设计

| Action | 角色 | 职责 |
|--------|------|------|
| `act.new_sketch` | 根（父） | 创建 SketchObject（事务）→ 压栈 `act.sketch_edit` |
| `act.sketch_edit` | 父（草绘模式根） | 持有当前草图上下文（SketchObject 指针）；接收子 Action 结果并写入；维护"草绘中"状态 |
| `act.sketch_line` / `act.sketch_circle` / `act.sketch_rect` / `act.sketch_arc` | 子（工具） | 收集鼠标点击 → 生成图元数据 → 框架出栈回传 |
| `act.sketch_delete` | 子 | 删除选中图元 |
| `act.sketch_exit` | 子 | 结束草绘模式（出栈） |

**这正是 ActionStack 设计的首次实战**：父挂起、子执行、结果回传（`OnSubActionFinished` + `GetResult`），验证框架的栈调度与数据回传契约。

### 6.3 绘制与事务粒度

- 每画一个图元：`BeginTransaction()` → `SketchObject::AddEntity(...)`（触发对象脏标记）→ `Commit()`。
- Undo 该笔：需要恢复"图元被加入前"的草图状态。
  - **方案 A（推荐 MVP）**：`AddEntity` 通过现有属性通知链不可行（集合不在属性系统），改为在事务内用 `Document::ApplyPropertySilent` 的思路，但针对集合需要专用机制。
  - **更简单**：草绘命令在 `Execute` 时通过一个**草图编辑事务包装**——把"图元加入 + 几何重建"作为一次原子操作；Undo 语义 = 移除该图元（记录图元 id），Redo = 重新加入。可在 Platform 层新增轻量的 `SketchTransactionRecord`（记录 `entityId` + 增删类型 + 图元快照），挂在 `TransactionManager` 之外或作为其扩展。
  - **备选（最简）**：MVP 阶段绘制命令直接操作 `SketchObject` 并把图元写入；Undo 采用"整草图的轻量快照"（每次绘制前深拷贝 entities 到事务条目）。图元数量少时足够，实现最省。
- 结论：MVP 采用"每笔绘制 = 一次事务，事务条目携带图元快照/反向操作"，**不扩展通用事务系统**；若后续图元量大再演进。

### 6.4 草绘模式 UI

- 进入草绘后：`ActionUIBuilder` 刷新使绘制工具 Action 可用、普通建模工具按需禁用（`IsEnabled` 由命令状态提供，天然支持）。
- 退出草绘：恢复普通工具栏状态。

---

## 7. 捕捉系统

### 7.1 捕捉类型（MVP）

| 捕捉 | 说明 |
|------|------|
| 网格捕捉 | 草图局部坐标对齐网格步长（如 10mm） |
| 端点捕捉 | 已有直线/圆弧端点 |
| 中点捕捉 | 已有线段中点 |
| 圆心捕捉 | 已有圆/圆弧圆心 |

### 7.2 实现

- `SketchGeometryUtils` 提供：`NearestCandidate(sketch, point, tol)` 返回捕捉到的候选点与类型。
- 鼠标移动时实时计算（草绘模式高频调用，图元数少时开销可忽略）。
- 候选优先级：端点/中点/圆心 > 网格。
- 捕捉结果在 UI 上可选高亮（MVP 可仅显示坐标 tooltip）。

---

## 8. 约束系统（分阶段）

### 8.1 阶段四目标

- **几何约束**：水平、垂直、相切、共点、平行、垂直（图元两两之间）。
- **尺寸约束**（可选）：长度、半径、角度。
- 约束存储：`SketchObject::constraints`（类型 + 引用 entityId + 参数），与图元一样为内部成员 + 序列化扩展。

### 8.2 求解策略

- **MVP 约束不做通用求解器**：采用"规则式局部求解"（约束变更时按固定规则重算被约束图元的参数）。
- 全约束/过约束检测：只做简单标记（不求解），留待后续。
- 通用数值求解器（如几何约束求解器）列为**长期方向**，需单独评估（涉及图元参数化、冲突检测、迭代求解），不建议 MVP 引入。

### 8.3 约束后重算与事务

- 约束变更 → 重算受影响图元 → 触发对象脏标记 → 渲染刷新（复用现有链路）。
- 约束编辑同样每步一个事务（记录约束参数变更）。

---

## 9. 持久化

### 9.1 现状

`JsonSerializer` 遍历 `TypeMeta` 属性序列化；`SketchObject` 的图元集合不在属性系统内，不会被序列化。

### 9.2 扩展方案（对现有系统最小侵入）

- `IObject` 增加**可选扩展接口**（默认空实现，现有对象零改动）：

```
virtual void   SerializeExtra(nlohmann::json& out) const {}      // 写入附加数据
virtual bool   DeserializeExtra(const nlohmann::json& in)        // 读取附加数据
```

- `JsonSerializer::SerializeObject` / `DeserializeObject` 在属性序列化后调用这两个方法。
- `SketchObject` 覆写：把 `entities` / `constraints` 序列化为 `extra` 节点。

### 9.3 文件格式（示意）

```json
{
  "type": "SketchObject",
  "id": 5,
  "properties": {
    "objName": "Sketch_1",
    "origin": "0,0,0",
    "normal": "0,0,1"
  },
  "extra": {
    "entities": [
      { "id": 1, "kind": "Line", "start": "0,0", "end": "100,0" },
      { "id": 2, "kind": "Circle", "center": "50,0", "radius": "20" }
    ],
    "nextEntityId": 3
  }
}
```

### 9.4 加载

- `MetaRegistry::CreateByTypeName("SketchObject")` 创建（复用现有流程）→ `DeserializeExtra` 恢复图元 → `addWithId` 恢复对象 id。
- 加载后 `RenderSystem::FullSyncFromDocument` 全量刷新（线框路径重建）。

---

## 10. 实施路线

### 阶段一：数据模型 + 显示 + 持久化
- 新增 `SketchObject`（含 plane 属性 + entities 内部成员 + `AddEntity/RemoveEntity`）
- `GeometryData.edges` + `OsgBackend` 线框绘制
- `IObject` 序列化扩展接口 + `SketchObject` 覆写
- `ObjectFactory::CreateSketchObject` + `RegisterAllTypes` 注册
- **验收**：XML/代码创建一个含图元的草图对象 → 3D 视图显示线框 → 保存/加载 roundtrip 数据一致

### 阶段二：草绘交互（MVP 核心）
- `act.new_sketch` / `act.sketch_edit` / 工具子 Action（ActionStack 父子回传）
- 屏幕射线 × 草图平面求交 + 捕捉（网格/端点/中点/圆心）
- 绘制命令 + 每笔事务（Undo/Redo 粒度 = 单图元）
- XML 增加草图工具栏
- **验收**：进入草图 → 画线/圆/矩形 → 每笔可 Undo/Redo → 退出草图后对象留在文档

### 阶段三：图元编辑
- 图元选中（对象内 entityId 级拾取）、删除、移动端点
- 属性面板显示草图参数（经 `GetParameters` 扩展）
- **验收**：选中图元可删除/拖动端点，属性联动

### 阶段四：约束基础（几何约束）
- `SketchObject::constraints` + 规则式局部求解
- 约束 UI（选择两图元 → 应用约束）
- **验收**：水平/垂直/相切等约束生效且可撤销

### 阶段五（后续）：草图 → 实体建模
- 封闭轮廓检测 → `OCCT BRepBuilderAPI_MakeWire` + `BRepPrimAPI_MakePrism`（挤出）
- `SketchObject::buildShape()` 返回实体
- 新增 `ExtrudeCommand`

### 依赖关系

```
阶段一 ──► 阶段二 ──► 阶段三 ──► 阶段四
   │                                   │
   └────────────► 阶段五（后续）────────┘
```

阶段一/二可并行开发（数据模型与交互解耦）；阶段三依赖一；阶段四依赖三；阶段五依赖四。

---

## 11. 风险与注意事项

| # | 风险 | 缓解 |
|---|------|------|
| 1 | **属性系统不支持集合**，图元数据脱离属性/事务体系 | 明确的边界：图元走"对象内部成员 + 序列化扩展 + 专用编辑方法"，事务用"整笔绘制快照"；不强行扩展属性宏 |
| 2 | **图元级事务粒度**与现有对象级事务体系不匹配 | MVP 用图元快照/反向操作记录在命令事务中；不改造通用事务系统（文档化边界，后续再演进） |
| 3 | **渲染层无线框路径** | `GeometryData.edges` 增量扩展 + `OsgBackend` 增加 `GL_LINES` 绘制，不改现有三角面路径 |
| 4 | **拾取粒度**（对象级 vs 图元级） | 草绘模式走"射线×平面求交"独立路径，与对象拾取并存互不影响 |
| 5 | **坐标系转换**（草图局部 ↔ 世界） | 统一矩阵变换；图元内部只存局部坐标，杜绝"显示坐标污染数据" |
| 6 | **草图平面变换后编辑** | MVP 仅支持世界平面（XY/YZ/XZ），避免平面矩阵 + 捕捉的复合复杂度 |
| 7 | **高频刷新性能**（鼠标移动捕捉） | 图元数量级小时全量重建可接受；若超标再引入局部脏标记 |
| 8 | 编码一致性 | 新文件沿用 `/utf-8` 编译选项（已加），避免 GBK 误读问题 |

---

## 12. 待决策问题

1. **草图平面定义**：MVP 用"世界平面 + 法向"（简单）还是引入"基准面对象"（灵活但重）？建议 MVP 前者。
2. **图元存储**：`vector<SketchEntity>` + 线性查找（简单）还是 `unordered_map<entityId, SketchEntity>`（删除/引用 O(1)，推荐）？
3. **事务粒度方案**：图元快照（最简）vs 反向操作记录（需扩展事务）？建议 MVP 快照。
4. **是否显示草图平面网格**：MVP 显示半透明网格（辅助绘制）还是仅逻辑捕捉？建议 MVP 显示简单网格，成本低收益高。
5. **圆弧输入方式**：三点式（起点/终点/通过点）vs 圆心+半径+角度？建议 MVP 支持三点式最简单实现。
6. **矩形是独立图元还是 4 条直线组合**：独立图元（编辑/约束更友好，推荐）vs 组合（复用画线逻辑）。
7. **约束求解器**：MVP 规则式局部求解，何时评估通用求解器（建议约束场景出现后再评估）。
8. **草图对象是否参与现有"添加图元"下拉**：建议独立菜单项（`act.new_sketch`），不混入 primitive 组。

---

## 附：与 Action 系统的对接点（本次开发即验证）

- 父 Action `act.sketch_edit` 实现 `OnSubActionFinished` 接收图元数据 → 验证栈回传契约。
- 子 Action（绘制工具）通过 `GetResult()` 返回图元 → 验证结果载体（`AnyValue` 或扩展结构化结果）。
- 工具按钮可用性由命令 `CanExecute` 驱动（如"草绘模式中画线可用"）→ 验证状态刷新的模式感知。
- 该场景若暴露 ActionStack 契约缺陷（如结果载体、嵌套深度限制），优先回改 Action 设计而非绕开。
