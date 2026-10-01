"""Required map effects must fail visibly before storing an invalid pool index."""

import subprocess
import sys


# Effect switch, orbiting projectile, short swing, long swing (item_types.h).
for object_id in (135, 136, 138, 139):
    for failure in ("exhausted", "outside"):
        result = subprocess.run(
            [sys.argv[1], "--invalid-map-effect", str(object_id), failure],
            capture_output=True, text=True, timeout=10,
        )
        if result.returncode != 1 or result.stderr != "Cannot allocate required map object effect.\n":
            raise RuntimeError(f"{object_id}/{failure}: {result.returncode}\n{result.stderr}")

for operation in (0, 2, 3):
    for x, z in ((0, 50), (99, 50), (50, 0), (50, 99)):
        for yaw in (0, 1024, 2048, 3072):
            result = subprocess.run(
                [sys.argv[1], "--door-edge", str(operation), str(x), str(z), str(yaw)],
                capture_output=True, text=True, timeout=10,
            )
            if result.returncode != 1 or result.stderr != "Door placement exceeds collision grid bounds.\n":
                raise RuntimeError(f"{operation}/{x}/{z}/{yaw}: {result.returncode}\n{result.stderr}")
