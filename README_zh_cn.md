# RandKey

 - en [English](README.md)
 - zh_cn [简体中文](README_zh_cn.md)

RandKey 是一个用于生成随机密钥或令牌的轻量级工具，适用于开发中需要随机字符串的场景，比如用户令牌、验证码、密码生成器等。

## ✨ 特性

- 支持多种字符集（字母、数字、特殊字符）
- 可配置生成长度
- 支持批量生成
- 易于集成和扩展

## 💻 使用示例

```bash
RandKey.exe -aa -l 32 -c 5 -ai 语言
```

```bash
hq语vm言kcgchpnffwzwxmjxzvgiabdhyw  
言rrpjniogr言言aosrpwcplnwzuexziwqy  
dggequqqsjvmirxf语vxm语frl言言ulqnew  
npr语wlrssfeuzhgectdrqxvaymv言cmvh  
mxkt言nzfdzyuqyhineatsxewirhrayll  
```

## 参数说明

| 参数       | 类型   | 说明                               | 默认值     |
| ---------- | ------ | ---------------------------------- | ---------- |
| -h, -help  |        | 显示帮助信息                       |            |
| -s <种子>  | uint64 | 密码生成器的种子 + 硬件随机数      | 硬件随机数 |
| -S <种子>  | uint64 | 密码生成器的种子，不使用硬件随机数 |            |
| -l <长度>  | uint64 | 密码长度                           | 12         |
| -c <数量>  | uint64 | 密码数量                           | 1          |
| -all       |        | 使用全部的默认字符选项             |            |
| -aa        |        | 使用小写字母                       |            |
| -aA        |        | 使用大写字母                       |            |
| -a0        |        | 使用数字                           |            |
| -a!        |        | 使用特殊字符                       |            |
| -ai <字符> | string | 使用指定字符                       |            |
| -af <文件> | string | 从文件读取字符集                   |            |
| -o <文件>  | string | 输出文件名                         |            |

## 📄 License

该项目使用 MIT 许可证，详情见 LICENSE。

## 🙌 贡献指南

欢迎提交 Pull Request，或提出 Issue 反馈问题。
