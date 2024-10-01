project('weather_cli', 'c')

caca_deps = [dependency('caca')]
curL_deps = [dependency('libcurl')]

inc_dir = include_directories('include')

lib = library('cJSON', 'src/cJSON.c')

# Define the source and destination paths
src_resources = join_paths(meson.source_root(),  'src', 'resources', )
build_resources = join_paths(meson.build_root(), 'src', 'resources', )

# BUG: There are two resource folders being created, builddir/src/resources builddir/src/resources/resources
# BUG: Also the .env file is in both but the programm cant find it

# Create a custom target to copy the resources directory
src_resources = join_paths(meson.source_root(), 'src', 'resources')
build_resources = join_paths(meson.build_root(), 'src', 'resources')

copy_resources = run_command('test', '-d', src_resources, check: false).returncode() == 0 ? custom_target(
  'copy-resources',
  command: [
    'sh', '-c', 
    'mkdir -p "@0@" && if [ "$(ls -A @1@)" ]; then cp -r "@1@"/* "@0@"; fi'.format(build_resources, src_resources)
  ],
  output: ['resources_folder'],
  build_by_default: true
) : []

# Create the executable
executable(meson.project_name(), ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c', 'src/terminal_display_picture.c'], 
  dependencies: [caca_deps, curL_deps],
  include_directories: inc_dir,
  link_with: lib,
  install: true)

