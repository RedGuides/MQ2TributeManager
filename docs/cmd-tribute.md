---
tags:
  - command
---

# /tribute

## Syntax

<!--cmd-syntax-start-->
```eqcommand
/tribute [auto|named|manual|forceoff|show|on|off]
```
<!--cmd-syntax-end-->

## Description

<!--cmd-desc-start-->
Control tribute via MQ2TributeManager, enabling automatic personal tribute based on combat status.
<!--cmd-desc-end-->

## Options

`on|off`
:   Toggle tribute on or off

`auto`
:   Turn on personal tribute when in combat.

`named`
:   Turn on personal tribute when in combat with a named monster.

`manual`
:   Manually control personal tribute

`forceoff`
:   Turn tribute off immediately ignoring any remaining time left.

`show`
:   Shows the current mode the plugin is running in (i.e. on/off/auto).

## Examples

`/tribute named`
:   Turns on personal tribute when in combat with a named mob.