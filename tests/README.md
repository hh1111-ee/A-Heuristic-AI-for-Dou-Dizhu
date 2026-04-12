压测脚本使用说明

生成测试用例：
- 脚本：tests/generate_requests.py
- 示例：

```powershell
python tests/generate_requests.py --count 200 --outdir tests/cases
```

运行压测：
- 脚本：tests/run_benchmark.py
- 示例（串行）：

```powershell
python tests/run_benchmark.py --cases tests/cases --bin my_bot.exe --repeat 1000 --workers 1
```

- 示例（并发）：

```powershell
python tests/run_benchmark.py --cases tests/cases --bin my_bot.exe --repeat 2000 --workers 8
```

说明：
- 请先在工作目录下编译可执行文件：

```powershell
g++ -o my_bot.exe main.cpp
```

- 脚本默认会把结果写入 `tests/results.json`，其中包含统计信息和部分延迟样本。
