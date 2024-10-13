# You can change the name of the cli by replacing 'main' with 'your desired name'
project('main', 'c')

# caca_deps = [dependency('caca')]
curL_deps = [dependency('libcurl')]

inc_dir = include_directories('include')

lib = library('cJSON', 'src/cJSON.c')

# Define the source and destination paths for img2ascii
img2ascii_source_dir = join_paths(meson.source_root(), 'img2ascii')
img2ascii_build_dir = join_paths(meson.build_root(), 'img2ascii')

# Check in timg is installed if the program is not found, this wont setup the builddir
find_program('timg', required: true)

# Custom target to copy img2ascii to the build directory
copy_img2ascii = custom_target(
  'copy-img2ascii',
  command: [
    'sh', '-c',
    'mkdir -p "@0@" && if [ "$(ls -A @1@)" ]; then cp -r "@1@"/* "@0@"; fi'.format(img2ascii_build_dir, img2ascii_source_dir)
  ],
  output: ['img2ascii'],
  build_by_default: true
)

# Custom target to build img2ascii using make
build_img2ascii = custom_target(
  'build-img2ascii',
  command: ['make', '-C', img2ascii_build_dir],
  depends: [copy_img2ascii],
  output: ['img2ascii_executable'],
  build_by_default: true
)

# Define the source and destination paths for the resources
src_resources = join_paths(meson.source_root(), 'src', 'resources')
build_resources = join_paths(meson.build_root(), 'src', 'resources')

# Custom target to copy the resources directory if it exists
copy_resources = run_command('test', '-d', src_resources, check: false).returncode() == 0 ? custom_target(
  'copy-resources',
  command: [
    'sh', '-c', 
    'mkdir -p "@0@" && if [ "$(ls -A @1@)" ]; then cp -r "@1@"/* "@0@"; fi'.format(build_resources, src_resources)
  ],
  output: ['resources_folder'],
  build_by_default: true
) : []

# Create the executable for the main project (weather_cli)
executable(
  meson.project_name(), 
  ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c', 'src/terminal_display_picture.c'],
  dependencies: [curL_deps],
  include_directories: inc_dir,
  link_with: lib,
  build_by_default: true,
  # install: true
  # install_dir: 'your/destination/path'
)

