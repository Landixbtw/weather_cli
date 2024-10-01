The "weather_cli" uses curl and cJSON to make the api requests, and parse / print the json data. 
The api that is being used is weatherstack. To use the programm you need a weatherstack api key.
The cJSON library is in the repository so you only need curl and an api key.  

If you are on mac you might need to use sudo before ./weather_cli <city name> 
because if not the json file needed might not be created

The WEATHERSTACK_API_KEY.env file goes into src/
The format for the API Key file is   

CORRECT FORMAT: YOUR_ACCESS_KEY  
WRONG FORMAT: access_key=YOUR_ACCESS_KEY  

You need curl -> https://curl.se/download.html  
Weatherstack key -> https://weatherstack.com/signup/free  

This project uses mason build.   
To build. --> https://mesonbuild.com/Quick-guide.html  

# Dependencies

## MacOs 

```sh
brew install pkc-config libcaca
```

## Arch Linux
```sh
pacman -S meson
yay -S libcaca
```




Building this has only been tested on.  
```
Linux tux 6.10.10-arch1-1 #1 SMP PREEMPT_DYNAMIC Thu, 12 Sep 2024 17:21:02 +0000 x86_64 GNU/Linux
---
```

Building is as easy as.  
```sh
meson setup builddir

cd builddir

meson compile
```

