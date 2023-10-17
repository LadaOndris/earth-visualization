import os
import click
from PIL import Image


@click.command()
@click.argument('input_image_path', type=click.Path(exists=True))
@click.argument('output_folder', type=click.Path())
@click.argument('output_file_name', type=click.Path())
@click.option('--max-level', type=int, default=3, help='Maximum level for tiling')
@click.option('--tile-width', type=int, default=128, help='Width of each tile')
@click.option('--tile-height', type=int, default=128, help='Height of each tile')
def generate_tiles(input_image_path, output_folder, output_file_name, max_level, tile_width, tile_height):
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    original_image = Image.open(input_image_path)
    original_width, original_height = original_image.size

    for level in range(1, max_level + 1):
        max_x_tiles = 2 ** level
        max_y_tiles = 2 ** (level - 1)
        level_folder = os.path.join(output_folder, f"level_{max_x_tiles}_{max_y_tiles}")
        if not os.path.exists(level_folder):
            os.makedirs(level_folder)

        for x_index in range(max_x_tiles):
            for y_index in range(max_y_tiles):
                current_tile_width = original_width // max_x_tiles
                current_tile_height = original_height // max_y_tiles
                left = x_index * current_tile_width
                top = y_index * current_tile_height
                right = left + current_tile_width
                bottom = top + current_tile_height
                tile = original_image.crop((left, top, right, bottom))
                tile = tile.resize((tile_width, tile_height), Image.ANTIALIAS)

                filename = f"{output_file_name}_{x_index}_{y_index}_{max_x_tiles}_{max_y_tiles}_{original_width}_{original_height}.png"
                tile.save(os.path.join(level_folder, filename), "PNG")


if __name__ == "__main__":
    generate_tiles()
