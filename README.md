# CANN Fuzzy Vault + LFW ArcFace + AdaMTrans

这是此前合成数据版 Fuzzy Vault 的独立真实数据扩展工程。它读取 512 维 LFW ArcFace 特征，将 WiFaKey 的 AdaMTrans 特征转换移植为 Ascend C，再生成适合 Vault 集合匹配的无序 token。

本工程不依赖、不修改也不覆盖 Fuzzy Commitment 和 Fuzzy Extractor。

## 为什么 Vault 不使用 LDPC

WiFaKey 的 BG2 LDPC 用于二进制纠错，适合 Commitment 和 Extractor。Fuzzy Vault 的原生恢复机制是“集合重合 + 有限域多项式插值 + chaff 点”，强行叠加 LDPC 会把三个协议混在一起。因此：

- Commitment/Extractor 的真实数据版本使用 WiFaKey BG2/Z=10 LDPC。
- Vault 使用同一 CANN AdaMTrans 前端，但保持 Vault 自己的有限域恢复结构。

## CANN 执行路径

1. `fuzzy_vault_lfw_adamtrans`
   - 512 维 ArcFace 浮点特征。
   - 四区间 LSSC，得到 1536 bit。
   - 固定可复现的 AdaMTrans 随机掩码。
2. Host 将 1536 bit 分成 96 个带位置的 16-bit token。
3. `fuzzy_vault_lfw_match_points`
   - 将 96 个查询 token 与 192 个乱序 Vault 点进行精确集合匹配。
4. Host 在 `GF(2147483647)` 中恢复 7 次多项式，并验证系数 SHA-256。

Vault 中包含 96 个 genuine 点和 96 个 chaff 点，恢复阈值为 8 个匹配点。

## 数据与运行

默认输入：

```text
/home/dsh/下载/embeddings_arcface_r100_lfw.csv
/home/dsh/cann-dev/biometric-data/lfw_issame.csv
```

运行：

```bash
./run-tests.sh
```

指定其他文件：

```bash
./run-tests.sh /path/to/embeddings.csv /path/to/lfw_issame.csv
```

对全部 6000 对 LFW 样本进行评估：

```bash
./run-all.sh
```

也可以指定数据位置：

```bash
./run-all.sh /path/to/embeddings.csv /path/to/lfw_issame.csv
```

`run-tests.sh` 保留原来的 5 对正样本 + 5 对负样本快速回归测试；
`run-all.sh` 编译同一套 Vault 算子的全量模式，处理全部 6000 对并输出
TP/TN/FP/FN、GMR、FMR、FNMR、平均 token 重合数和总耗时。两种模式
均由固定 20 个 CANN worker 循环处理样本，适合当前 CPU 仿真环境。

特征必须是 `12000 × 512` CSV，每两行一对，并与 6000 行标签顺序一致。150 MB ArcFace 文件不上传到仓库。

## 当前测试结果

- 正样本：LFW pair 0–4，token 重合分别为 `9, 25, 20, 20, 25`。
- 负样本：LFW pair 300–304，token 重合分别为 `1, 1, 0, 2, 2`。
- 恢复阈值：8。

```text
[RESULT] CANN/Host pipeline agreement: 10/10
[RESULT] Biometric classification: 10/10
[PASS] Fuzzy vault LFW CANN pipeline completed.
```

全量模式的实测结果记录在下方；如更换特征或参数，请以新一次运行输出为准：

```text
[FULL_RESULT] TP=2684 TN=2997 FP=3 FN=316
[FULL_RESULT] accuracy=0.946833 GMR=0.894667 FMR=0.001000 FNMR=0.105333
[FULL_RESULT] average_positive_token_overlap=14.002000 average_negative_token_overlap=1.861333
[FULL_RESULT] CANN/Host pipeline agreement=6000/6000 elapsed_seconds=1.915156
[PASS] Full 6000-pair fuzzy vault CANN evaluation completed.
```

分类准确率只作为观察值；CANN 与独立 Host 参考实现不一致才是测试失败条件。

## 主要参数

| 项目 | 值 |
|---|---:|
| ArcFace / LSSC | 512 float / 1536 bit |
| Token | 96 个带位置的 16-bit 块 |
| 有限域 | GF(2147483647) |
| 多项式次数 / 阈值 | 7 / 8 |
| Genuine / chaff | 96 / 96 |
| Vault 总点数 | 192 |

## 局限

- CANN CPU 仿真不能代表真实 NPU 性能。
- 10 对快速样本用于端到端回归；6000 对全量模式用于遍历数据集和输出统计，仍不等同于规范的交叉验证论文指标。
- 16-bit 精确 token 是可运行原型；正式设计应评估量化参数、熵和 token 碰撞。
- 为避免本原型的随机 chaff 干扰基础流程，chaff `x` 使用与生物 token 不相交的数值域。这不具备生产级 Vault 隐蔽性，正式方案必须让 chaff 与 genuine 点不可区分，并使用容错多项式恢复。
- 阈值当前由整份数据统计得到，正式评估必须使用训练折/测试折隔离。
- 固定掩码和伪随机数不适合生产密钥系统。

## 参考

- [WiFaKey 论文](https://arxiv.org/abs/2407.14804)
- [WiFaKey 官方源代码](https://github.com/xingbod/WiFaKey)
- Juels and Sudan, [A Fuzzy Vault Scheme](https://cse.msu.edu/~rossarun/BiometricsTextBook/Papers/Security/JuelsSudan_FuzzyVault.pdf)

详见 `THIRD_PARTY_NOTICES.md`。

