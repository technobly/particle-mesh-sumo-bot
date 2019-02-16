# Particle Mesh Sumo Bot

- RC Receiver Mode (Control with a [modified PartiBadge!](https://github.com/technobly/parti-badge/tree/sumo-bot))
- TODO: Add Logo Turtle OS control system

## Local Compile

```
$ git checkout v0.9.0-rc.3
$ cd firmware/main
// enter DFU mode
firmware/main $ make clean all -s APPDIR=~/code/debugging/technobly/particle-mesh-sumo-bot/firmware PLATFORM=xenon COMPILE_LTO=n program-dfu
```

![particle mesh sumo bot](https://raw.github.com/technobly/particle-mesh-sumo-bot/master/images/mesh-sumo-bot.png)