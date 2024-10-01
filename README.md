# Weather cli

This is my 3rd attempt at a final project, and I think I found the balance this time, between having fun but it also not being too hard.
my first attempt was trying to write a small operating system with c. But once I figured out that in freestanding I would have to implement libc myself, I put that project on hold.  
I was / am pretty sure I would not have been able to make something working / something that I could present I about 7 Months. So I set my eye on making a small game with raylib and c. Sadly this burned when I was 70% finished since apprently playing games is just more fun then making them...
I kinda wanted to do something that seemed simple in e.g python, but would still be a challenge in c. And since curl is a library that you encounter often, for example when downloading something. I had the idea to make this weather cli.

## Dependencies

### Curl
You need [curl](https://curl.se) you can either [download](https://curl.se/download.html) it from the official website, or the build system ([meson](https://mesonbuild.com/index.html)) will download it for you since libcurl has a [meson wrapDB package](https://mesonbuild.com/Wrapdb-projects.html).

### Weatherstack API key
Then you you need your Weatherstack api key, which you can get with a Weatherstack account.  
You can sign up here: https://weatherstack.com/signup/free  

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

### The API Key

Before using ```meson setup builddir``` you have to create the `"WEATHERSTACK_API_KEY.env"` 
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
> Also when running the binary sudo ./weather_cli Berlin.
> It might not let you compile, and when running without sudo the .json 
> and .png files might not be created.



###  Using the weather cli

bla blah

### How it works

The "weather_cli" uses curl and cJSON to make the api requests, and parse / print the json data. 
The api that is being used is weatherstack. To use the programm you need a weatherstack api key.
The cJSON library is in the repository so you only need curl and an api key.  


blah blah, api request, string manipulation, terminal and image viewer detection
