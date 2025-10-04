## Brew

common cmds
```
brew upgrade
brew autoremove
brew cleanup --prune=all -s
rm -rf "$(brew --cache)"
```

install
```
# utils
brew install wget ncdu

# dev
brew install cmake ninja
```

package sizes
```
du -h -d 1 /usr/local/Cellar/ | sort -h
```

other
```
brew leaves
brew deps --tree qemu
```
