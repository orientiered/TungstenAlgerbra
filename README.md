# Tungsten Algebra

Parody on Wolfram Alpha
---
## Table of Contents
+ [Installation](#installation)
+ [Features](#features)
+ [Usage and examples](#usage-and-examples)
---

### Installation
0. Work on Windows is not guaranteed
1. Clone repository
2. Use ```make BUILD=RELEASE``` to compile

**Dependencies:**
1. `Make`
2. `g++`
3. `pdflatex` -> for compiling tex files to pdf
4. `dot` -> Creating graphical dumps in debug version

### Features

* Expression evalution
* Symbolic derivative of expressions with respect to any variable
* Taylor expansion

### Usage and examples

Run `TungstenAlgebra` with `./diff.out`

Use flag `-t <N>` or `--taylor <N>` to compute taylor expansion up to $o(x^{N - 1})$

By default, expansion is computed at $x = 0$.

Use `-p <D>` or `--point <D>` to calculate expansion at `x = D `

