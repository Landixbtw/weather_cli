# Weather cli

This is my 3rd attempt at a final project, and I think I found the balance this time, between having fun but it also not being too hard.
my first attempt was trying to write a small operating system with c. But once I figured out that in freestanding I would have to implement libc myself, I put that project on hold.  
I was / am pretty sure I would not have been able to make something working / something that I could present I about 7 Months. So I set my eye on making a small game with raylib and c. Sadly this burned when I was 70% finished since apprently playing games is just more fun then making them...  
I kinda wanted to do something that seemed simple in e.g python, but would still be a challenge in c. And since curl is a library that you encounter often, for example when downloading something. I had the idea to make this weather cli.

## Dependencies

### Curl
You need [curl](https://curl.se) you can either [download](https://curl.se/download.html) it from the official website, or the build system ([meson](https://mesonbuild.com/index.html)) will download it for you since libcurl has a [meson wrapDB package](https://mesonbuild.com/Wrapdb-projects.html).

### Meson build system
For meson you **need python**. You can find installation methods for meson [here](https://mesonbuild.com/Getting-meson.html).

### MacOs 

```sh
brew install pkg-config 
brew install libcaca
```

### Arch Linux
```sh
pacman -S meson
pacman -S pkgconf
yay -S libcaca
```

Building this has only been tested on.  
```sh
Linux tux 6.10.10-arch1-1 #1 SMP PREEMPT_DYNAMIC Thu, 12 Sep 2024 17:21:02 +0000 x86_64 GNU/Linux
---
```

### Weatherstack API Key

Last but not least you need your Weatherstack API key, which you can get with a Weatherstack account.  
You can sign up here: https://weatherstack.com/signup/free  
(With the free api you get 250 request per month, for 10$ a month you get 50,000.)  

**Before** using ```meson setup builddir``` you have to create the `"WEATHERSTACK_API_KEY.env"` 
file in `src/resources` (if the folder does not exist, create it) and paste your api Key.
Also use the correct format. ↓


> [!NOTE]
> CORRECT FORMAT: YOUR_ACCESS_KEY  
> WRONG FORMAT: access_key=YOUR_ACCESS_KEY  

Altough the api error message might say different, use the format that is here CORRECT.

### Building the binary

Building is as easy as.  
```sh
meson setup builddir

cd builddir

meson compile
```

### Installing the cli
If you want to install the cli to your machine to be able to use it everywhere 
and not just in the build directory. You can also set a custom path to install the programm. 
If you so, I will not be able help you troubleshoot.
You can uncomment the following line in the meson.build file.

```build

executable(
  meson.project_name(), 
  ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c', 'src/terminal_display_picture.c'],
  dependencies: [curL_deps],
  include_directories: inc_dir,
  link_with: lib,
  build_by_default: true,
 -->  # install: true <--
 --> # install_dir: 'your/destination/path' <--
)
```

This will install the cli to the default path. You can also set a custom path.
If you want to read more about meson install you can read here [here](https://mesonbuild.com/Installing.html).

###  Using the weather cli

After building / installing the programm, you can just execute ./weather_cli 
with no options to get a usage screen. That looks something like this.

``` 
➜  builddir git:(main) ./weather_cli
Usage: ./weather_cli <city>
Example: ./weather_cli New+York
```

> [!Important]
> On MacOS you might need to execute both commands with sudo.
> Also when running the binary sudo ./weather_cli Berlin.
> It might not let you compile, and when running without sudo the .json 
> and .png files might not be created.


### How it works

The "weather_cli" uses curl and cJSON to make the api requests, and parse / print the json data. 
The api that is being used is weatherstack. To use the programm you need a weatherstack api key.
The cJSON library is in the repository so you only need curl and an api key.  


blah blah, api request, string manipulation, terminal and image viewer detection


TODO:
- image to ascii
