# AIMWARE — CS2 Internal

🇧🇷 **Português BR** · 🇺🇸 **English**

---

# 🇧🇷 Português BR

## 🎯 Sobre

**AIMWARE** é um projeto **Internal Open Source para Counter-Strike 2**, desenvolvido com foco em performance, modularidade, personalização e facilidade de desenvolvimento.

O projeto reúne sistemas de **AIM, Triggerbot, Autofire, Autowall, ESP, Glow, Chams, Visuals, World, Skinchanger, Movement, Misc, Nade Prediction e HUD Widgets**, organizados em módulos independentes e altamente configuráveis.

O código-fonte é disponibilizado publicamente para **estudo, aprendizado, modificação e compilação**, enquanto o projeto permanece classificado como **internal**, e não external.

---

# 📦 Features

## ⚔️ AIM

* Aimbot com FOV, Smooth e Humanize
* 3 modos de smoothing:

  * Constant
  * Linear
  * Sine
* RCS standalone ou integrado ao Aimbot
* RCS com Scale X/Y e Smooth
* Perfis individuais por arma:

  * General
  * Pistol
  * SMG
  * Rifle
  * Shotgun
  * Sniper
  * LMG
* Hitboxes configuráveis:

  * Head
  * Neck
  * Chest
  * Stomach
  * Pelvis
  * Arms
  * Legs
  * Feet
* Multipoint por hitbox
* Multipoint Scale configurável
* Dynamic Multipoint
* Reaction Delay
* Target Switch Delay
* First Shot Delay
* Visibility Check
* Smoke Check
* Flash Check
* Scoped Only
* FOV Circle independente para:

  * Aimbot
  * Autofire
  * Magnet
* Cores independentes
* Keybind:

  * Always
  * Hold
  * Toggle

---

## 🎯 TRIGGERBOT

* Configuração individual por arma
* Hitchance via Monte Carlo
* Seed Nospread
* Delay configurável
* Autowall
* Scoped Only
* Flash Check
* Smoke Check
* Autostop
* Hitboxes configuráveis por grupo
* Magnet:

  * Soft Aim
  * Smooth
  * FOV
  * Silent
  * Head Priority
  * Deadzone
  * Hitboxes próprias
* Keybind:

  * Always
  * Hold
  * Toggle

---

## 🔫 AUTOFIRE

* FOV independente
* Hitchance
* Seed Nospread Mode
* Autostop
* Autoscope
* Scoped Only
* Autowall
* Minimum Damage:

  * Visible
  * Wallbang
* Minimum Damage Override
* Target Selection:

  * Crosshair
  * Distance
  * Damage
* Visibility Check
* Flash Check
* Smoke Check
* Focus Target
* Body if Lethal
* Prefer Body
* Hitboxes configuráveis
* Dynamic Multipoint
* Perfis individuais por arma
* Keybind:

  * Always
  * Hold
  * Toggle

---

## 🧱 AUTOWALL

* Sistema de penetração
* Keybind global:

  * Always
  * Hold
  * Toggle
* Autowall Crosshair:

  * Dot
  * Box
* Indicador de penetração:

  * Can
  * Can't
* Cores independentes

---

# 👁️ ESP

### Player ESP

* Full Box
* Corner Box
* Espessura configurável
* Fill com opacidade
* Visibility Check
* Cores independentes para visível/ocluído
* Health Bar
* Armor Bar
* Posição configurável
* Auto Color / Manual Color
* Nome
* Distância
* Arma:

  * Texto
  * Weapon Icon
* Skeleton
* Skeleton Thickness
* Cores Visible / Invisible
* Toggle da cabeça
* Steam Avatar
* Offscreen Arrows
* Competitive Rank
* 3D Box

  * Oriented AABB
* Floating Damage Numbers:

  * Normal
  * Headshot
  * Kill
* Damage Speed
* Damage Duration
* Flags:

  * Flashed
  * Bomb
  * Scoped
  * Reloading
  * Defusing
  * Money
  * Kit
  * Helmet
  * Nades
* Posição independente dos elementos:

  * Top
  * Bottom
  * Left
  * Right
* Sound ESP:

  * Ring
  * Duration
  * Size
  * Color

---

# 🌎 WORLD ESP

* Dropped Weapons

  * Icon
  * Text
  * Distance
