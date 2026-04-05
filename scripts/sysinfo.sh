#!/usr/bin/env bash
set -euo pipefail

mkdir -p profile
out=profile/sysinfo.txt

cpu=$(sysctl -n machdep.cpu.brand_string 2>/dev/null || echo "unknown")
pcores=$(sysctl -n hw.perflevel0.physicalcpu 2>/dev/null || echo "N/A")
ecores=$(sysctl -n hw.perflevel1.physicalcpu 2>/dev/null || echo "N/A")
memsize=$(sysctl -n hw.memsize 2>/dev/null || echo "0")
ram_gb=$((memsize / 1073741824))
cacheline=$(sysctl -n hw.cachelinesize 2>/dev/null || echo "N/A")

p_l1d=$(($(sysctl -n hw.perflevel0.l1dcachesize 2>/dev/null || echo "0") / 1024))
p_l1i=$(($(sysctl -n hw.perflevel0.l1icachesize 2>/dev/null || echo "0") / 1024))
p_l2=$(($(sysctl -n hw.perflevel0.l2cachesize 2>/dev/null || echo "0") / 1048576))
e_l1d=$(($(sysctl -n hw.perflevel1.l1dcachesize 2>/dev/null || echo "0") / 1024))
e_l1i=$(($(sysctl -n hw.perflevel1.l1icachesize 2>/dev/null || echo "0") / 1024))
e_l2=$(($(sysctl -n hw.perflevel1.l2cachesize 2>/dev/null || echo "0") / 1048576))

os_name=$(sw_vers -productName 2>/dev/null || echo "unknown")
os_ver=$(sw_vers -productVersion 2>/dev/null || echo "unknown")
os_build=$(sw_vers -buildVersion 2>/dev/null || echo "unknown")
compiler=$(/opt/homebrew/opt/llvm/bin/clang++ --version 2>/dev/null | head -1 || echo "unknown")
xcode=$(xcodebuild -version 2>/dev/null | head -1 || echo "N/A")
cmake_ver=$(cmake --version 2>/dev/null | head -1 || echo "unknown")
ninja_ver=$(ninja --version 2>/dev/null || echo "unknown")

cat > "$out" << EOF
System Information
==================

Date:           $(date '+%Y-%m-%d %H:%M:%S %z')

Hardware
--------
CPU:            ${cpu}
P-cores:        ${pcores}
E-cores:        ${ecores}
RAM:            ${ram_gb} GB
Cache line:     ${cacheline} bytes

Cache Hierarchy (per-core)
--------------------------
P-core L1d:     ${p_l1d} KB
P-core L1i:     ${p_l1i} KB
P-core L2:      ${p_l2} MB
E-core L1d:     ${e_l1d} KB
E-core L1i:     ${e_l1i} KB
E-core L2:      ${e_l2} MB

Software
--------
OS:             ${os_name} ${os_ver} (${os_build})
Compiler:       ${compiler}
Xcode:          ${xcode}
CMake:          ${cmake_ver}
Ninja:          ${ninja_ver}
EOF

echo "saved → ${out}"
