# alfred-sound-switcher
Alfred script filter for selecting audio devices

Forum post: https://www.alfredforum.com/topic/17556-fast-sound-inputoutput-selector/

## Compiling from source
1. Download source code from releases
2. Navigate to the source directory and run:
```
clang++ -Wall -std=c++20 filter.cpp -o sound_filter
```

## Credits
- [deweller/switchaudio-osx](https://github.com/deweller/switchaudio-osx)
- [kazuho/picojson](https://github.com/kazuho/picojson)
- [julienXX/terminal-notifier](https://github.com/julienXX/terminal-notifier)
