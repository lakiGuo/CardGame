# Qt5 项目调试总结

## 项目概述

本项目是一个使用 C++ 和 Qt5 开发的自定义知识卡牌游戏，包含卡牌数据模型、可视化卡牌组件、场景管理和图形界面。

## 开发环境

- **语言**: C++17
- **框架**: Qt5
- **构建工具**: CMake
- **编译器**: GCC 13.1.0
- **平台**: Linux

---

## 调试过程与问题解决

### 问题 1: Qt5 API 兼容性

#### 症状
编译错误，提示 `qDegreesToRadians` 和 `QTimer::singleShot` 函数不存在。

#### 原因分析
代码最初为 Qt6 编写，但系统只安装了 Qt5。Qt5 和 Qt6 之间存在 API 差异：

| 功能 | Qt6 | Qt5 |
|------|-----|-----|
| 角度转换 | `qDegreesToRadians()` | 需要手动实现 |
| 光标设置 | `setCursor(Qt::CursorShape)` | `setCursor(QCursor(Qt::CursorShape))` |
| QTimer singleShot | 支持 lambda + context | 只支持 lambda |

#### 解决方案

**1. 角度转换**
```cpp
// Qt6 写法（错误）
angle = qDegreesToRadians(angle);

// Qt5 写法（正确）
angle = angle * M_PI / 180.0;
```

**2. 光标设置**
```cpp
// 添加头文件
#include <QCursor>

// Qt6 写法（错误）
setCursor(Qt::OpenHandCursor);

// Qt5 写法（正确）
setCursor(QCursor(Qt::OpenHandCursor));
```

**3. QTimer 延迟调用**
```cpp
// Qt6 写法（错误）
QTimer::singleShot(i * 100, this, [widget, x, y]() {
    widget->animateTo(QPointF(x, y), 600);
}, Qt::UniqueConnection);

// Qt5 写法（正确）
QTimer::singleShot(i * 100, [widget, x, y]() {
    widget->animateTo(QPointF(x, y), 600);
});
```

**4. 容器选择**
```cpp
// 将 QVector 改为 std::vector 以避免 Qt5 的潜在问题
std::vector<Card> m_cardPool;
```

---

### 问题 2: 段错误 (Segmentation Fault)

#### 症状
程序启动后立即崩溃，输出 `Segmentation fault (core dumped)`。

#### 调试步骤

**步骤 1: 使用 GDB 获取调用栈**
```bash
gdb -batch -ex "run" -ex "bt" ./KnowledgeCardGame
```

输出结果：
```
#0  0x00005555555643f0 in QVector<Card>::operator=(QVector<Card> const&) ()
#1  0x0000555555564327 in CardTable::setCardPool(QVector<Card> const&) ()
#2  0x00005555555633ee in MainWindow::setupSampleCards() ()
#3  0x0000555555561a6b in MainWindow::MainWindow(QWidget*) ()
#4  0x000055555556185d in main ()
```

**分析**: 崩溃发生在 `QVector<Card>::operator=` 中。

**步骤 2: 创建单元测试隔离问题**

创建了三个测试程序：

1. **test_simple.cpp** - 测试 Card 类基本功能
2. **test_widget.cpp** - 测试 CardWidget 可视化组件
3. **test_card_vector.cpp** - 测试 std::vector 和 Card 的交互

test_card_vector.cpp:
```cpp
#include "../src/Card.h"
#include <iostream>
#include <vector>

int main()
{
    std::cout << "Test 1: Create Card\n";
    Card card1(1, "Test", "Content");
    std::cout << "  Card created OK\n";

    std::cout << "Test 2: Create std::vector<Card>\n";
    std::vector<Card> cards;
    std::cout << "  Empty vector created OK\n";

    std::cout << "Test 3: Reserve space\n";
    cards.reserve(10);
    std::cout << "  Reserve OK\n";

    std::cout << "Test 4: Push back card\n";
    cards.push_back(card1);
    std::cout << "  Push back OK\n";

    std::cout << "Test 5: Create vector with initializer list\n";
    std::vector<Card> cards2 = {
        Card(1, "A", "B"),
        Card(2, "C", "D")
    };
    std::cout << "  Initializer list OK, size=" << cards2.size() << "\n";

    std::cout << "Test 6: Copy vector\n";
    std::vector<Card> cards3 = cards2;
    std::cout << "  Copy OK\n";

    std::cout << "Test 7: Assign vector\n";
    cards = cards2;
    std::cout << "  Assign OK, size=" << cards.size() << "\n";

    std::cout << "\nAll tests passed!\n";
    return 0;
}
```

测试结果：全部通过！说明 Card 类本身没有问题。

**步骤 3: 重新检查调用栈**

单元测试通过后，问题必然出在主程序的初始化顺序上。再次使用 GDB：
```bash
gdb -batch -ex "start" -ex "step 10" -ex "bt" ./KnowledgeCardGame
```

发现调用链：`main()` → `MainWindow::MainWindow()` → `setupSampleCards()` → `CardTable::setCardPool()`

**步骤 4: 检查代码逻辑**