* Bomb
* Bomb Timer
* Smoke
* Molotov
* HE Grenade
* Flashbang
* Decoy
* Cores individuais por tipo

---

# ✨ GLOW

### Players

* Team
* Enemy
* Visible
* Behind Wall

### World

* Weapons

* Grenades

* Cores independentes para Visible / Invisible

---

# 🎨 CHAMS

### Players

* Enemy Visible Layer
* Enemy XQZ Layer
* Team Visible Layer
* Team XQZ Layer

### Local

* Thirdperson Body
* Viewmodel Arms
* Viewmodel Weapon

### Other

* Ragdoll
* World Items

World Items por categoria:

* Pistol
* SMG
* Rifle
* Shotgun
* Sniper
* Utility

### Materials

* Flat
* Illuminate
* Glow
* Ghost
* Latex
* XQZ disponível nos materiais

---

# 👀 VISUALS

* FOV Changer
* Viewmodel:

  * FOV
  * X
  * Y
  * Z
* Thirdperson:

  * Distance
  * Always
  * Hold
  * Toggle
* Enemy Spectate enquanto morto:

  * Thirdperson
  * In-eye
* Antiflash:

  * 0–100%
* Remove Legs
* Remove Smoke
* Remove Decals
* Remove Crosshair
* Force Crosshair

  * Sniper Unscoped
* Custom Scope:

  * Size
  * Gap
  * Thickness
  * Color
* Scope Zoom FOV por estágio
* Hide Viewmodel
* Visual Recoil Removal
* Forced Aspect Ratio

---

# 🌌 WORLD

* Night Mode
* Exposure
* Skybox Tint
* Global Lighting Color
* Map Mesh Tint
* Custom Fog:

  * Color
  * Start
  * End
  * Falloff
* Weather:

  * Rain
  * Snow
  * Ash
  * Intensity
* Smoke Color
* Fire Color
* Inferno Color
* Explosion Color

---

# 🔪 SKINCHANGER

### Knife

* Model
* Paint
* Wear
* Seed
* StatTrak
* Custom Tag

### Gloves

* Model
* Paint
* Wear
* Seed
* StatTrak
* Custom Tag

### Weapons

* 70+ weapon slots
* WeaponSkin por DefIndex
* Custom Paintkit Colors:

  * Color0
  * Color1
  * Color2
  * Color3
  * Tint
* Glove Color independente

### Agents

* T Agent
* CT Agent

---

# 🏃 MOVEMENT

* Bunnyhop
* Autostrafe:

  * Mouse
  * Vectorial
* Jumpbug
* Edgejump
* Fastladder
* Keybinds configuráveis

---

# 🛠️ MISC

* Auto Pistol

  * Configurable Delay
* Auto Accept Matchmaking
* Auto Defuse
* Backtrack:

  * Configurable MS
  * Skeleton Visualization
* Hitmarker:

  * Screen
  * World 3D
  * Size
  * Thickness
  * Duration
  * Normal / Head / Kill Colors
* Hitsound:

  * Custom WAV
  * Headshot Sound
  * Kill Sound
* Hitlog:

  * Panel
  * Duration
  * Position
  * Width
  * Max Rows
  * Remaining HP
  * Session Statistics
* Bullet Impact Effect:

  * Overlay
  * Sparks
  * Both
  * Duration
  * Glow
* Bullet Tracers:

  * Color
  * Duration
* Vote Reveal
* Auto Vote:

  * Yes / No
  * Delay
* Scoreboard Weapon Icons
* Unlock Inventory

  * Loadout Mid-match
* Watermark

---

# 🖥️ WIDGETS / HUD

### Keybinds

* Active Keybinds
* Show Only Active
* Show All
* Custom Position

### Bomb Timer

* Damage
* Defuse Information
* Urgent Color

### Spectators

* Steam Avatars
* Maximum Spectators
* Custom Color

### Radar

* Position
* Size
* Circle / Square
* Custom Color

---

# 💣 NADE

### Grenade Helper

* Stand Circle
* Aim Marker
* Ativado somente segurando a grenade
* Selection Radius
* Lineup Capture

### Nade Prediction

* Live Trajectory
* Bounces
* In-Air Prediction
* Labels

---

# ⚙️ CONFIG

Sistema de configuração centralizado.

