# Windows clipboard storage enhancement

Make Windows clipboard store text as stack/queue. Tested on Windows 10 2004 19041, Visual Studio 2019, Windows SDK 10.0.19041.0

## Compile

Open ClipStack.sln with Visual Studio, then press Ctrl + Shift + B. Uncomment line 3 of ClipStack.cpp if you want to use queue as clipboard storage.

## Added key binding  

- CTRL + V: Paste item in top of storage.  
- CTRL + Shift + V: Paste all item in storage.  
- CTRL + Shift + A: Clear all item in storage.  

## Trayicon

The icon shows current clipboard stack size and status. Right click trayicon to exit program.

## Screen shot  

Use stack as clipboard storage:

![paste](https://cdn.igayuka.moe/clip/stack.gif)

Paste all item in stack:

![paste_all](https://cdn.igayuka.moe/clip/stack_all_.gif)

Clear storage:

![clear](https://cdn.igayuka.moe/clip/clear.gif)