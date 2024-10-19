# Weather cli

https://cs50.harvard.edu/x/2024/project/
/*
*   Video Demo: <yt url>
*       3min video maybe pp / and live demo
<!-- *   Description of the project -->
<!-- *   Talk about all the files, and what they, do  -->
*   Did I debate certain design choices, why did I make them
*/

This command line tool is actually my third attempt at a final project for cs50x 2024. My first attempt was trying to create a small operating system, but it got clear quite fast
that this was way out of scope for me. Altough this is not my final project, I will probably pick this up again when I know c better. Between working this low level and
implementing my own libc this would have been too hard, and taken too much time. My second attempt for a final project was remaking spaceinvaders with c and raylib.
But there where I had some big issues and I could not get around them, even after weeks of trying. Finally I had the idea for this weather_cli, it seemed hard enough but definitly
possible to do in time.

## Dependencies
### Curl
You need [curl](https://curl.se) you can either [download](https://curl.se/download.html) it from the official website, or the build system ([meson](https://mesonbuild.com/index.html)) will download it for you since libcurl has a [meson wrapDB package](https://mesonbuild.com/Wrapdb-projects.html).

### Meson build system
For meson you **need python**. You can find installation methods for meson [here](https://mesonbuild.com/Getting-meson.html).
You also **need [ninja](https://ninja-build.org/)**.

### timg
The weather_cli uses [timg](https://github.com/hzeller/timg/) to display the images on the terminal. Meson will check if timg is installed, if not it will not setup the builddir.

> [!NOTE]
> THERE IS NO MAC SUPPORT CURRENTLY I DONT THINK THIS WORKS FOR MAC RIGHT NOW.
### MacOs
```sh
brew install pkg-config
brew install ninja
brew install meson
brew install timg
```

### Arch Linux
```sh
pacman -S pkgconf
pacman -S ninja
pacman -S meson
yay -S timg
```

### Weatherstack API Key

Last but not least you need your Weatherstack API key, which you can get with a Weatherstack account.
You can sign up here: https://weatherstack.com/signup/free
(With the free API you get 250 request per month, for 10$ a month you get 50,000.)

**Before** using ```meson setup builddir``` you have to create the `"WEATHERSTACK_API_KEY.env"`
file in `src/resources` (if the folder does not exist, create it) and paste your api key.
Also use the correct format. ↓

> [!NOTE]
> CORRECT FORMAT: YOUR_ACCESS_KEY
> WRONG FORMAT: access_key=YOUR_ACCESS_KEY

Altough the error message might say different, use the format that is here CORRECT.

## Building the binary
Building is as easy as.
```sh
meson setup builddir

cd builddir

meson compile
```

Building this has been tested on.
```sh
Linux tux 6.10.10-arch1-1 #1 SMP PREEMPT_DYNAMIC x86_64 GNU/Linux
Linux tux 6.11.3-arch1-1 #1 SMP PREEMPT_DYNAMIC x86_64 GNU/Linux
```

## Installing the cli
> [!IMPORTANT]
> I DO NOT RECOMMEND IT. AND I WILL NOT HELP TROUBLESHOOT.

If you so, I will not be able help you troubleshoot.
You can uncomment the following line in the ```meson.build``` file.

```build
executable(
  meson.project_name(),
  ['src/main.c', 'src/terminal_support.c', 'src/cJSON.c', 'src/image_to_ascii.c', 'src/terminal_display_picture.c'],
  dependencies: [curL_deps],
  include_directories: inc_dir,
  link_with: lib,
  build_by_default: true,
 --> install: true <--
 --> install_dir: 'your/destination/path' <--
)
```
```sh
meson install
```

This will install the cli to the default path. You can also set a custom path.

If you want to read more about meson install you can read here [here](https://mesonbuild.com/Installing.html).

##  Using the weather cli

After building / installing the programm, you can just execute ./weather_cli
with no options to get a usage screen. That looks something like this.

```
➜  builddir git:(main) ./weather_cli
Usage: ./weather_cli <city>
Example: ./weather_cli New+York
```

> [!Important]
> On MacOS you might need to execute both commands with sudo.
> Also when running the binary
> sudo ./weather_cli Berlin.
> It might not let you compile, and when running without sudo the .json
> and .png files might not be created.


## How it works

The ```src/main.c``` file handles thing like the input, building the url, "transforming" the umlaute.
the programm uses curl to make the api requests, to the weatherstack api.
It receives a json_data.json file, that is then being parsed by the cJSON library.
The json data is then being split into the information that we want to display on the command line.

For example like this:

```c
api_success = cJSON_GetObjectItemCaseSensitive(json, "success");
    if (cJSON_IsFalse(api_success)) {
        api_error = cJSON_GetObjectItemCaseSensitive(json, "error");
        if (api_error != NULL) {
            api_error_code = cJSON_GetObjectItemCaseSensitive(api_error, "code");
            api_error_info = cJSON_GetObjectItemCaseSensitive(api_error, "info");
            if (cJSON_IsNumber(api_error_code) && (api_error_code->valueint) && cJSON_IsString(api_error_info) && (api_error_info->valuestring)) {
                fprintf(stderr, "API ERROR RESPONSE CODE: %i %s\n", api_error_code->valueint, api_error_info->valuestring);
            } else {
                fprintf(stderr, "Uknown error code.\n");
            }
		}
	}
```

The "json" is a cJSON pointer (```cJSON *json```) that has the content of the json\_data.json file that we received.
That is stored in a file called temp\_json\_file that temporarily hold the content.

```c
while (fread(buffer, file_size , 1, temp_json_file)) {
    json = cJSON_Parse(buffer);
}
```

## Converting characters

I am German, and in the german language there are so called "[umlaute](https://en.wikipedia.org/wiki/Umlaut_%28linguistics%29)" these are letters with the small dots over them ö ä ü. Some city names most notably München have these. I had to discover that the weatherstack API does not support these umlaute.
So I thought about how I can detect them and convert them to oe ae ue, which means the same but is a format that the weatherstack api can read and process.
So München becomes Muenchen. I tried over days, with many different approaches, but could not figure out. You can still find my inital
attempts on trying to convert the characters, at the bottom of the ```main.c``` file. Eventually I caved and asked [claude.ai](https://www.claude.ai/new).

- Files in the project what do they do

The ```src/main.c``` file, handles the user input, taking the city name, passing it to the 'transliterate_umlaut' function, to change umlaute, into something the weatherstack api can read.
The user input
The api request for displaying information like the city, temperature, windspeed, humidity, ...
The building of the url with the build_url() function, putting together the city for the request, and the api key.
Transforming ö ä ü into oe ae ue

## Getting and downloading the weather picture

The API this is all built on is the [weatherstack API](https://www.weatherstack.com).
It provides us with all the information needed, and even provides a link to a picture.
The picture that is being displayed / downloaded, is dependant on the weather of the
city we are trying to look up. The picture is then being downloaded with curl.


## Displaying the picture on the command line

Displaying the image on the command line is as easy as checking if the user has one of the
"supported" terminal image viewers installed on their system and executing a command.
With ```popen()``` we can pipe the output directly onto the terminal.


## Determining the terminal emulator protocol

Different terminal emulators, use different protocols, for example:

- xterm
- xterm-256color
- xterm-kitty
- iTerm.app
- mlterm

Some are able to display pictures, and some are not, so in the ```terminal_support.c``` file,
we are checking which protocol your terminal emulator is using.
This way we can display different things depending on the outcome, on some terminals you will see a small picture with what the weather currently looks like.
And for some you will just get a small text telling you that your terminal emulator cannot display pictures, or you may get ascii art.
If you want a terminal that can display pictures, you should choose one with the following protocols: xterm-256color, xterm-kitty or iTerm.app.
Some would be:
- [wezterm](https://wezfurlong.org/wezterm/index.html)
- [kitty](https://sw.kovidgoyal.net/kitty/)




TODO:
-- image to ascii doesnt work yet --
- Downloading the picture takes a long time, maybe cache it ? Or put it somewhere that is not the top.
- Readme is not long / detailed enough yet.
- Go more into detail with term emulator protocol ?
- Maybe find a way to actually cache the pictures, shouldn't be that much since you would only look up the places around you
- Make more code comments ?
- https://stackoverflow.com/questions/5134891/how-do-i-use-valgrind-to-find-memory-leaks
- If not weatherstack api key file detected, ask user for api key, and make file echo the key, into the file
