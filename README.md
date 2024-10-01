# Weather cli

The "weather_cli" uses curl and cJSON to make the api requests, and parse / print the json data. 
The api that is being used is weatherstack. To use the programm you need a weatherstack api key.
The cJSON library is in the repository so you only need curl and an api key.  


## Dependencies

You need (curl)[https://curl.se] you can either (download)[https://curl.se/download.html] 
it from the official website, or the build system ((meson)[https://mesonbuild.com/index.html]) will download it for you since libcurl has a (meson wrapDB package)[https://mesonbuild.com/Wrapdb-projects.html].

Then you you need your Weatherstack api key, which you can get with a Weatherstack account. You can sign up here:  
https://weatherstack.com/signup/free  

For meson you **need python**. You can find installation methods for meson (here)[https://mesonbuild.com/Getting-meson.html].

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

### The API Key

Before using ```meson setup **``` you have to create the `"WEATHERSTACK_API_KEY.env"` 
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

> [!Important]
> On MacOS you might need to execute both commands with sudo.
> Also when running the binary sudo ./weather_cli Berlin

It might not let you compile, and when running without sudo the .json file 
and .png might not be created.
