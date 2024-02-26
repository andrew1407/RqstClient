# RqstClient Unreal Project

## Description

The project made with Unreal Engine 5.2 as a showcase for client connection strategies (HTTP, WS, UDP, TCP) **[plugin](https://github.com/Andrew1407/ClientConnectionStrategies)**. It shows both UI and gameplay implementations of the strategies. 

Address options (port & host) can be passed via input text boxes or command-line arguments (**-host=** and **-port=**) for .exe launch file.

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

## Usage demonstartion

### 1. Client form

Default form state:

![Default form](./Doc/Resources/ClientForm/default.png)

Client selector dropdown:

![Client selector](./Doc/Resources/ClientForm/client-selector.png)

HTTP client usage:

![http client](./Doc/Resources/ClientForm/http.png)

WS client usage:

![ws client](./Doc/Resources/ClientForm/ws.png)

UDP client usage:

![udp client](./Doc/Resources/ClientForm/udp.png)

TCP client usage:

![tcp client](./Doc/Resources/ClientForm/tcp.png)

Form is closed and user is able to switch a [playground level](#2-clients-playground):

![Closed form](./Doc/Resources/ClientForm/form-closed.png)

### 2. Clients playground

Defalt spawn ay the start:

![Playground default](./Doc/Resources/ClientsPlayground/default.png)

A trigger to make a request via chosen cleint:

![Default request](./Doc/Resources/ClientsPlayground/default-request.png)

A trigger to make a request via chosen cleint (protocol):

![Default request](./Doc/Resources/ClientsPlayground/default-request.png)

Choose HTTP client:

![http client](./Doc/Resources/ClientsPlayground/http-trigger.png)

Make a request using HTTP client:

![http request](./Doc/Resources/ClientsPlayground/http-request.png)

Choose WS client:

![ws client](./Doc/Resources/ClientsPlayground/ws-trigger.png)

Make a request using WS client:

![ws request](./Doc/Resources/ClientsPlayground/ws-request.png)

Choose UDP client:

![udp client](./Doc/Resources/ClientsPlayground/udp-trigger.png)

Make a request using UDP client:

![ucp request](./Doc/Resources/ClientsPlayground/udp-request.png)

Choose TCP client:

![TCP client](./Doc/Resources/ClientsPlayground/tcp-trigger.png)

Make a request using TCP client:

![tcp request](./Doc/Resources/ClientsPlayground/tcp-request.png)

Clear a chosen client:

![Clear client](./Doc/Resources/ClientsPlayground/clear-trigger.png)

Client connection error:

![Connection error](./Doc/Resources/ClientsPlayground/error-trigger.png)

Client request error:

![Request error](./Doc/Resources/ClientsPlayground/error-request.png)

Go to the [client form](#1-client-form) level:

![Go to the client form level](./Doc/Resources/ClientsPlayground/client-form-portal.png)
