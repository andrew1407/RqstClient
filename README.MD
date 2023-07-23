# RqstClient Unreal Project

## Description

The project made with Unreal Engine 5.2 as a showcase for client connection strategies (HTTP, WS, UDP, TCP) **[plugin](https://github.com/Andrew1407/ClientConnectionStrategies)**. It shows both UI and gameplay implementations of the strategies. 

Address options (port & host) can be passed via input text boxes or command-line arguments (**=host=** and **-port=**) for a .exe launch file.

## Plugins (Submodules)

All the necessary submodules listed in [.gitmodules](.gitmodules). The main submodule is an unreal plugin - ClientConnectionStrategies which should be present in **[Plugins](./Plugins/)** dir. To set up submodules initialize them with git command:

```bash
git submodule init
```

And update them:

```bash
git submodule update
```

## Scripts

The project contains three .bat scripts for Windows platforms for a process automation in the **[ScriptsWin](./ScriptsWin)** folder. All main configurable parameters are present in teh **[config.bat](./ScriptsWin/config.bat)** script.

1. To run project compilation use **[run.bat](./ScriptsWin/run.bat)**:

```batch
.\ScriptsWin\run.bat <mode>
```

or (from the ScriptsWin dir):

```batch
.\run.bat <mode>
```

It can take one optional argument for unreal run mode. By default it runs editor dev. build.

2. To run tests use **[test.bat](./ScriptsWin/test.bat)**:

```batch
.\ScriptsWin\test.bat <testname> <skipbuild>
```

or (from the ScriptsWin dir):

```batch
.\test.bat <testname> <skipbuild>
```

It can take one optional argument for unreal run mode:
+ Test context name; by default runs all; besides exact test name the script argument takes **"projectonly"** option to run RqstClient tests only (this tests are contained in the **[tests](./Source/RqstClient/Private/Tests/)** dir);
+ Skip build flag; by default accomplishes the build; by passing **"skipbuild"** trigger test only (without build) otherwise the script runs build before tests; 

3. To build game use **[build_game.bat](./ScriptsWin/build_game.bat)**:

```batch
.\ScriptsWin\build_game.bat <mode>
```

or (from the ScriptsWin dir):

```batch
.\build_game.bat <mode>
```

An optional flag here is a build mode which is set as **"Shipping"** by default; can be defined as **"Development"** as well.
