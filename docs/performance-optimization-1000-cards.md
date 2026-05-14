# Browse 模式 1000 张卡片性能优化

## 问题现象

在 Browse 模式下平铺 1000 张卡片时，存在明显卡顿：
- 卡片"一组一组"飞入，约 30 秒才全部就位
- 加载过程中有奇怪的视觉闪烁效果

## 性能瓶颈分析

### 1. 1000 个独立动画叠加（主要原因）

`CardTable::layoutCardsInGrid` 为每张卡片创建了延迟飞入动画：

```cpp
// CardTable.cpp - 优化前
int delay = i * 30;  // 第 i 张卡片延迟 i*30ms
QTimer::singleShot(delay, [widget, targetX, targetY]() {
    widget->animateTo(QPointF(targetX, targetY), 400);
});
```

同时 `CardWidget` 构造函数中也有 scale 0→1 的弹出动画：

```cpp
// CardWidget.cpp - 优化前
setScale(0);
QPropertyAnimation *scaleAnim = new QPropertyAnimation(this, "scale", this);
scaleAnim->setStartValue(0.0);
scaleAnim->setEndValue(1.0);
scaleAnim->setDuration(300);
scaleAnim->start(QAbstractAnimation::DeleteWhenStopped);
```

1000 张卡片 × 30ms 间隔 = **30 秒**才能全部到位。1000 个并行的 `QPropertyAnimation` 对象本身也是巨大的 CPU 和内存负担。

### 2. QGraphicsDropShadowEffect（次要原因）

```cpp
// CardWidget.cpp - 优化前
auto *shadow = new QGraphicsDropShadowEffect(this);
shadow->setBlurRadius(20);
shadow->setColor(QColor(0, 0, 0, 100));
shadow->setOffset(QPointF(0, 4));
setGraphicsEffect(shadow);
```

Qt 的 `QGraphicsEffect` 使用 **CPU 软件渲染**，每个 shadow effect 需要额外的 off-screen buffer。1000 个 shadow effect 会严重消耗 CPU 和内存，并导致视觉闪烁。

### 3. FullViewportUpdate 加重渲染负担

```cpp
// CardTable.cpp - 优化前
setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
```

`FullViewportUpdate` 模式下，任何 item 的变化（如动画中间帧）都会触发整个视口重绘。1000 个卡片同时动画时，每一帧都是全视口重绘。

## 优化方案

### A. CardWidget 构造函数增加轻量模式参数

为 `CardWidget` 增加两个可选参数，Browse 模式下可以禁用特效和动画：

```cpp
// CardWidget.h
explicit CardWidget(const Card &card, QGraphicsItem *parent = nullptr,
                    bool enableEffects = true, bool enableAnimation = true);
```

```cpp
// CardWidget.cpp
CardWidget::CardWidget(const Card &card, QGraphicsItem *parent,
                       bool enableEffects, bool enableAnimation)
{
    // ...
    if (enableEffects) {
        auto *shadow = new QGraphicsDropShadowEffect(this);
        // ...
    }
    if (enableAnimation) {
        setScale(0);
        QPropertyAnimation *scaleAnim = new QPropertyAnimation(this, "scale", this);
        // ...
    }
}
```

默认值 `true` 保证 Play 模式和其他调用方式的行为不变。

### B. layoutCardsInGrid 直接放置卡片

取消所有延迟动画，直接将卡片放到目标位置：

```cpp
// CardTable.cpp - 优化后
for (int i = 0; i < n; ++i) {
    // enableEffects=false, enableAnimation=false
    auto *widget = new CardWidget(card, nullptr, false, false);
    addItem(widget);

    // 直接放置，不使用 animateTo
    widget->setPos(targetX, targetY);
    widget->setFlag(QGraphicsItem::ItemIsMovable, false);
    // ...
}
```

### C. MinimalViewportUpdate

```cpp
// CardTable.cpp - 优化后
setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
```

只重绘实际发生变化的区域，而非整个视口。

## 优化结果

| 操作 | 优化前 | 优化后 |
|------|--------|--------|
| Grid layout (1000 cards) | 3 ms + 30s 动画 | 0 ms，瞬间完成 |
| 视觉闪烁 | 有（shadow effect） | 无 |
| Browse 模式体验 | 卡顿、逐批加载 | 流畅 |
| Play 模式动画 | 正常 | 不受影响 |

## 涉及文件

- `src/CardWidget.h` — 构造函数声明
- `src/CardWidget.cpp` — 条件化 shadow effect 和弹出动画
- `src/CardTable.cpp` — layoutCardsInGrid 直接放置 + viewport update mode
