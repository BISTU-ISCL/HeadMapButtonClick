#!/usr/bin/env python3
import math
import os
from typing import List, Tuple

# 基于标准库实现的简易热力图生成脚本，用于在无法运行 Qt Demo 时生成示例截图。
# 生成的位图文件位于 artifacts/sample_heatmap.bmp，便于在文档中展示。

Color = Tuple[int, int, int]


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def blend_channel(bg: int, fg: int, alpha: float) -> int:
    return int(round(bg * (1.0 - alpha) + fg * alpha))


def generate_heatmap(
    size: Tuple[int, int],
    points: List[Tuple[float, float]],
    radius: float = 60.0,
    opacity: float = 0.6,
    cold_color: Color = (0, 188, 212),
    hot_color: Color = (255, 87, 34),
    background: Color = (245, 247, 250),
) -> bytes:
    width, height = size
    sigma2 = radius * radius / 2.0

    # 预计算背景（简易渐变以凸显叠加效果）。
    bg_rows: List[Color] = []
    for y in range(height):
        grad = 0.05 + 0.95 * (y / max(1, height - 1))
        row_color = (
            int(background[0] * grad),
            int(background[1] * grad),
            int(background[2] * grad),
        )
        bg_rows.append(row_color)

    intensities = [[0.0 for _ in range(width)] for _ in range(height)]
    max_intensity = 1e-6

    for px, py in points:
        cx = px * width
        cy = py * height
        x0 = max(0, int(cx - 3 * radius))
        x1 = min(width - 1, int(cx + 3 * radius))
        y0 = max(0, int(cy - 3 * radius))
        y1 = min(height - 1, int(cy + 3 * radius))
        for y in range(y0, y1 + 1):
            dy2 = (y - cy) ** 2
            row = intensities[y]
            for x in range(x0, x1 + 1):
                dist2 = (x - cx) ** 2 + dy2
                val = math.exp(-dist2 / sigma2)
                row[x] += val
                if row[x] > max_intensity:
                    max_intensity = row[x]

    # 自动归一化，将最大值映射到 1.0。
    for y in range(height):
        row = intensities[y]
        for x in range(width):
            row[x] = row[x] / max_intensity

    # 构造像素数据（BMP 底部朝上）。
    row_padding = (4 - (width * 3) % 4) % 4
    pixel_bytes = bytearray()
    for y in reversed(range(height)):
        bg_r, bg_g, bg_b = bg_rows[y]
        for x in range(width):
            t = intensities[y][x]
            heat_r = int(round(lerp(cold_color[0], hot_color[0], t)))
            heat_g = int(round(lerp(cold_color[1], hot_color[1], t)))
            heat_b = int(round(lerp(cold_color[2], hot_color[2], t)))
            alpha = opacity * t
            out_r = blend_channel(bg_r, heat_r, alpha)
            out_g = blend_channel(bg_g, heat_g, alpha)
            out_b = blend_channel(bg_b, heat_b, alpha)
            pixel_bytes.extend((out_b, out_g, out_r))
        pixel_bytes.extend(b"\x00" * row_padding)

    # 构造 BMP 头（24 位）。
    file_size = 14 + 40 + len(pixel_bytes)
    header = bytearray()
    header.extend(b"BM")
    header.extend(file_size.to_bytes(4, "little"))
    header.extend((0).to_bytes(4, "little"))
    header.extend((14 + 40).to_bytes(4, "little"))

    dib = bytearray()
    dib.extend((40).to_bytes(4, "little"))  # DIB header size
    dib.extend(width.to_bytes(4, "little", signed=True))
    dib.extend(height.to_bytes(4, "little", signed=True))
    dib.extend((1).to_bytes(2, "little"))  # planes
    dib.extend((24).to_bytes(2, "little"))  # bpp
    dib.extend((0).to_bytes(4, "little"))  # compression
    dib.extend(len(pixel_bytes).to_bytes(4, "little"))
    dib.extend((2835).to_bytes(4, "little"))  # X ppm (~72 DPI)
    dib.extend((2835).to_bytes(4, "little"))  # Y ppm
    dib.extend((0).to_bytes(4, "little"))  # colors used
    dib.extend((0).to_bytes(4, "little"))  # important colors

    return bytes(header + dib + pixel_bytes)


def main() -> None:
    width, height = 900, 600
    clicks = [
        (0.22, 0.30),
        (0.28, 0.32),
        (0.50, 0.55),
        (0.52, 0.60),
        (0.76, 0.42),
        (0.78, 0.45),
        (0.80, 0.40),
    ]

    bmp = generate_heatmap(
        size=(width, height),
        points=clicks,
        radius=70.0,
        opacity=0.65,
    )

    os.makedirs("artifacts", exist_ok=True)
    out_path = os.path.join("artifacts", "sample_heatmap.bmp")
    with open(out_path, "wb") as f:
        f.write(bmp)
    print(f"Generated sample heatmap -> {out_path}")


if __name__ == "__main__":
    main()