* JSON
* XPRESS_HUFF Compression
* Per-Weapon Profiles
* Copy-to-All
* Presets
* Menu customization:

  * Accent
  * Background
  * Sidebar
  * Border
  * Text
  * Rounding
  * Opacity
  * Glass
  * Compact Mode
  * DPI Scale
  * Font Size
  * Sidebar Labels

---

# 🏗️ Arquitetura

O projeto utiliza uma arquitetura modular para facilitar manutenção, desenvolvimento e expansão.

```text
AIMWARE
├── AIM
│   ├── Aimbot
│   ├── RCS
│   ├── Triggerbot
│   ├── Autofire
│   ├── Autowall
│   └── Multipoint
│
├── ESP
│   ├── Players
│   ├── World
│   ├── Glow
│   └── Chams
│
├── Visuals
│   ├── Camera
│   ├── Viewmodel
│   ├── Scope
│   ├── Recoil
│   └── World
│
├── Skinchanger
├── Movement
├── Misc
│
├── Nade
│   ├── Helper
│   └── Prediction
│
├── Widgets
│   ├── Keybinds
│   ├── Bomb
│   ├── Spectators
│   └── Radar
│
└── Config
```

O objetivo é manter os módulos desacoplados, permitindo adicionar ou modificar features sem criar dependências desnecessárias.

---

# 🔧 Build

### Requirements

* Windows
* Visual Studio 2022
* C++20
* Windows SDK
* DirectX 11
* Dependências incluídas no projeto

### Build

```bash
git clone <repository-url>
cd <repository-directory>
```

Abra a solution no Visual Studio e compile utilizando:

```text
Release
x64
```

---

# 🔓 Open Source

O **AIMWARE é Open Source**.

Qualquer pessoa pode:

* Visualizar o código
* Estudar a implementação
* Fazer fork
* Modificar o projeto
* Criar suas próprias features
* Compilar sua própria versão
* Enviar Pull Requests
* Reportar bugs e melhorias

O projeto é **internal**, mas seu código-fonte é aberto.

---

# ⚠️ Disclaimer

Este projeto é disponibilizado para fins educacionais e de pesquisa de software.

O uso de software que modifica ou automatiza o comportamento de jogos pode violar regras ou Termos de Serviço aplicáveis.

Não me responsabilizo por bans, perda de conta, uso indevido ou quaisquer consequências resultantes do uso do projeto.

Use por sua própria conta e risco.

---

# 📜 License

Open Source — consulte o arquivo `LICENSE` deste repositório para os termos de uso, modificação e redistribuição.

---

<br>

# 🇺🇸 English

## 🎯 About

**AIMWARE** is an **Internal Open Source project for Counter-Strike 2**, designed with a focus on performance, modularity, customization and ease of development.

The project combines **AIM, Triggerbot, Autofire, Autowall, ESP, Glow, Chams, Visuals, World, Skinchanger, Movement, Misc, Nade Prediction and HUD Widgets** into independent and highly configurable modules.

The source code is publicly available for **learning, research, modification and compilation**, while the project itself remains classified as **internal**, rather than external.

---

# 📦 Features

## ⚔️ AIM

* Aimbot with FOV, Smooth and Humanize
* 3 smoothing modes:

  * Constant
  * Linear
  * Sine
* Standalone or integrated RCS
* RCS Scale X/Y and Smooth
* Per-weapon profiles:

  * General
  * Pistol
  * SMG
  * Rifle
  * Shotgun
  * Sniper
  * LMG
* Configurable hitboxes:

  * Head
  * Neck
  * Chest
  * Stomach
  * Pelvis
  * Arms
  * Legs
  * Feet
* Per-hitbox multipoint
* Configurable multipoint scale
* Dynamic multipoint
* Reaction Delay
* Target Switch Delay
* First Shot Delay
* Visibility Check
* Smoke Check
* Flash Check
* Scoped Only
* Independent FOV circles for:

  * Aimbot
  * Autofire
  * Magnet
* Independent colors
* Keybind modes:

  * Always
  * Hold
  * Toggle

---

## 🎯 TRIGGERBOT

* Per-weapon configuration
* Monte Carlo hitchance
* Seed Nospread
* Configurable delay
* Autowall
* Scoped Only
* Flash Check
* Smoke Check
* Autostop
* Per-group hitboxes
* Magnet:

  * Soft Aim
  * Smooth
  * FOV
  * Silent
  * Head Priority
  * Deadzone
  * Independent hitboxes
