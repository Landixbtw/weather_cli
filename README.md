
<h1>
    <p align="center">
        🌥️Weather cli 
    </p>
</h1>

"weather_cli" is a command-line interface tool that allows the user to display the current weather conditions, and other information about a given city. The "weather_cli" sends
an API request and parses the content that is returned, to then display the output. 

## 🔗 Dependencies
#### Curl
You need [curl](https://curl.se) you can either [download](https://curl.se/download.html) it from the official website, or the build system ([meson](https://mesonbuild.com/index.html)) will download it for you since libcurl has a [meson wrapDB package](https://mesonbuild.com/Wrapdb-projects.html).

#### Meson build system
For meson you **need python**. You can find installation methods for meson [here](https://mesonbuild.com/Getting-meson.html).
You also **need [ninja](https://ninja-build.org/)**.

#### Terminal image viewer
The weather_cli uses [timg](https://github.com/hzeller/timg/) to display the images on the terminal. Meson will check if timg is installed, if not it will not setup the builddir.

#### cJSON
The [cJSON](https://github.com/DaveGamble/cJSON?tab=MIT-1-ov-file#readme) library is shipped within the project source code.

> [!NOTE]
> THERE IS NO MAC SUPPORT CURRENTLY I DONT THINK THIS WORKS FOR MAC RIGHT NOW.
#### macOS
```zsh
brew install pkg-config
brew install ninja
brew install meson
brew install timg
```
#### Arch Linux
```bash
pacman -S pkgconf ninja meson
yay -S timg
```

#### Weatherstack API Key

Last but not least you need your Weatherstack API key, which you can get with a Weatherstack account.
You can sign up here: https://weatherstack.com/signup/free
(With the free API you get 250 request per month, for 10$ a month you get 50,000.)

**Before** using ```meson setup builddir``` you have to create the `"WEATHERSTACK_API_KEY.env"`
file in `src/resources` (if the folder does not exist, create it) and paste your api key.
Also use the correct format. ↓

> [!NOTE]
> CORRECT FORMAT: YOUR_ACCESS_KEY  
> WRONG FORMAT: access_key=YOUR_ACCESS_KEY

```bash
mkdir -p src/resources
echo "your_access_key" > src/resources/WEATHERSTACK_API_KEY.env
```

## 🚀 Getting Started
###  Building the binary
As long as you have followed all the steps above.  
**For Linux** systems building should be as easy as.
```bash
meson setup builddir

cd builddir

meson compile
```

Building this has been tested on.
```bash
Linux tux 6.10.10-arch1-1 #1 SMP PREEMPT_DYNAMIC x86_64 GNU/Linux
Linux tux 6.11.3-arch1-1 #1 SMP PREEMPT_DYNAMIC x86_64 GNU/Linux
```

**For macOS** you might have to add a sudo before the compile command.

<!-- ### Installing the cli -->
<!-- > [!IMPORTANT] -->
<!-- > I DO NOT RECOMMEND IT. AND I WILL NOT HELP TROUBLESHOOT. -->
<!---->
<!-- If you still choose to install, I will not be able help you troubleshoot.   -->
<!-- I _might_ work on getting this ready, but no promises made.   -->
<!-- You can uncomment the following line in the ```meson.build``` file. -->
<!---->
<!-- ```build -->
<!-- executable( -->
<!--   meson.project_name(), -->
<!--   ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c', 'src/terminal_display_picture.c'], -->
<!--   dependencies: [curL_deps], -->
<!--   include_directories: inc_dir, -->
<!--   link_with: lib, -->
<!--   build_by_default: true, -->
<!--  -> install: true <- -->
<!--  -> install_dir: 'your/destination/path' <- -->
<!-- ) -->
<!-- ``` -->
<!-- ```bash -->
<!-- meson install -->
<!-- ``` -->
<!---->
<!-- This will install the cli to the default path. You can also set a custom path. -->
<!---->
<!-- If you want to read more about meson install you can read here [here](https://mesonbuild.com/Installing.html). -->
<!---->
###  Using the weather cli

After building the program, you can just execute ./weather_cli
with no options to get a usage screen. That looks something like this.

```
❯ ./weather_cli

Usage: ./weather_cli [city]

A command-line tool to display weather information about a city

For city names capitalization does not matter.
Washington or washington.
For cities that have a [space] in between you have to enter a plus.
New+York or new+york.
```

> [!Important]
> On macOS you might need to execute both commands with sudo.  
> sudo ./weather_cli Berlin.
> If you run the commands without sudo the .json and .png files might not be created.

## ⚙️ How it works

The ```src/main.c``` file handles, taking the user input, building the url with the [build_url()](https://github.com/Landixbtw/weather_cli/blob/main/src/main.c) function, with the city, and API key. It also handles the "umlaute" if the city name 
has them (For example München -> Muenchen). The weatherstack cannot handle the umlaute so this is necessary.
It takes the json data it gets to from the api and writes it into a json file so that it can be read and displayed to the console with the help of [cJSON](https://github.com/DaveGamble/cJSON). 
It does all the parsing and checking of the json data.

We first have to parse the json file with the [cJSON_Parse](https://github.com/DaveGamble/cJSON/blob/master/cJSON.c#L1195) function, to then be able to dissect it with the cJSON library, to display the parts we want. We read the json file with all the data, into a buffer and then into the function.
```c
while (fread(buffer, file_size , 1, temp_json_file)) {
    json = cJSON_Parse(buffer);
}
```
We can then access the data like this:
```c
location = cJSON_GetObjectItemCaseSensitive(json, "location");
if (location != NULL) {
    name = cJSON_GetObjectItemCaseSensitive(location, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL )) {
        fprintf(stdout, "\033[4mWeather report for:\033[0m ");
        fprintf(stdout, "%s\n", name->valuestring);
    }
}
```

The Object in this case "location", is a json object 
```json
"location":{"name":"New York","country":"United States of America","region":"New York","lat":"40.714","lon":"-74.006","timezone_id":"America\/New_York","localtime":"2024-10-19 18:51","localtime_epoch":1729363860,"utc_offset":"-4.0"}
```

Which we look for in the json file if we find it, we then look for the "name", 
if this is a string, and the valuestring is not NULL we display it on the terminal.

### Converting characters

I am German, and in the German language there are so called "[umlaute](https://en.wikipedia.org/wiki/Umlaut_%28linguistics%29)" these are letters with the small dots over them ö ä ü. Some city names most notably München have these. I had to discover that the weatherstack API does not support these umlaute.
So I thought about how I can detect them and convert them to oe ae ue, which means the same but is a format that the weatherstack api can read and process.

The [transliterate_umlaut]() function just checks for the first four bytes which are always the same, in this case "0xC3", if they match the next four are checked. 
They are then replaced with ae for ä ue for ü and oe for ö.  
München -> Muenchen  
Köln -> Koeln  
Märkisch Buchholz -> Maerkisch Buchholz  

This is the [UTF8-Character chart](https://www.utf8-zeichentabelle.de/unicode-utf8-table.pl?start=128&utf8=0x)

### Getting and downloading the weather picture  <!-- (terminal_display_picture.c) -->

The API provides a direct link to a png, that we download with curl. I implemented a system that checks if the png already exists,
if it does the program displays the picture that is already on your machine, this is possible because the pictures get the same names every time. 
With the [get_filename()](https://github.com/Landixbtw/weather_cli/blob/main/src/terminal_display_picture.c#L241) function we take the url to the picture and deleting everything before 
the last /. This then gives us /filename.png, 
we then delete the / and there we have the filename. The [file_exists()](https://github.com/Landixbtw/weather_cli/blob/main/src/terminal_display_picture.c#L241)
function checks if the file exists, if not then the download part is triggered.

### Displaying the picture on the command line

Displaying the image on the command line is as easy as checking if the user has one of the
"supported" terminal image viewers installed on their system and executing a command.
With ```popen()``` we can pipe the output directly onto the terminal.


.## Determining the terminal emulator protocol <!-- (terminal_support.c) -->
Different terminal emulators, use different protocols, for example:
- xterm
- xterm-256color
- xterm-kitty
- iTerm.app
- mlterm  

Some are able to display pictures, and some are not, so in the ```terminal_support.c``` file,
we are checking which protocol your terminal emulator is using.
This way we can display different things depending on the outcome, on some terminals you will see a small picture with what the weather currently looks like.
And for some you will just get a small text telling you that your terminal emulator cannot display pictures, <del>or you may get ascii art</del>.
If you want a terminal that can display pictures, you should choose one with the following protocols:   
xterm-256color, xterm-kitty or iTerm.app.
Two terminals with such a protocol would be:
- [wezterm](https://wezfurlong.org/wezterm/index.html)
- [kitty](https://sw.kovidgoyal.net/kitty/)
- [iterm2](https://iterm2.com/downloads.html)
  
If you want to know more about this topic there is a post on [unix.stackexchange](https://unix.stackexchange.com/questions/5800/what-protocol-standard-is-used-by-terminals) about 
the topic.
### 🖌️ Design Choices

The only design choice I really had was, where to put the image, if displayed. It used to be at the right, next to the text. But there was no good way of determining the string 
length, of the output and still displaying the picture where I wanted it, since the text that was too long was in the bottom third of the picture and I could not get the string 
length and make the adjustment to the image placement. It does look good at the top, but the problem is that it takes forever to load / download. I might get some multithreading 
going and drawing the picture piece by piece. 


<!-- TODO:
    - Make more code comments ?
    - https://stackoverflow.com/questions/5134891/how-do-i-use-valgrind-to-find-memory-leaks
    - If not weatherstack api key file detected, ask user for api key, and make file echo the key, into the file
--> 

## 📃 License
This project uses the MIT-License it can be found in the LICENSE file
