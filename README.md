# Hit Window Lab · 近战判定试验场

独立的 2D 近战判定原型。浏览器运行用 C++17 编译的 WebAssembly 判定核心，JavaScript 负责场景采样、绘制和显示结果。

## 打开演示

用 Chrome 或 Edge 打开 `HitWindowLab.html`。单文件内嵌判定模块，可离线运行。

- 点击「重放 A17 用例」，或者用「单步」逐帧查看结果。
- 点击「挥击」可自由试打。WASD 移动，空格挥击。通过防御选单切换 E04 的无敌、格挡、弹反。
- 点击「重置」恢复血量和判定记录。点击「打断」使当前有效攻击实例后续的接触失效。

| 逻辑帧 | 裁决 | E04 血量 | 累计扣血次数 |
|---|---|---:|---:|
| 103 | `OUTSIDE_WINDOW` | 100 | 0 |
| 104 | `INVULNERABLE` | 100 | 0 |
| 105 | `HIT` | 75 | 1 |
| 106 | `DUPLICATE` | 75 | 1 |
| 107 | `OUTSIDE_WINDOW` | 75 | 1 |

## 源码和复验

- `combat.hpp`、`combat.cpp` 包含攻击窗口、按实例和目标去重、防御裁决、打断及二维线段扫圆。
- `verify.mjs` 用 Node.js 执行同一份 `combat.wasm`，断言上述序列、同帧去重、双目标、格挡和弹反、过期回调、重新出招、快速扫过目标和非法输入。
- `verification.txt` 保存交付前实际执行 `node verify.mjs` 的输出和 WASM 文件的 SHA-256。
- `demo.template.html` 是演示页源码。`build.ps1` 编译判定模块并将其嵌入网页。

在解压目录运行以下命令复验现成二进制。

```powershell
node .\verify.mjs
```

从源码重建需要 Zig。交付构建所用版本为 `0.12.0-dev.1286+cc394431a`。

```powershell
.\build.ps1 -Compiler zig
node .\verify.mjs
```

本原型的范围是单个攻击方、一个活动攻击窗口、两名固定训练目标。防御快照由演示提供，规则层在有效命中时更新试验场血量。引擎接入、动画同步和多人结算属于后续工作。