* Keybind:

  * Always
  * Hold
  * Toggle

---

## 🔫 AUTOFIRE

* Independent FOV
* Hitchance
* Seed Nospread Mode
* Autostop
* Autoscope
* Scoped Only
* Autowall
* Minimum Damage:

  * Visible
  * Wallbang
* Minimum Damage Override
* Target Selection:

  * Crosshair
  * Distance
  * Damage
* Visibility Check
* Flash Check
* Smoke Check
* Focus Target
* Body if Lethal
* Prefer Body
* Configurable hitboxes
* Dynamic Multipoint
* Per-weapon profiles
* Keybind:

  * Always
  * Hold
  * Toggle

---

## 🧱 AUTOWALL

* Penetration system
* Global keybind:

  * Always
  * Hold
  * Toggle
* Autowall Crosshair:

  * Dot
  * Box
* Penetration indicator:

  * Can
  * Can't
* Independent colors

---

# 👁️ ESP

### Player ESP

* Full Box
* Corner Box
* Configurable thickness
* Fill with opacity
* Visibility Check
* Separate visible/occluded colors
* Health Bar
* Armor Bar
* Configurable element positions
* Auto Color / Manual Color
* Name
* Distance
* Weapon:

  * Text
  * Weapon Icon
* Skeleton
* Skeleton Thickness
* Visible / Invisible colors
* Head toggle
* Steam Avatar
* Offscreen Arrows
* Competitive Rank
* 3D Box

  * Oriented AABB
* Floating Damage Numbers:

  * Normal
  * Headshot
  * Kill
* Damage Speed
* Damage Duration
* Flags:

  * Flashed
  * Bomb
  * Scoped
  * Reloading
  * Defusing
  * Money
  * Kit
  * Helmet
  * Nades
* Independent element positioning:

  * Top
  * Bottom
  * Left
  * Right
* Sound ESP:

  * Ring
  * Duration
  * Size
  * Color

---

# 🌎 WORLD ESP

* Dropped Weapons:

  * Icon
  * Text
  * Distance
* Bomb
* Bomb Timer
* Smoke
* Molotov
* HE Grenade
* Flashbang
* Decoy
* Individual colors per type

---

# ✨ GLOW

### Players

* Team
* Enemy
* Visible
* Behind Wall

### World

* Weapons

* Grenades

* Separate Visible / Invisible colors

---

# 🎨 CHAMS

### Players

* Enemy Visible Layer
* Enemy XQZ Layer
* Team Visible Layer
* Team XQZ Layer

### Local

* Thirdperson Body
* Viewmodel Arms
* Viewmodel Weapon

### Other

* Ragdoll
* World Items

World Item Chams:

* Pistol
* SMG
* Rifle
* Shotgun
* Sniper
* Utility

### Materials

* Flat
* Illuminate
* Glow
* Ghost
* Latex
* XQZ support for materials

---

# 👀 VISUALS

* FOV Changer
* Viewmodel:

  * FOV
  * X
  * Y
  * Z
* Thirdperson:

  * Distance
  * Always
  * Hold
  * Toggle
* Enemy Spectate while dead:

  * Thirdperson
  * In-eye
* Antiflash:

  * 0–100%
* Remove Legs
* Remove Smoke
* Remove Decals
* Remove Crosshair
* Force Crosshair

  * Sniper Unscoped
* Custom Scope:

  * Size
  * Gap
  * Thickness
  * Color
* Scope Zoom FOV per stage
* Hide Viewmodel
* Visual Recoil Removal
* Forced Aspect Ratio

---

# 🌌 WORLD

* Night Mode
* Exposure
* Skybox Tint
* Global Lighting Color
* Map Mesh Tint
* Custom Fog:

  * Color
  * Start
  * End
  * Falloff
* Weather:

  * Rain
  * Snow
  * Ash
  * Intensity
* Smoke Color
* Fire Color
* Inferno Color
* Explosion Color

---

# 🔪 SKINCHANGER

### Knife

* Model
* Paint
* Wear
* Seed
* StatTrak
* Custom Tag

### Gloves

* Model
* Paint
* Wear
* Seed
* StatTrak
* Custom Tag

### Weapons

