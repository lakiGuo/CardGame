# Knowledge Card Game

![Qt](https://img.shields.io/badge/Qt-5.15%2B-41CD52?logo=qt)
![C++](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus)
![CMake](https://img.shields.io/badge/CMake-3.5%2B-064F8C?logo=cmake)
![Platform](https://img.shields.io/badge/Platform-Linux-E95420)

一个基于 Qt5 的知识卡牌游戏，帮助通过卡片联想的方式学习和复习知识点。

![Screenshot](screenshot.png)

## ✨ 特性

- 🎴 **多卡组管理** - 创建和管理多个主题卡组
- 🎲 **随机抽卡** - 每次随机抽取 N 张卡片进行学习
- 🔄 **自动迁移** - 自动从旧版本迁移数据
- 💾 **持久化存储** - 卡组数据保存在 `~/.knowledgecardgame/decks/`
- ✏️ **卡片编辑** - 双击卡片即可编辑内容，修改自动保存
- 🧮 **LaTeX 公式** - 支持 `$...$` 行内公式和 `$$...$$` 独立行公式渲染
- 🎨 **现代化 UI** - 暗色主题，流畅的动画效果
- 🖱️ **鼠标操作** - 支持拖拽、缩放等交互

## 📋 系统要求

- **操作系统**: Linux (已测试 Ubuntu 22.04)
- **编译器**: GCC 7.0+ 或 Clang 5.0+ (支持 C++17)
- **Qt**: 5.12 或更高版本
- **CMake**: 3.5 或更高版本

## 🚀 快速开始

### 安装依赖

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install qtbase5-dev qt5-qmake cmake build-essential
```

**LaTeX 公式渲染（可选）：**
```bash
sudo apt-get install texlive-latex-recommended texlive-latex-extra dvipng
```
> 未安装 LaTeX 时应用仍可正常编译和运行，公式将以原始文本显示。

**Fedora:**
```bash
sudo dnf install qt5-qtbase-devel cmake gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S qt5-base cmake gcc
```

### 编译项目

```bash
# 克隆仓库
git clone <repository-url>
cd CardGame

# 创建构建目录
mkdir build && cd build

# 配置和编译
cmake ..
make -j$(nproc)

# 运行
./KnowledgeCardGame
```

## 📖 使用说明

### 基本操作

1. **选择卡组**
   - 点击左侧面板的 "Select Deck" 按钮
   - 从列表中选择一个卡组
   - 或者点击 "Create New Deck" 创建新卡组

2. **开始游戏**
   - 点击 "Play" 按钮随机抽取卡片
   - 使用滑块调整抽取数量 (1-10 张)
   - 每次点击 Play 会清空桌面并重新发牌

3. **添加卡片**
   - 点击 "Add Card" 添加新卡片到当前卡组
   - 输入标题和内容
   - 点击 OK 保存

4. **编辑卡片**
   - 双击桌面上的任意卡片进行编辑
   - 修改内容后点击 Save 即可，更改会自动保存到磁盘

5. **清空桌面**
   - 点击 "Clear Table" 清除所有桌面上的卡片

### 卡组管理

- **保存卡组**: 点击 "Save Deck" 保存当前卡组的所有更改
- **卡组存储**: 所有卡组保存在 `~/.knowledgecardgame/decks/` 目录
- **文件格式**: JSON 格式，便于备份和迁移

### 鼠标操作

- **拖拽卡片**: 按住鼠标左键拖动卡片
- **缩放视图**: 使用鼠标滚轮缩放
- **平移视图**: 按住鼠标中键或使用拖拽模式

## 🏗️ 项目结构

```
CardGame/
├── src/                    # 源代码
│   ├── main.cpp           # 程序入口
│   ├── MainWindow.h/cpp   # 主窗口
│   ├── Deck.h/cpp         # 卡组数据模型
│   ├── DeckManager.h/cpp  # 卡组管理器
│   ├── DeckRepository.h/cpp # 文件存储
│   ├── Card.h/cpp         # 卡片数据模型
│   ├── CardManager.h/cpp  # 卡片管理器
│   ├── CardWidget.h/cpp   # 卡片UI组件
│   ├── CardTable.h/cpp    # 卡牌场景
│   ├── CardEditDialog.h/cpp # 编辑对话框
│   ├── LatexParser.h/cpp    # LaTeX 公式解析器
│   ├── LatexRenderer.h/cpp  # LaTeX 公式渲染器
│   └── DeckSelectionDialog.h/cpp # 卡组选择对话框
├── test/                   # 测试代码
│   ├── test_deck_system.cpp # 完整测试套件
│   └── ...
├── build/                  # 构建输出
├── CMakeLists.txt         # 构建配置
└── README.md              # 本文件
```

## 🧪 测试

项目包含完整的单元测试套件：

```bash
cd test/build
cmake ..
make test_deck_system
./test_deck_system
```

**测试覆盖:**
- ✅ Deck 类的创建和序列化
- ✅ DeckRepository 文件存储
- ✅ DeckManager 多卡组管理
- ✅ 数据迁移功能
- ✅ 完整工作流集成测试

## 🔧 开发计划

- [x] 增加latex支持
- [x] 修改弹出窗口字体的颜色
- [ ] 支持卡片标签和分类
- [ ] 添加卡片搜索功能
- [ ] 实现学习进度跟踪
- [ ] 支持导入/导出卡组
- [ ] 添加卡片模板
- [ ] 支持图片和富文本
- [ ] 添加撤销/重做功能

## 🛠️ 技术栈

| 组件 | 技术 |
|------|------|
| **GUI 框架** | Qt5 (Widgets) |
| **语言** | C++17 |
| **构建系统** | CMake |
| **测试框架** | Qt Test |
| **数据格式** | JSON |

## 📝 数据格式

卡组文件采用 JSON 格式存储：

```json
{
  "version": "2.0",
  "name": "C++ Basics",
  "description": "Core C++ programming concepts",
  "createdAt": "2026-03-01T10:00:00",
  "lastModified": "2026-03-01T15:30:00",
  "cards": [
    {
      "id": 1,
      "title": "RAII",
      "content": "Resource Acquisition Is Initialization",
      "created": "2026-03-01T10:05:00"
    }
  ]
}
```

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

## 🙏 致谢

- Qt Framework - 跨平台应用开发框架
- Qt 社区提供的文档和示例

## 📮 联系方式

如有问题或建议，欢迎通过以下方式联系：

- 📧 Email: guoyouj13@gmail.com
- 🐛 Issues: [GitHub Issues](https://github.com/your-username/CardGame/issues)

---

⭐ 如果这个项目对你有帮助，请给个 Star！
