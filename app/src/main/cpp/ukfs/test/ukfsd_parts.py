#!/usr/bin/env python3
# Partition-table regression for ukfsd: PARTS enumeration + per-partition MOUNT
# (uksd0pN) over a UKFSD_DEVDIR file backend. Drives a running ukfsd whose
# UKFSD_DEVDIR contains a multi-partition image named "uksd0".
#
#   ukfsd_parts.py <fs-sock-name>
#
# Exits 0 and prints "ALL PARTITION TESTS PASSED" on success.
import socket, struct, sys

FS = sys.argv[1]
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM); s.settimeout(30)
s.connect('\0' + FS)
buf = bytearray(); fails = 0
def line():
    while b'\n' not in buf:
        d = s.recv(65536)
        if not d: raise EOFError("ukfsd closed connection")
        buf.extend(d)
    i = buf.index(b'\n'); l = bytes(buf[:i]); del buf[:i+1]; return l.decode()
def nb(n):
    while len(buf) < n: buf.extend(s.recv(65536))
    d = bytes(buf[:n]); del buf[:n]; return d
def cmd(c): s.sendall(c.encode() + b'\n')
def listdir(p):
    cmd('LIST ' + p); r = line().split()
    if r[0] != 'OK': return None
    blob = nb(int(r[2])); off = 0; out = []
    for _ in range(int(r[1])):
        nl = struct.unpack('<H', blob[off+17:off+19])[0]
        out.append(blob[off+19:off+19+nl].decode()); off += 19 + nl
    return out
def readf(p, n, o=0):
    cmd('READ %d %d %s' % (o, n, p)); r = line()
    return nb(int(r.split()[1])) if r.startswith('OK') else None
def writef(p, data, o=0):
    cmd('WRITE %d %d %s' % (o, len(data), p)); s.sendall(data); return line()
def check(name, cond):
    global fails
    print(("  PASS  " if cond else "  FAIL  ") + name)
    if not cond: fails += 1

# 1) enumerate the partition table
cmd('PARTS uksd0'); h = line().split()
check("PARTS returns OK", h[0] == 'OK')
n = int(h[1]); parts = []
for _ in range(n):
    f = line().split(); parts.append((f[0], f[1], int(f[2]), int(f[3])))
print("  (info) partitions:", parts)
check("PARTS found 2 partitions", n == 2)
check("p1 starts at 1 MiB (0x0c FAT)", parts[0][0] == 'p1' and parts[0][2] == 1048576)
check("p2 follows p1 (0x83 Linux)", parts[1][0] == 'p2' and parts[1][2] > parts[0][2])

# 2) mount + use partition 1 (FAT boot)
cmd('MOUNT auto uksd0p1'); check("MOUNT uksd0p1 (FAT)", line() == 'OK')
check("p1 sees seeded CMDLINE.TXT", 'CMDLINE.TXT' in (listdir('/') or []))
check("p1 WRITE new file", writef('/boot.cfg', b'enable_uart=1\n') == 'OK 14')
check("p1 READ back", readf('/boot.cfg', 64) == b'enable_uart=1\n')
cmd('SYNC'); line()

# 3) mount + use partition 2 (ext4 root), incl. a multi-block file
cmd('MOUNT auto uksd0p2'); check("MOUNT uksd0p2 (ext4)", line() == 'OK')
check("p2 MKDIR /etc", (cmd('MKDIR 493 /etc'), line())[1] == 'OK')
check("p2 WRITE /etc/hostname", writef('/etc/hostname', b'neoterm\n') == 'OK 8')
big = bytes((i * 37 & 0xff) for i in range(200000))
check("p2 WRITE 200K (extents)", writef('/big.dat', big) == 'OK 200000')
back = bytearray(); off = 0
while off < len(big):
    d = readf('/big.dat', 65536, off)
    if not d: break
    back.extend(d); off += len(d)
check("p2 READ 200K byte-exact", bytes(back) == big)
cmd('SYNC'); line()

# 4) partition isolation: re-mount p1, p2's content must not be visible there
cmd('MOUNT auto uksd0p1'); check("re-MOUNT p1", line() == 'OK')
r1 = listdir('/') or []
check("p1 kept boot.cfg across p2 session", 'boot.cfg' in r1)
check("p1 does NOT see p2's /etc (isolation)", 'etc' not in r1)

s.close()
if fails:
    print("\n%d PARTITION TEST(S) FAILED" % fails); sys.exit(1)
print("\nALL PARTITION TESTS PASSED")
