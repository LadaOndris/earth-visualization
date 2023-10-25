import os
import click
import math
import glob
import cv2 as cv


def generate_tiles(input_image_path, output_folder, output_file_name, initial_level, max_level,
                   base_level=0, base_index=(0, 0)):
    """
    Supports tiles as input images. The level of the tile is determined by 'base_level'.

    :param base_level: Determines the level of the input image path.
    :return:
    """
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    image = cv.imread(input_image_path, cv.IMREAD_UNCHANGED)
    if image is None:
        raise ValueError(f"Failed to load the image from {input_image_path}: {cv.error}")

    image_height, image_width = image.shape[:2]
    # base_level == 0 if the original image is not a tile.
    # base_level == 1 means the original image is one of the two tiles (2x1)
    # base_level == 2 means the original image is one of the eight tiles (4x2)

    total_levels = base_level + max_level
    target_tile_size = math.ceil((2 ** base_level) * image_width / (2 ** total_levels))

    if base_level == 0:
        x_tiles_on_base_level = 1
        y_tiles_on_base_level = 1

        original_width = image_width
        original_height = image_height
    else:
        x_tiles_on_base_level = 2 ** base_level
        y_tiles_on_base_level = 2 ** (base_level - 1)

        original_width = image_width * x_tiles_on_base_level
        original_height = image_height * y_tiles_on_base_level

    for level in range(initial_level, max_level + 1):
        max_x_tiles = 2 ** level
        max_y_tiles = 2 ** (level - 1 * (base_level == 0))

        # Original indices are required for correct naming
        # level=1, base_level=2 => (8, 4)
        max_x_tiles_in_original = 2 ** (level + base_level)
        max_y_tiles_in_original = 2 ** (level + base_level - 1)

        level_folder = os.path.join(output_folder, f"level_{max_x_tiles_in_original}_{max_y_tiles_in_original}")
        if not os.path.exists(level_folder):
            os.makedirs(level_folder)

        for x_index in range(max_x_tiles):
            for y_index in range(max_y_tiles):
                current_tile_width = image_width // max_x_tiles
                current_tile_height = image_height // max_y_tiles
                left = x_index * current_tile_width
                top = y_index * current_tile_height
                right = left + current_tile_width
                bottom = top + current_tile_height
                tile = image[top:bottom, left:right]
                tile = cv.resize(tile, (target_tile_size, target_tile_size), interpolation=cv.INTER_LINEAR)

                x_index_in_original = int((max_x_tiles_in_original / x_tiles_on_base_level) * base_index[0])
                y_index_in_original = int((max_y_tiles_in_original / y_tiles_on_base_level) * base_index[1])
                x_index_on_current_level = x_index_in_original + x_index
                y_index_on_current_level = y_index_in_original + y_index

                filename = f"{output_file_name}" \
                           f"_{x_index_on_current_level}_{y_index_on_current_level}" \
                           f"_{max_x_tiles_in_original}_{max_y_tiles_in_original}" \
                           f"_{original_width}_{original_height}_{target_tile_size}.png"
                cv.imwrite(os.path.join(level_folder, filename), tile)


@click.command()
@click.argument('input_image_path', type=click.Path(exists=True))
@click.argument('output_folder', type=click.Path())
@click.argument('output_file_name', type=click.Path())
@click.option('--max-level', type=int, default=3, help='Maximum level for tiling')
def generate_texture_tiles(input_image_path, output_folder, output_file_name, max_level):
    initial_level = 1
    base_level = 0
    generate_tiles(input_image_path, output_folder, output_file_name, initial_level,
                   max_level, base_level=base_level)


def generate_height_maps():
    """
    Heightmaps start at 4x2
    """
    indices = {
        # (x, y)
        'A1': (0, 0),
        'A2': (0, 1),
        'B1': (1, 0),
        'B2': (1, 1),
        'C1': (2, 0),
        'C2': (2, 1),
        'D1': (3, 0),
        'D2': (3, 1)
    }

    # Processes tiles starting at level 4x2
    # Thus, each tile should devided into 32x16, producing 128x64 in total.

    input_path_pattern = 'textures/heightmaps/gebco_08_rev_elev_*_grey_geo.tif'
    output_folder = 'textures/heightmaps'
    output_file_name = 'height'
    base_level = 2  # The images loaded are already tiles of this level.
    initial_level = 0
    max_level = 4

    files = glob.glob(input_path_pattern)
    for file in files:
        tile_type = file.split('_')[4]
        assert (tile_type in indices.keys())
        generate_tiles(file, output_folder, output_file_name, initial_level, max_level,
                       base_level, base_index=indices[tile_type])


if __name__ == "__main__":
    generate_height_maps()
