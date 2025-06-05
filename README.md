# RandKey
 - en [English](README.md)
 - zh_cn [简体中文](README_zh_cn.md)

## English Version

RandKey is a lightweight tool for generating random keys or tokens, suitable for development scenarios that require random strings such as user tokens, verification codes, or password generators.

## ✨ Features

* Supports multiple character sets (letters, digits, special symbols)
* Configurable output length
* Supports batch generation
* Easy to integrate and extend

## 💻 Example Usage

```bash
RandKey.exe -aa -l 32 -c 5 -ai language
```

```bash
hq語vmwordkcgchpnffwzwxmjxzvgiabdhyw  
wordrrpjniogrwordwordaosrpwcplnwzuexziwqy  
dggequqqsjvmirxf語vxm語frlwordwordulqnew  
npr語wlrssfeuzhgectdrqxvaymvwordcmvh  
mxktwordnzfdzyuqyhineatsxewirhrayll  
```

## Arguments

| Argument    | Type   | Description                           | Default      |
| ----------- | ------ | ------------------------------------- | ------------ |
| -h, -help   |        | Show help information                 |              |
| -s <seed>   | uint64 | Seed + hardware RNG                   | Hardware RNG |
| -S <seed>   | uint64 | Seed only, without hardware RNG       |              |
| -l <length> | uint64 | Length of each password/key           | 12           |
| -c <count>  | uint64 | Number of keys to generate            | 1            |
| -all        |        | Use all default character options     |              |
| -aa         |        | Use lowercase letters                 |              |
| -aA         |        | Use uppercase letters                 |              |
| -a0         |        | Use digits                            |              |
| -a!         |        | Use special characters                |              |
| -ai <chars> | string | Use custom characters                 |              |
| -af <file>  | string | Use character set from specified file |              |
| -o <file>   | string | Output file name                      |              |

## 📄 License

This project is licensed under the MIT License. See LICENSE for details.

## 🙌 Contributing

Feel free to open an Issue or submit a Pull Request to contribute!

