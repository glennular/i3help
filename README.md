i3help - keyboard binding help dialog
======================================
i3help is a simple popup dialog to display all of your configure key binding. By default it will load the bindsyms from the running i3wm instance, there is also the abiliy to pass in a config file.

As your i3 config will be consistently changes, you can bind this to $mod+? to remind yourself for your key bindings.

![Image](../main/img/i3help.gif?raw=true)
 *image provided by [u/EllaTheCat](https://www.reddit.com/user/EllaTheCat)*


Requirements
-----------

* GTK >= 3.0

Install
-------

```
autoreconf --force --install
./configure
make
make install
```

Running i3help
--------------

 `i3help` should be ran while i3 is running so it can access the config via i3-msg.

 * Pressing any key will exit out the popup

 * There are basic sizes set to try to make long command and config files show up. For long config files, you can use `<space>` to cycle through the pages.

 * For longer configs, sizing can be adjusted with the `-col` and `-maxrows` flags

 * Add to your i3/config:
```
bindsym $mod+question exec i3help
```

 Optional Arguments:
| Parameter                 | Default       | Description   |
| :------------------------ |:-------------:| :-------------|
| -f  --file 	       |	null           | the file name of a i3 config file to load
| -c  --col          | 2           | number of columns to draw in the dialog
| -r  --maxrows          | 50           | max number of rows to draw per a column
| -l  --maxtextlen          | 100           | max length of a column before the text is trimmed


TODOs
------
- [ ] Better parsing for complex commands
- [ ] add a `#i3help` tag to ignore or replace the actual line
       Ex: replace all $mod+1 to 9 with a single entry
- [ ] add ability to replace the command with user friendly text via comment
- [ ] sort options
- [ ] dynamic sizing based on screen size and (instead of hardcoded values)