* 70+ weapon slots
* WeaponSkin by DefIndex
* Custom Paintkit Colors:

  * Color0
  * Color1
  * Color2
  * Color3
  * Tint
* Independent glove color

### Agents

* T Agent
* CT Agent

---

# 🏃 MOVEMENT

* Bunnyhop
* Autostrafe:

  * Mouse
  * Vectorial
* Jumpbug
* Edgejump
* Fastladder
* Configurable keybinds

---

# 🛠️ MISC

* Auto Pistol

  * Configurable Delay
* Auto Accept Matchmaking
* Auto Defuse
* Backtrack:

  * Configurable MS
  * Skeleton Visualization
* Hitmarker:

  * Screen
  * World 3D
  * Size
  * Thickness
  * Duration
  * Normal / Head / Kill Colors
* Hitsound:

  * Custom WAV
  * Headshot Sound
  * Kill Sound
* Hitlog:

  * Panel
  * Duration
  * Position
  * Width
  * Max Rows
  * Remaining HP
  * Session Statistics
* Bullet Impact Effect:

  * Overlay
  * Sparks
  * Both
  * Duration
  * Glow
* Bullet Tracers:

  * Color
  * Duration
* Vote Reveal
* Auto Vote:

  * Yes / No
  * Delay
* Scoreboard Weapon Icons
* Unlock Inventory

  * Loadout Mid-match
* Watermark

---

# 🖥️ WIDGETS / HUD

### Keybinds

* Active Keybinds
* Show Only Active
* Show All
* Custom Position

### Bomb Timer

* Damage
* Defuse Information
* Urgent Color

### Spectators

* Steam Avatars
* Maximum Spectators
* Custom Color

### Radar

* Position
* Size
* Circle / Square
* Custom Color

---

# 💣 NADE

### Grenade Helper

* Stand Circle
* Aim Marker
* Only active while holding a grenade
* Selection Radius
* Lineup Capture

### Nade Prediction

* Live Trajectory
* Bounces
* In-Air Prediction
* Labels

---

# ⚙️ CONFIG

Centralized configuration system.

* JSON
* XPRESS_HUFF Compression
* Per-weapon profiles
* Copy-to-all
* Presets
* Menu customization:

  * Accent
  * Background
  * Sidebar
  * Border
  * Text
  * Rounding
  * Opacity
  * Glass
  * Compact Mode
  * DPI Scale
  * Font Size
  * Sidebar Labels

---

# 🏗️ Architecture

The project uses a modular architecture designed to simplify maintenance, development and expansion.

```text
AIMWARE
├── AIM
│   ├── Aimbot
│   ├── RCS
│   ├── Triggerbot
│   ├── Autofire
│   ├── Autowall
│   └── Multipoint
│
├── ESP
│   ├── Players
│   ├── World
│   ├── Glow
│   └── Chams
│
├── Visuals
│   ├── Camera
│   ├── Viewmodel
│   ├── Scope
│   ├── Recoil
│   └── World
│
├── Skinchanger
├── Movement
├── Misc
│
├── Nade
│   ├── Helper
│   └── Prediction
│
├── Widgets
│   ├── Keybinds
│   ├── Bomb
│   ├── Spectators
│   └── Radar
│
└── Config
```

The goal is to keep modules decoupled, making it easier to add or modify features without introducing unnecessary dependencies.

---

# 🔧 Build

### Requirements

* Windows
* Visual Studio 2022
* C++20
* Windows SDK
* DirectX 11
* Project dependencies

### Build

```bash
git clone <repository-url>
cd <repository-directory>
```

Open the solution in Visual Studio and build using:

```text
Release
x64
```

---

# 🔓 Open Source

**AIMWARE is Open Source.**

Anyone can:

* Read the source code
* Study the implementation
* Fork the project
* Modify the code
* Create their own features
* Build their own version
* Submit Pull Requests
* Report bugs and improvements

The project is **internal**, while the source code remains publicly available.

---

# ⚠️ Disclaimer

This project is provided for educational and software research purposes.

Software that modifies or automates game behavior may violate applicable rules or Terms of Service.

I am not responsible for bans, account restrictions, misuse or any consequences resulting from the use of this project.

Use at your own risk.

---

# 📜 License

Open Source — see the `LICENSE` file in this repository for the terms governing use, modification and redistribution.
