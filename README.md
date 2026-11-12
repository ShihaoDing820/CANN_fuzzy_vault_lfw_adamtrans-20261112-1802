# CANN LDPC + Fuzzy Vault + LFW ArcFace

这是独立的真实生物特征混合原型：读取 512 维 LFW ArcFace 特征，先通过
CANN AdaMTrans 得到二值见证，再用 WiFaKey 同结构的 5G-NR BG2/Z=10
LDPC 在 CANN 中纠错和重构见证，最后执行 CANN Fuzzy Vault 集合匹配。

本工程不依赖、不修改也不覆盖 Fuzzy Commitment 和 Fuzzy Extractor。

## 混合协议

注册阶段：

1. `fuzzy_vault_lfw_adamtrans` 将 ArcFace 特征转换成 1536-bit 见证 `w`。
2. 将 `w` 拆成三个 512-bit 块，每块补 8 个零成为 520 bit。
3. 为每块生成 WiFaKey BG2、`Z=10` 的随机 LDPC 码字 `c`，公开
   helper data `P = w XOR c`。
4. 将注册见证转换为 96 个带位置的 16-bit token，并生成 96 个 genuine
   点、96 个 chaff 点和一个 7 次多项式 Vault。

验证阶段：

1. CANN AdaMTrans 得到探针见证 `w'`。
2. `fuzzy_vault_lfw_ldpc_reconstruct` 对 `w' XOR P` 执行 BG2/Z=10
   Normalized Min-Sum，并输出重构见证 `w_hat = P XOR c_hat`。
3. 只使用 `w_hat` 生成查询 token；原始探针不会直接进入 Vault 匹配。
4. `fuzzy_vault_lfw_match_points` 将 96 个查询 token 与 192 个 Vault 点匹配。
5. 匹配数达到 80 后，Host 使用其中 8 点在 `GF(2147483647)` 中恢复
   7 次多项式，并用 SHA-256 验证系数。

80-token 是混合协议的解锁策略，不是多项式的最低插值点数。7 次多项式
数学上只需要 8 点，但 LDPC 每块对应 32 个 token；门限必须高于 64，才能
避免仅纠正一到两个块就直接解锁 Vault。

## CANN 算子

- `fuzzy_vault_lfw_adamtrans`：LSSC 量化、二值化和 AdaMTrans 掩码。
- `fuzzy_vault_lfw_ldpc_reconstruct`：BG2/Z=10 Normalized Min-Sum 解码及见证重构。
- `fuzzy_vault_lfw_match_points`：纠错后 token 与 Vault 点的集合匹配。

三个核心阶段均使用 Ascend C/CANN kernel。有限域插值、SHA-256、文件读取
和测试统计位于 Host。当前在 CANN 9.0.0 CPU 仿真中验证，不代表真实 NPU 性能。

## 数据与运行

默认输入：

```text
/home/dsh/下载/embeddings_arcface_r100_lfw.csv
/home/dsh/cann-dev/biometric-data/lfw_issame.csv
```

5 对正样本 + 5 对负样本的逐 bit CANN/Host 回归：

```bash
./run-tests.sh
```

全部 6000 对 LFW 样本：

```bash
./run-all.sh
```

也可以指定数据位置：

```bash
./run-tests.sh /path/to/embeddings.csv /path/to/lfw_issame.csv
./run-all.sh /path/to/embeddings.csv /path/to/lfw_issame.csv
```

特征 CSV 必须是 `12000 × 512`，每两行一对，并与 6000 行标签顺序一致。
150 MB ArcFace 特征文件不上传到仓库。

## 当前测试结果

快速回归对 AdaMTrans、三个 LDPC 块的重构 bit/状态/迭代数以及 Vault 匹配
结果执行独立 Host 对比：

```text
[RESULT] CANN/Host hybrid pipeline agreement: 10/10
[RESULT] Biometric classification: 10/10
[PASS] LDPC + fuzzy vault LFW CANN pipeline completed.
```

全量 6000 对实测：

```text
[FULL_RESULT] TP=2989 TN=2824 FP=176 FN=11
[FULL_RESULT] accuracy=0.968833 GMR=0.996333 FMR=0.058667 FNMR=0.003667
[FULL_RESULT] average_positive_token_overlap=95.821333 average_negative_token_overlap=34.458333
[FULL_RESULT] valid_ldpc_pipeline_pairs=6000/6000 average_ldpc_iterations=22.584722
[FULL_RESULT] valid_hybrid_pipeline_pairs=6000/6000 elapsed_seconds=11.290574
[PASS] Full 6000-pair LDPC + fuzzy vault CANN evaluation completed.
```

全量模式对 12000 个 AdaMTrans 输出和 6000 组 Vault 匹配执行 Host 对比；
18000 个 LDPC 块全部由 CANN 执行并检查输出范围、状态和迭代元数据。完整
LDPC 逐 bit Host 对比保留在快速回归中，避免全量模式重复执行全部解码。

分类结果是当前固定参数下的观察值，不是正式论文指标，也不是程序退出条件。

## 主要参数

| 项目 | 值 |
|---|---:|
| ArcFace / LSSC | 512 float / 1536 bit |
| LDPC | 5G-NR BG2 Set 2，Z=10 |
| LDPC N / K / CN / 边 | 520 / 100 / 420 / 1970 |
| LDPC 块数 | 每对 3 个 |
| LDPC 解码 | Normalized Min-Sum，最多 50 轮，系数 0.75 |
| Token | 96 个带位置的 16-bit 块 |
| 有限域 | GF(2147483647) |
| 多项式次数 / 插值点 | 7 / 8 |
| 混合解锁门限 | 80 token |
| Genuine / chaff | 96 / 96 |
| Vault 总点数 | 192 |

## 局限

- 这是“LDPC 预纠错 + Fuzzy Vault”的混合原型，不是经典 Fuzzy Vault。
- CANN CPU 仿真不能代表真实 NPU 性能。
- 当前全量统计没有按 LFW 十折训练/测试协议重新拟合阈值。
- helper data 与 Vault 同时公开时的联合泄漏、可链接性和模板熵尚未完成安全证明。
- 16-bit 精确 token、固定掩码和伪随机码字仅用于可复现测试。
- chaff `x` 使用与生物 token 不相交的数值域，不具备生产级 Vault 隐蔽性。
- 生产实现需要 CSPRNG、经过审计的参数、不可区分 chaff 和 side-channel 防护。

## 参考

- [WiFaKey 论文](https://arxiv.org/abs/2407.14804)
- [WiFaKey 官方源代码](https://github.com/xingbod/WiFaKey)
- Juels and Sudan, [A Fuzzy Vault Scheme](https://cse.msu.edu/~rossarun/BiometricsTextBook/Papers/Security/JuelsSudan_FuzzyVault.pdf)

详见 `THIRD_PARTY_NOTICES.md`。
