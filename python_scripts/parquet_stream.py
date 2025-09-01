#!/usr/bin/env python3
import sys
import pyarrow.parquet as pq
import pyarrow as pa

# 输出 CSV 行格式：
# type,symbol,side,id,price,size,fill_size
# type: ADD/MOD/CXL/FILL/HB   side: B/S
# 价位 price & 数量 size 建议都换成整数最小单位（如 价格 * 1e2/1e4，或交易所给的 tick 单位）
# 你需要把列名映射到自己拿到的 Databento L3 parquet 的列名。

if len(sys.argv) < 2:
    print("Usage: parquet_stream.py <file.parquet>", file=sys.stderr)
    sys.exit(1)

path = sys.argv[1]
tbl = pq.read_table(path)

# ---- 根据你数据的真实列名做映射（下面是常见/示例名）----
# 例：event_type in ["ADD","MODIFY","CANCEL","TRADE"]
col_evt   = "event_type"
col_sym   = "symbol"
col_side  = "side"        # "B"/"S" or 1/-1
col_oid   = "order_id"
col_px    = "price"       # 若是浮点，后面*100或*10000转整数
col_sz    = "size"
col_fill  = "fill_size"   # 若无此列，遇到 TRADE 就用 trade_size；否则输出 0

# 统一列
cols_needed = [col_evt, col_sym, col_side, col_oid, col_px, col_sz]
missing = [c for c in cols_needed if c not in tbl.schema.names]
if missing:
    print(f"Missing columns in parquet: {missing}", file=sys.stderr)
    sys.exit(2)

arr_evt  = tbl[col_evt]
arr_sym  = tbl[col_sym]
arr_side = tbl[col_side]
arr_oid  = tbl[col_oid]
arr_px   = tbl[col_px]
arr_sz   = tbl[col_sz]
arr_fill = tbl[col_fill] if col_fill in tbl.schema.names else pa.array([0]*len(tbl))

n = len(tbl)
to_int_price = True if pa.types.is_floating(arr_px.type) else False

for i in range(n):
    t  = arr_evt[i].as_py()
    sy = arr_sym[i].as_py()
    sd = arr_side[i].as_py()
    if isinstance(sd, int):
        sd = "B" if sd>0 else "S"
    oid= str(arr_oid[i].as_py())

    px = arr_px[i].as_py()
    if px is None: continue
    if to_int_price:
        px = int(round(px*100))   # 例：保留两位；按你的合约改
    else:
        px = int(px)

    sz = arr_sz[i].as_py() or 0
    fl = arr_fill[i].as_py() or 0

    # 标准化类型
    if t.upper().startswith("ADD"):
        typ = "ADD"
    elif t.upper().startswith("MOD"):
        typ = "MOD"
    elif t.upper().startswith("CANC"):
        typ = "CXL"
    elif t.upper().startswith("TRADE") or t.upper()=="FILL":
        typ = "FILL"
    else:
        typ = "HB"

    # CSV：type,symbol,side,id,price,size,fill_size
    print(f"{typ},{sy},{sd},{oid},{px},{sz},{fl}")
