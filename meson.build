project('weather_cli', 'c')

caca_deps = [dependency('caca')]
curL_deps = [dependency('libcurl')]

inc_dir = include_directories('include')

lib = library('cJSON', 'src/cJSON.c')

# Define the source and destination paths
src_resources = join_paths(meson.source_root(),  'src', 'resources', '')
build_resources = join_paths(meson.build_root(), 'src', 'resources', '')

# Create a custom target to copy the resources directory
copy_resources = custom_target(
  'copy-resources',
  command: [
    'mkdir', '-p', join_paths(meson.build_root(), 'src'),  # Ensure builddir/src exists
    '&&', 'cp', '-r', src_resources, build_resources  # Copy the resources folder
  ],
  output: ['resources_folder'],  # Virtual name for the output
  build_by_default: true  # Ensure this runs during the build
)

# Create the executable
executable(meson.project_name(), ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c', 'src/terminal_display_picture.c'], 
  dependencies: [caca_deps, curL_deps],
  include_directories: inc_dir,
  link_with: lib,
  install: true)

