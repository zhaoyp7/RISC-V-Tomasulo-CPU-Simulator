#!/bin/bash
set -e

# === 配置 ===
CXX="${CXX:-g++}"
CXXFLAGS="-std=c++17 -O2"
SRC="main.cpp"
BIN="./main_tmp"
DATA_DIRS=("data/testcases" "data/sample")
LOG_DIR="log"
TIMEOUT_SEC=10

# === 颜色 ===
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# === 预期输出 ===
declare -A EXPECTED=(
    ["naive"]=94
    ["sample"]=94
    ["array_test1"]=123
    ["array_test2"]=43
    ["basicopt1"]=88
    ["bulgarian"]=159
    ["expr"]=58
    ["gcd"]=178
    ["hanoi"]=20
    ["lvalue2"]=175
    ["magic"]=106
    ["manyarguments"]=40
    ["multiarray"]=115
    ["pi"]=137
    ["qsort"]=105
    ["queens"]=171
    ["statement_test"]=50
    ["superloop"]=134
    ["tak"]=186
)

# === 初始化日志 ===
mkdir -p "$LOG_DIR"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="$LOG_DIR/test_$TIMESTAMP.log"

log()  { echo "$1" >> "$LOG_FILE"; }

# === 编译 ===
echo -e "${YELLOW}[1/3] 编译 $SRC ...${NC}"
log "========================================="
log "  RISC-V Tomasulo CPU Simulator Test Report"
log "========================================="
log "Date: $(date '+%Y-%m-%d %H:%M:%S')"
log "Compiler: $CXX"
log "Flags: $CXXFLAGS"
log "Timeout: ${TIMEOUT_SEC}s"
log ""

COMPILE_START=$(date +%s%N)
$CXX $CXXFLAGS "$SRC" -o "$BIN" 2>&1 | tee -a "$LOG_FILE" > /dev/null
COMPILE_STATUS=$?
COMPILE_END=$(date +%s%N)
COMPILE_MS=$(( (COMPILE_END - COMPILE_START) / 1000000 ))

if [ $COMPILE_STATUS -ne 0 ]; then
    echo -e "${RED}编译失败${NC}"
    log "编译失败"
    exit 1
fi
echo -e "${GREEN}编译成功${NC} (${COMPILE_MS}ms)"
log "Compile: OK (${COMPILE_MS}ms)"
log ""

# === 运行测试 ===
TOTAL=0
PASS=0
FAIL=0

echo -e "${YELLOW}[2/3] 运行测试用例 (timeout=${TIMEOUT_SEC}s)${NC}"
log "Test Results:"
log ""

# 表头
HEADER_FMT="%-20s %-8s %-8s %-10s %s\n"
printf "$HEADER_FMT" "Testcase" "Expect" "Actual" "Time" "Status" >> "$LOG_FILE"
printf "$HEADER_FMT" "--------" "------" "------" "----------" "------" >> "$LOG_FILE"

for dir in "${DATA_DIRS[@]}"; do
    for data_file in "$dir"/*.data; do
        [ -f "$data_file" ] || continue
        name=$(basename "$data_file" .data)
        expected="${EXPECTED[$name]}"
        TOTAL=$((TOTAL + 1))

        if [ -z "$expected" ]; then
            echo -e "  ${YELLOW}SKIP${NC} $name (无预期值)"
            printf "$HEADER_FMT" "$name" "-" "-" "-" "SKIP" >> "$LOG_FILE"
            continue
        fi

        TEST_START=$(date +%s%N)
        result=$(timeout $TIMEOUT_SEC "$BIN" < "$data_file" 2>&1) || true
        exit_code=$?
        TEST_END=$(date +%s%N)
        TEST_MS=$(( (TEST_END - TEST_START) / 1000000 ))

        if [ $exit_code -eq 124 ]; then
            echo -e "  ${RED}FAIL${NC} $name — 超时 (>${TIMEOUT_SEC}s)"
            printf "$HEADER_FMT" "$name" "$expected" "TIMEOUT" "${TEST_MS}ms" "FAIL" >> "$LOG_FILE"
            FAIL=$((FAIL + 1))
        elif [ "$result" = "$expected" ]; then
            echo -e "  ${GREEN}PASS${NC} $name — $result (${TEST_MS}ms)"
            printf "$HEADER_FMT" "$name" "$expected" "$result" "${TEST_MS}ms" "PASS" >> "$LOG_FILE"
            PASS=$((PASS + 1))
        else
            echo -e "  ${RED}FAIL${NC} $name — 期望 $expected，实际 $result (${TEST_MS}ms)"
            printf "$HEADER_FMT" "$name" "$expected" "$result" "${TEST_MS}ms" "FAIL" >> "$LOG_FILE"
            FAIL=$((FAIL + 1))
        fi
    done
done

log ""

# === 汇总 ===
echo ""
echo -e "${YELLOW}[3/3] 结果汇总${NC}"
echo -e "  总计: $TOTAL"
if [ $FAIL -gt 0 ]; then
    echo -e "  通过: ${GREEN}$PASS${NC}"
    echo -e "  失败: ${RED}$FAIL${NC}"
else
    echo -e "  通过: ${GREEN}$PASS${NC}"
    echo -e "  失败: 0"
fi
echo -e "  详见: ${LOG_FILE}"

log "========================================="
log "  总计: $TOTAL"
log "  通过: $PASS"
log "  失败: $FAIL"
log "========================================="

# === 清理 ===
rm -f "$BIN"

# === 退出码 ===
if [ $FAIL -gt 0 ]; then
    exit 1
else
    exit 0
fi