MainWindow.cpp:
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupSampleCards();  // ❌ 使用 m_table
    setupUI();            // ✅ 创建 m_table
    applyStylesheet();
}
```

**问题根源**: `setupSampleCards()` 在 `setupUI()` 之前调用，此时 `m_table` 尚未初始化，是空指针。

#### 解决方案

**方案 1: 调整调用顺序（推荐）**
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();           // 先创建 m_table
    setupSampleCards();  // 再使用 m_table
    applyStylesheet();
}
```

**方案 2: 在构造函数初始化列表中创建**
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_table(new CardTable(this))  // 先创建
{
    setupSampleCards();
    setupUI();
    applyStylesheet();
}
```

---

## 调试技巧总结

### 1. GDB 调试命令

| 命令 | 说明 |
|------|------|
| `gdb ./program` | 启动 GDB |
| `run` | 运行程序 |
| `bt` / `backtrace` | 查看调用栈 |
| `frame N` | 切换到第 N 层栈帧 |
| `print var` | 打印变量值 |
| `info locals` | 打印所有局部变量 |
| `step` / `s` | 单步执行（进入函数） |
| `next` / `n` | 单步执行（跳过函数） |
| `continue` / `c` | 继续执行 |
| `quit` / `q` | 退出 GDB |

**批量调试**:
```bash
# 自动运行并打印调用栈
gdb -batch -ex "run" -ex "bt" ./program

# 启动后执行多步
gdb -batch -ex "start" -ex "step 10" -ex "bt" ./program

# 打印所有线程的调用栈
gdb -batch -ex "run" -ex "thread apply all bt" ./program
```

### 2. 逐步测试法

**原则**: 从简单到复杂，逐层验证

```
1. 数据类 (Card) ✅
   ↓
2. 可视化组件 (CardWidget) ✅
   ↓
3. 场景管理 (CardTable) ✅
   ↓
4. 主窗口 (MainWindow) ✅
```

### 3. 常见段错误原因

| 原因 | 检查方法 |
|------|----------|
| 空指针解引用 | GDB 查看变量值 |
| 数组越界 | Valgrind 检测 |
| 野指针 | 使用智能指针 |
| 栈溢出 | 减少局部变量或改用堆分配 |
| 未初始化成员 | 检查构造函数初始化列表 |

### 4. CMake 构建调试

```bash
# 清理并重新构建
make clean && cmake .. && make

# 详细编译输出
make VERBOSE=1

# 检查链接库
ldd ./KnowledgeCardGame

# 检查符号
nm ./KnowledgeCardGame | grep Card
```

### 5. Qt 项目特定问题

**MOC 文件缺失**:
```cpp
// 头文件中必须有 Q_OBJECT 宏
class MyClass : public QObject {
    Q_OBJECT  // 必须有
public:
    ...
};
```

**信号槽连接错误**:
```cpp
// 检查信号槽返回值
bool ok = connect(sender, &Sender::signal,
                  receiver, &Receiver::slot);
if (!ok) {
    qDebug() << "Connect failed!";
}
```

**内存泄漏检测**:
```cpp
// 在 main 函数中
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // ... 程序逻辑 ...

    // 检测内存泄漏（Qt 调试模式）
    qDebug() << "Allocated objects:" << QObject::children().count();

    return app.exec();
}
```

---

## 项目结构

```
/home/pot/CardGame/
├── CMakeLists.txt          # 主项目构建文件
├── build/                  # 构建目录
├── src/
│   ├── main.cpp           # 程序入口
│   ├── MainWindow.h/cpp   # 主窗口
│   ├── Card.h/cpp         # 卡牌数据类
│   ├── CardWidget.h/cpp   # 卡牌可视化组件
│   └── CardTable.h/cpp    # 场景管理
└── test/                   # 单元测试
    ├── CMakeLists.txt
    ├── test_simple.cpp
    ├── test_widget.cpp
    └── test_card_vector.cpp
```

---

## 构建和运行

```bash
# 构建主项目
cd /home/pot/CardGame
mkdir -p build && cd build
cmake ..
make

# 运行程序
./KnowledgeCardGame

# 构建测试
cd ../test
mkdir -p build && cd build
cmake ..
make

# 运行测试
./test_simple
./test_card_vector
```

---

## 经验教训

### 1. 先写测试，后写功能
单元测试能快速定位问题，避免在复杂环境中调试。

### 2. 从简单开始
先实现最小可运行版本，再逐步添加功能。

### 3. 注意初始化顺序
C++ 成员变量按声明顺序初始化，不是按初始化列表顺序。

### 4. 使用现代 C++ 特性
- 使用 `std::vector` 而非 `QVector`（除非有特殊需求）
- 使用智能指针管理动态内存
- 使用 `auto` 简化类型声明

### 5. 查看错误信息
编译警告和 AutoMoc 警告也需要关注：
```
AutoMoc warning
---------------
"SRC:/src/CardWidget.cpp"
includes the moc file "CardWidget.moc", but does not contain a Q_OBJECT...
```

---

## 参考资源

- [Qt5 官方文档](https://doc.qt.io/qt-5/)
- [GDB 调试指南](https://www.gnu.org/software/gdb/documentation/)
- [CMake 官方文档](https://cmake.org/documentation/)
- [C++17 特性](https://en.cppreference.com/w/cpp/17)
