import os
import math
import glob
import cv2 as cv


TEXTURES = [
    {
        'name': 'day',
        'input_path': 'textures/raw/2_no_clouds_16k.jpg',
        'output_folder': 'textures/generated/daymaps',
        'output_file_name': 'day',
        'base_level': 0,
        'initial_level': 2,
        'max_level': 6,
    },
    {
        'name': 'night',
        'input_path': 'textures/raw/5_night_16k.jpg',
        'output_folder': 'textures/generated/nightmaps',
        'output_file_name': 'night',
        'base_level': 0,
        'initial_level': 2,
        'max_level': 6,
    },
    {
        'name': 'height',
        'input_pattern': 'textures/raw/heightmaps/gebco_08_rev_elev_*_grey_geo.tif',
        'output_folder': 'textures/generated/heightmaps',
        'output_file_name': 'height',
        'base_level': 2,
        'initial_level': 0,
        'max_level': 4,
        'tile_indices': {
            'A1': (0, 0), 'A2': (0, 1),
            'B1': (1, 0), 'B2': (1, 1),
            'C1': (2, 0), 'C2': (2, 1),
            'D1': (3, 0), 'D2': (3, 1),
        },
    },
]


def generate_tiles(input_path, output_folder, output_file_name, initial_level, max_level,
                   base_level, base_index=(0, 0)):
    image = cv.imread(input_path, cv.IMREAD_UNCHANGED)
    if image is None:
        raise ValueError(f"Failed to load image from {input_path}")

    image_height, image_width = image.shape[:2]
    target_tile_size = math.ceil((2 ** base_level) * image_width / (2 ** (base_level + max_level)))

    if base_level == 0:
        x_tiles_on_base_level, y_tiles_on_base_level = 1, 1
        original_width, original_height = image_width, image_height
    else:
        x_tiles_on_base_level = 2 ** base_level
        y_tiles_on_base_level = 2 ** (base_level - 1)
        original_width = image_width * x_tiles_on_base_level
        original_height = image_height * y_tiles_on_base_level

    for level in range(initial_level, max_level + 1):
        max_x_tiles = 2 ** level
        max_y_tiles = 2 ** (level - (1 if base_level == 0 else 0))
        max_x_tiles_in_original = 2 ** (level + base_level)
        max_y_tiles_in_original = 2 ** (level + base_level - 1)

        level_folder = os.path.join(output_folder, f"level_{max_x_tiles_in_original}_{max_y_tiles_in_original}")
        os.makedirs(level_folder, exist_ok=True)

        x_origin = int(max_x_tiles_in_original / x_tiles_on_base_level * base_index[0])
        y_origin = int(max_y_tiles_in_original / y_tiles_on_base_level * base_index[1])

        for x_index in range(max_x_tiles):
            for y_index in range(max_y_tiles):
                tile_width = image_width // max_x_tiles
                tile_height = image_height // max_y_tiles
                left, top = x_index * tile_width, y_index * tile_height
                tile = image[top:top + tile_height, left:left + tile_width]
                tile = cv.resize(tile, (target_tile_size, target_tile_size), interpolation=cv.INTER_LINEAR)

                filename = (
                    f"{output_file_name}"
                    f"_{x_origin + x_index}_{y_origin + y_index}"
                    f"_{max_x_tiles_in_original}_{max_y_tiles_in_original}"
                    f"_{original_width}_{original_height}_{target_tile_size}.png"
                )
                cv.imwrite(os.path.join(level_folder, filename), tile)


def process_single_file_texture(config):
    input_path = config['input_path']
    if not os.path.exists(input_path):
        print(f"  [SKIP] File not found: {input_path}")
        return
    generate_tiles(
        input_path,
        config['output_folder'],
        config['output_file_name'],
        config['initial_level'],
        config['max_level'],
        config['base_level'],
    )


def process_tiled_texture(config):
    files = sorted(glob.glob(config['input_pattern']))
    if not files:
        print(f"  [SKIP] No files matched: {config['input_pattern']}")
        return
    for file in files:
        tile_key = os.path.basename(file).split('_')[4]
        if tile_key not in config['tile_indices']:
            print(f"  [WARN] Unrecognised tile key '{tile_key}' in '{file}', skipping.")
            continue
        generate_tiles(
            file,
            config['output_folder'],
            config['output_file_name'],
            config['initial_level'],
            config['max_level'],
            config['base_level'],
            base_index=config['tile_indices'][tile_key],
        )


def process_texture(config):
    if 'input_path' in config:
        process_single_file_texture(config)
    else:
        process_tiled_texture(config)


def main():
    for config in TEXTURES:
        print(f"Generating {config['name']} tiles...")
        process_texture(config)
        print(f"  Done.")


if __name__ == "__main__":
    main()
