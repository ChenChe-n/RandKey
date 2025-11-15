# RandKey 2.0

RandKey 是一个跨平台的安全随机密钥生成工具，提供命令行与可复用核心库两种形态，适用于访问令牌、一次性密码、测试数据批量生成等场景。

- 默认启用密码学安全随机源（Windows 使用 `BCryptGenRandom`，POSIX 使用 `/dev/urandom`）。
- 支持字符集组合、自定义字符文件、指定长度与批量数量。
- 支持确定性种子与硬件熵混合种子两种模式，可用于重现结果或加强不可预测性。
- 提供模块化 C++ API，便于集成到其他应用。

> 如果你偏好中文说明，可参阅 [`README_zh_cn.md`](README_zh_cn.md)。

## 目录结构

```
├── CMakeLists.txt           # CMake 构建脚本
├── include/
│   └── randkey/             # 对外暴露的库头文件
├── src/                     # 各功能模块实现
├── tests/                   # 单元测试与 CLI 集成测试
├── .github/workflows/ci.yml # GitHub Actions 流水线
└── todo.md                  # 体检后待办列表
```

核心模块职责：

- `randkey/random_engine.hpp`：跨平台安全随机数抽象。
- `randkey/options.hpp`：命令行参数解析与配置对象。
- `randkey/charset_registry.hpp`：字符集组合与文件加载。
- `randkey/generator.hpp`：密钥生成器，支持可选种子回传。
- `randkey/platform/*`：系统语言探测与本地编码 ↔ UTF-8/UTF-32 转换。
- `randkey/i18n/*`：帮助信息与错误提示的本地化。

## 构建与测试

项目使用 CMake + C++20：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRANDKEY_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

在 Windows 平台需要可用的 MSVC/MinGW 或者 clang toolchain，并确保链接 `bcrypt` 库。

## CLI 用法

```
randkey [options]

Options:
  -h, --help            显示帮助
      --version         显示版本号
  -S, --seed-only <n>   使用纯确定性种子（禁用硬件熵）
  -s, --seed <n>        指定确定性种子并与硬件熵混合
  -l, --length <n>      每个密钥长度（默认 12）
  -c, --count <n>       生成的密钥数量（默认 1）
  -all, --all           加入内置的所有字符集
  -aa, --lower          加入小写字母
  -aA, --upper          加入大写字母
  -a0, --digits         加入数字
  -a!, --special        加入特殊符号
  -ai, --append <chars> 追加自定义字符
  -af, --append-file <file> 从文件读取字符集（UTF-8）
  -o, --output <file>   输出到文件（默认 STDOUT）
      --force           允许覆盖已存在的输出文件
      --show-seed       输出实际使用的种子信息
```

示例：

```bash
# 生成 5 个长度 32 的随机密钥，包含大小写、数字与自定义汉字
randkey --all --length 32 --count 5 --append 語言

# 使用确定性种子复现结果
randkey --seed-only 123456 --length 16 --count 3

# 输出到文件并展示种子
randkey --seed 42 --length 24 --count 10 --output result.txt --force --show-seed
```

## 安全注意事项

- 默认使用系统提供的密码学随机源，若随机源不可用会报错退出。
- `--seed-only` 会关闭硬件熵，仅适合调试或需要完全可复现的场景；生产中建议使用 `--seed` 或默认模式。
-- 默认兼容平台本地编码（Windows 代码页、Linux/macOS locale），也建议优先使用 UTF-8 以获得最佳兼容性。
- 输出文件包含敏感密钥时请妥善保管，避免纳入版本控制。

## CI/CD

仓库内含 GitHub Actions workflow：在 Windows 与 Ubuntu 运行编译与测试，确保跨平台可靠性。

## 许可证

MIT License，详见 [LICENSE](LICENSE)。

