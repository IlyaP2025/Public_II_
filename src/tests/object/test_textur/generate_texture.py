from PIL import Image

size = 512
cells = 8
cell = size // cells
img = Image.new('RGB', (size, size))
for i in range(cells):
    for j in range(cells):
        color = (255, 255, 255) if (i + j) % 2 == 0 else (0, 0, 0)
        for x in range(cell):
            for y in range(cell):
                img.putpixel((i*cell + x, j*cell + y), color)
img.save('cube_texture.bmp')
print("Texture saved as cube_texture.bmp")
