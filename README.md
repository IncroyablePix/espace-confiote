# Espace Confiote - Nobodyth person shooter (in space)

Espace Confiote is a simple game in which the gameplay consists of the movement of your space vessel.
You must explore your surroundings and eliminate the space monsters that appear by pre-specifying the behaviour of your 
vessel through an algorithm coded by yourself.

## Features

- **Multi-platform**: The game can run on Desktop (Windows, Linux, Mac) and Web
- **Dual AI Backends**: Depending on your build chain, you can implement your player AI with plain C (or C++) for the desktop version, or Python for the web version.
- **Lightweight**: Relatively lightweight due to building with RayLib as a sole dependency, ensuring minimal overhead.
- **Open Source**: Fully open source, allowing you to modify and extend the game as you see fit.
- **Simple to build**: Uses xmake for easy building and dependency management.

## Building and Running

This project uses xmake as the build system and dependency manager.
[XMake](https://xmake.io/) is a cross-platform build utility based on Lua that has the specificity not to be crappy like CMake.

### XMake digression

XMake is not yet another tool, it is a complete ecosystem that allows you to manage your builds, dependencies, packaging, testing, and more.
It also allows you to use other tools like CMake, Make, Ninja, etc. as backends if needed, or even to import dependencies from other package managers like vcpkg, conan, etc.
It has also the specificity to be very easy to use and to learn, with a simple and intuitive syntax, unlike CMake.

### Building 

To actually build the project, you just need to have xmake and a C compiler installed on your system;
You can then just build the project with the following commands:

```bash
xmake
```

That is pretty much it, since XMake will take care of downloading and building RayLib as a dependency automatically.

If you want to build for web, you just have to specify it like this:

```bash
xmake f -p wasm
```

...against which you will need to have Emscripten installed and configured on your system.

You can then run the project with:

```bash
xmake run espace-confiote
```

## Playing

**Python AI:**
- The game will automatically load `player_ai.py` from the project root
- If the Python script fails to load, the game will display a warning but continue running (without AI)

**C AI:**
- Edit `src/player_ai_backend/player_a_ci.c` to implement your AI
- No Python dependency required

### Web Build

**Prerequisites:**
- Emscripten SDK installed and configured
- xmake configured to use Emscripten toolchain

**Installing Emscripten:**

1. Download and install Emscripten SDK:
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   ```

2. On Windows (PowerShell):
   ```powershell
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   .\emsdk install latest
   .\emsdk activate latest
   .\emsdk_env.bat
   ```

3. xmake will automatically detect Emscripten if installed in common locations:
   - `%USERPROFILE%\AppData\Local\emsdk` (Windows)
   - `~/.emsdk` (Linux/Mac)
   - Or set `EMSDK` environment variable
   
   If auto-detection fails, you can specify the path manually:
   ```bash
   xmake config --platform=web --emsdk_path=C:\Users\jaja\AppData\Local\emsdk
   ```

**Build for Web:**
```bash
# Configure for web platform
xmake config --platform=web

# Build
xmake

# The output will be in build/web/
# Serve the files with a web server (required for CORS)
# Example with Python:
cd build/web
python -m http.server 8000

# Then open http://localhost:8000/index.html in your browser
```

**Note:** If you get errors about `emscripten.h` not found, make sure:
1. Emscripten is installed and activated
2. The Emscripten environment is sourced (run `emsdk_env.bat` on Windows or `source emsdk_env.sh` on Linux/Mac)
3. xmake can find the `emcc` compiler

**Web Features:**
- Game runs in the browser using WebAssembly
- Python AI code can be written directly in the browser UI
- Uses Pyodide (Python compiled to WebAssembly) for Python execution
- No server-side Python installation needed

## How to Play

You can implement your player AI in either **C** or **Python**:

### Python AI (Recommended)

Edit `player_ai.py` in the project root and implement the `on_player_update(context)` function. The game will automatically load and use your Python script.

**Python API:**
- `context['playerPosition']` - dict with 'x' and 'y' keys
- `context['playerVelocity']` - dict with 'x' and 'y' keys
- `context['playerRotation']` - rotation in degrees
- `context['visibleMonsters']` - list of monster dicts
- `context['visibleMonsterCount']` - number of visible monsters
- `context['deltaTime']` - time since last frame

**Control Functions:**
- `player_ai_c.shoot(direction)` - shoot (0=FORWARD, 1=LEFT, 2=RIGHT)
- `player_ai_c.set_thrusters(leftPower, rightPower)` - set thrusters (-1.0 to 1.0)

See `player_ai.py` for a complete example implementation.

### C AI

Edit `src/player_ai_backend/player_a_ci.c` and implement the `OnPlayerUpdate()` function. Your AI receives limited information and must make decisions based on visible monsters only.

### Player API

**Context Information (PlayerContext):**
- `context.playerPosition` - Your ship's current {x, y} position
- `context.playerVelocity` - Your ship's current {x, y} velocity
- `context.playerRotation` - Your ship's rotation in degrees (0° = right)
- `context.visibleMonsters` - Array of visible monster information
- `context.visibleMonsterCount` - Number of visible monsters
- `context.deltaTime` - Time since last frame in seconds

**Available Actions:**
- `PlayerShoot(direction)` - Fire a projectile (FORWARD, LEFT, or RIGHT)
- `PlayerSetThrusters(leftPower, rightPower)` - Control movement (-1.0 to 1.0)
  - Both same = move forward/backward
  - Different = turn (right > left = turn right)

**Monster Information:**
Each visible monster provides:
- `position` - {x, y} coordinates
- `size` - Monster size
- `health` - Health value (0.0 to 1.0)
- `invincible` - Whether monster is currently invincible

## Project Structure

```
space-jam/
├── src/                    # Source code
│   ├── main.c             # Main game loop
│   ├── player.c           # Player logic
│   ├── monster.c          # Monster AI and logic
│   ├── ui.c               # UI rendering
│   └── player_ai_backend/ # AI implementations
│       ├── player_ai_python.c  # Desktop Python AI
│       ├── player_ai_web.c     # Web Python AI (Pyodide)
│       └── player_a_ci.c       # C AI
├── resources/             # Game assets
├── web/                   # Web build assets
│   └── index.html        # Web UI with code editor
├── player_ai.py          # Default Python AI script
├── xmake.lua            # Build configuration
└── README.md            # This file
```

## Web Build Details

The web build uses:
- **Emscripten**: Compiles C to WebAssembly
- **Pyodide**: Python runtime compiled to WebAssembly
- **JavaScript Bridge**: Connects C game code with Python execution

Users can write Python code directly in the browser's text editor and load it dynamically without page refresh.
