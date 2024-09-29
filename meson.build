project('weather_cli', 'c')

caca_deps = [dependency('caca')]
curL_deps = [dependency('libcurl')]

inc_dir = include_directories('include')

lib = library('cJSON', 'src/cJSON.c')

# copy the resources into the build directory

# SOMEHOW COPY THE KEY INTO builddir/src/resources) or change the code so that all the resources are in the same folder as the exe
resource_files = files('src/resources/WEATHERSTACK_API_KEY.env')
foreach file : resource_files
  configure_file(input : file,
                  output: '@PLAINNAME@',
                  copy : true)
endforeach


executable(meson.project_name(), ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c'], 
  dependencies: [caca_deps, curL_deps],
  include_directories: inc_dir,
  link_with: lib,
  install: true)
